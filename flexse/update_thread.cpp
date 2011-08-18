/*
 * 更新线程处理的操作分为
 * (1) 插入文档，需要分配内部的ID和更新倒排索引及文档属性
 * (2) 修改文档，需要分配内部的ID，把modbitmap置位，更新倒排索引及文档属性
 * (3) 更新文档属性，更新文档属性即可。
 * (4) 删除文档，把delbitmap置位
 * (5) 反删除文档，把delbitmap复位
 */
#include "update_thread.h"
#include "config.h"
#include "flexse_plugin.h"
#include "MyException.h"
#include "mylog.h"
#include "myutils.h"
#include "xhead.h"
#include "mem_indexer.h"
#include "nlp_processor.h"
#include "index_group.h"
#include <json/json.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

extern Config* myConfig;
using namespace flexse;

void* update_thread(void* args)
{
    flexse_plugin* pflexse_plugin = (flexse_plugin*)args;

    int32_t listenfd = mylisten(myConfig->UpdatePort());
    MySuicideAssert(listenfd != -1);
    xhead_t* recv_head = (xhead_t*)malloc(myConfig->UpdateReadBufferSize());
    xhead_t  send_head;
    memset(&send_head, 0, sizeof(xhead_t));
    snprintf(send_head.srvname, sizeof(send_head.srvname), "%s", PROJNAME);
    send_head.log_id = recv_head->log_id;
    send_head.detail_len = 0;

    while(1)
    {
        sockaddr_in cltaddr;
        socklen_t   caddr_len=0;
        int clientfd = accept(listenfd, (struct sockaddr *) &cltaddr, &caddr_len);
        if(clientfd < 0)
        {
            ALARM( "accept client fail, don't fuck my besiness. msg[%m]");
            continue;
        }
        // setnonblock(clientfd);
        int tcp_nodelay = 1;
        setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int));
        int ret = 0;
        while(0 == (ret = xrecv(clientfd, recv_head, myConfig->UpdateReadBufferSize(),
                        myConfig->UpdateSocketTimeOutMS())))
        {
            char* jsonstr = (char*)(&recv_head[1]);
            jsonstr[recv_head->detail_len] = 0;
            Json::Value root;
            Json::Reader reader;
            if (! reader.parse(jsonstr, root))
            {
                ALARM("jsonstr format error. [%s]", jsonstr);
                break;
            }

            if ( root["OPERATION"].isNull() || ! root["OPERATION"].isString() )
            {
                ALARM("jsonstr NOT contain 'OPERATION'. [%s]", jsonstr);
                break;
            }

            const char* str_operation = root["OPERATION"].asCString();
            if (0 == strcmp(str_operation, "INSERT"))
            {
                if (0 != add(pflexse_plugin, jsonstr))
                {
                    break;
                }
            }
            else if (0 == strcmp(str_operation, "UPDATE"))
            {
                // add 和 mod 一回事
                if (0 != add(pflexse_plugin, jsonstr))
                {
                    break;
                }
            }
            else if (0 == strcmp(str_operation, "DELETE"))
            {
                if (0 != del(pflexse_plugin, jsonstr))
                {
                    break;
                }
            }
            else if (0 == strcmp(str_operation, "RESTORE"))
            {
                if (0 != undel(pflexse_plugin, jsonstr))
                {
                    break;
                }
            }
            else
            {
                ALARM("undefined OPERATION[%s].", str_operation);
                break;
            }

            if(0 != xsend(clientfd, &send_head, myConfig->UpdateSocketTimeOutMS()))
            {
                ALARM("send socket error. ret[%d] detail_len[%u] timeoutMS[%u] msg[%m]",
                        ret, send_head.detail_len, myConfig->UpdateSocketTimeOutMS());
                break;
            }
            else
            {
                ROUTN("update OK! message[%s]", jsonstr);
            }
        }
        ALARM("recv socket error. ret[%d] buffsiz[%u] timeoutMS[%u] msg[%m]",
                ret, myConfig->UpdateReadBufferSize(), myConfig->UpdateSocketTimeOutMS());
        close(clientfd);
    }
    free(recv_head);
    close(listenfd);
    return NULL;
}

