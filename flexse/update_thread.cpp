#include "update_thread.h"
#include "Config.h"
#include "MyException.h"
#include "mylog.h"
#include "utils.h"
#include "xHead.h"
#include "mem_indexer.h"
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
    snprintf(send_head.srvname, sizeof(send_head.srvname), "%s", PROJNAME);

    const uint32_t cell_size     = sizeof(uint32_t);
    const uint32_t bucket_size   = 20;
    const uint32_t headlist_size = 0x1000000;
    uint32_t blocknum_list[8];
    for (uint32_t i=0; i<8; i++)
    {
        blocknum_list[i] = (i==0)? 4096: blocknum_list[i-1]/2;
    }
    mem_indexer mymem_indexer(cell_size, bucket_size, headlist_size, blocknum_list, 8);

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
                        myConfig->UpdateReadTimeOutMS())))
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
            for (uint32_t i=0; i<vstr.size(); i++)
            {
                if (postinglist::FULL == mymem_indexer.set_posting_list(vstr[i].c_str(), &doc_id))
                {
                    ALARM( "SET POSTING LIST ERROR. LIST FULL, GO TO SWITCH. ID[%u]", doc_id);
                }
            }
            send_head.log_id = recv_head->log_id;
            send_head.detail_len = 0;
            if(0 != xsend(clientfd, &send_head, myConfig->UpdateReadTimeOutMS()))
            {
                ALARM("send socket error. ret[%d] detail_len[%u] timeoutMS[%u] msg[%m]",
                        ret, send_head.detail_len, myConfig->UpdateReadTimeOutMS());
                break;
            }
            // DEBUG
//            uint32_t plist[1000];
//            int rnum = mymem_indexer.get_posting_list("this", plist, sizeof(plist));
//            for (int i=0; i<rnum; i++)
//            {
//                printf("[%u] ", plist[i]);
//            }
//            printf("\n");
        }
        ALARM("recv socket error. ret[%d] buffsiz[%u] timeoutMS[%u] msg[%m]",
                ret, myConfig->UpdateReadBufferSize(), myConfig->UpdateReadTimeOutMS());
        close(clientfd);
    }
    free(recv_head);
    close(listenfd);
    return NULL;
}
