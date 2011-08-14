#include "update_thread.h"
#include "config.h"
#include "MyException.h"
#include "mylog.h"
#include "myutils.h"
#include "xhead.h"
#include "mem_indexer.h"
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
extern index_group* myIndexGroup;

using namespace flexse;

int insert_process(const char* jsonstr, uint32_t& doc_id, vector<string> & vstr)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }
    if (root["DOC_ID"].isNull())
    {
        ALARM("jsonstr NOT contain 'DOC_ID'. [%s]", jsonstr);
        return -1;
    }
    else
    {
        doc_id = root["DOC_ID"].asInt();
    }

    if (root["CONTENT"].isNull() || !root["CONTENT"].isString())
    {
        ALARM("jsonstr NOT contain 'CONTENT'. [%s]", jsonstr);
        return -1;
    }
    else
    {
        flexse::strspliter((char*)root["CONTENT"].asCString(), vstr);
    }
    return 0;
}

void* update_thread(void*)
{
    int32_t listenfd = mylisten(myConfig->UpdatePort());
    MyThrowAssert(listenfd != -1);
    xhead_t* recv_head = (xhead_t*)malloc(myConfig->UpdateReadBufferSize());
    xhead_t  send_head;
    memset(&send_head, 0, sizeof(xhead_t));
    snprintf(send_head.srvname, sizeof(send_head.srvname), "%s", PROJNAME);
    send_head.log_id = recv_head->log_id;

    mem_indexer* pindexer = myIndexGroup->get_cur_mem_indexer();

    while(1)
    {
        sockaddr_in cltaddr;
        socklen_t   caddr_len=0;
        int clientfd = accept(listenfd, (struct sockaddr *) &cltaddr, &caddr_len);
        if(clientfd < 0)
        {
            ALARM( "accept client fail, don't fuck me. msg[%m]");
            continue;
        }
        //        setnonblock(clientfd);
        int tcp_nodelay = 1;
        setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int));
        int ret = 0;
        while(0 == (ret = xrecv(clientfd, recv_head, myConfig->UpdateReadBufferSize(),
                        myConfig->UpdateSocketTimeOutMS())))
        {
            char* jsonstr = (char*)(&recv_head[1]);
            jsonstr[recv_head->detail_len] = 0;
//            ROUTN("client[%s] ip[%s] message[%s]",
//                    recv_head->srvname, inet_ntoa (cltaddr.sin_addr), (char*)(&recv_head[1]));
            // 你该干点什么了
            // (1) 插入，自己分辨出什么是否需要更新 d-a 数据
            // (2) 删除
            vector<string> vstr;
            uint32_t       doc_id;
            int retp = insert_process(jsonstr, doc_id, vstr);
            if (retp != 0)
            {
                break;
            }

            bool have_swap = false;
            bool nearly_full = false;
            for (uint32_t i=0; i<vstr.size(); i++)
            {
                int setret = pindexer->set_posting_list(vstr[i].c_str(), &doc_id);
                if (postinglist::FULL == setret)
                {
                    ALARM( "SET POSTING LIST ERROR. LIST FULL, GO TO SWITCH. ID[%u]", doc_id);
                    pindexer = myIndexGroup->swap_mem_indexer();
                    MyThrowAssert(postinglist::OK == pindexer->set_posting_list(vstr[i].c_str(), &doc_id));
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
            send_head.log_id = recv_head->log_id;
            send_head.detail_len = 0;
            if(0 != xsend(clientfd, &send_head, myConfig->UpdateSocketTimeOutMS()))
            {
                ALARM("send socket error. ret[%d] detail_len[%u] timeoutMS[%u] msg[%m]",
                        ret, send_head.detail_len, myConfig->UpdateSocketTimeOutMS());
                break;
            }
            else
            {
                ROUTN("update OK! doc_id[%u] message[%s]", doc_id, jsonstr);
            }
            // DEBUG
//            uint32_t plist[1000];
//            int rnum = myIndexGroup->get_posting_list("this", plist, sizeof(plist));
//            for (int i=0; i<rnum; i++)
//            {
//                printf("[%u] ", plist[i]);
//            }
//            printf("\n");
        }
        ALARM("recv socket error. ret[%d] buffsiz[%u] timeoutMS[%u] msg[%m]",
                ret, myConfig->UpdateReadBufferSize(), myConfig->UpdateSocketTimeOutMS());
        close(clientfd);
    }
    free(recv_head);
    close(listenfd);
    return NULL;
}