int add(flexse_plugin* pflexse_plugin, const char* jsonstr)
{
    vector<string> vstr;
    uint32_t       doc_id;
    int retp = pflexse_plugin->add(jsonstr, doc_id, vstr);
    if (retp != 0)
    {
        return -1;
    }

    // 为doc_id分配内部id
    // 检查一下以前有没有内部id，如果有的话，则把mod_bitmap置位
    uint32_t tmpid = pflexse_plugin->mysecore->m_idmap->getInnerID(doc_id);
    if (tmpid > 0)
    {
        _SET_BITMAP_1_(*(pflexse_plugin->mysecore->m_mod_bitmap), tmpid);
    }
    uint32_t innerid = pflexse_plugin->mysecore->m_idmap->allocInnerID(doc_id);
    if (innerid == 0)
    {
        ALARM("allocInnerID Fail.");
        return -1;
    }
    return _insert(pflexse_plugin, innerid, vstr);
}

int _insert(flexse_plugin* pflexse_plugin, const uint32_t id, const vector<string>& vstr)
{
    index_group* myIndexGroup = pflexse_plugin->mysecore->m_pindex_group;
    mem_indexer* pindexer = myIndexGroup->get_cur_mem_indexer();

    bool have_swap = false;
    bool nearly_full = false;
    for (uint32_t i=0; i<vstr.size(); i++)
    {
        int setret = pindexer->set_posting_list(vstr[i].c_str(), &id);
        if (postinglist::FULL == setret)
        {
            ALARM( "SET POSTING LIST ERROR. LIST FULL, GO TO SWITCH. ID[%u]", id);
            pindexer = myIndexGroup->swap_mem_indexer();
            MySuicideAssert(postinglist::OK == pindexer->set_posting_list(vstr[i].c_str(), &id));
            have_swap = true;
        }
        if (postinglist::NEARLY_FULL == setret)
        {
            nearly_full = true;
        }
    }
    if (nearly_full && !have_swap)
    {
        pindexer = myIndexGroup->swap_mem_indexer();
        ROUTN( "SET POSTING LIST NEARLY_FULL. SWAPED");
    }
    // DEBUG
    // uint32_t plist[1000];
    // int rnum = myIndexGroup->get_posting_list("this", plist, sizeof(plist));
    // for (int i=0; i<rnum; i++)
    // {
    //     printf("[%u] ", plist[i]);
    // }
    // printf("\n");
    return 0;
}

int del(flexse_plugin* pflexse_plugin, const char* jsonstr)
{
    vector<uint32_t> id_list;
    int retp = pflexse_plugin->del(jsonstr, id_list);
    if (retp != 0)
    {
        return -1;
    }

    // update the del bitmap
    for (uint32_t i=0; i<id_list.size(); i++)
    {
        uint32_t innerid = pflexse_plugin->mysecore->m_idmap->getInnerID(id_list[i]);
        if (innerid == 0)
        {
            ALARM("getInnerID(%u) Fail.", id_list[i]);
            continue;
        }
        DEBUG("_SET_BITMAP_1_ outerid[%u] innerid[%u]", id_list[i], innerid);
        _SET_BITMAP_1_(*(pflexse_plugin->mysecore->m_del_bitmap), innerid);
    }

    return 0;
}

int undel(flexse_plugin* pflexse_plugin, const char* jsonstr)
{
    vector<uint32_t> id_list;
    int retp = pflexse_plugin->undel(jsonstr, id_list);
    if (retp != 0)
    {
        return -1;
    }

    // update the del bitmap
    for (uint32_t i=0; i<id_list.size(); i++)
    {
        uint32_t innerid = pflexse_plugin->mysecore->m_idmap->getInnerID(id_list[i]);
        if (innerid == 0)
        {
            ALARM("getInnerID(%u) Fail.", id_list[i]);
            continue;
        }
        DEBUG("_SET_BITMAP_0_ outerid[%u] innerid[%u]", id_list[i], innerid);
        _SET_BITMAP_0_(*(pflexse_plugin->mysecore->m_del_bitmap), innerid);
    }

    return 0;
}
