#include "update_thread.h"
#include "Config.h"
#include "MyException.h"
#include "mylog.h"
#include "utils.h"
#include "xHead.h"
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

void* update_thread(void*)
{
    int32_t listenfd = mylisten(myConfig->UpdatePort());
    MyThrowAssert(listenfd != -1);
    xhead_t* recv_head = (xhead_t*)malloc(myConfig->UpdateReadBufferSize());
    xhead_t  send_head;
    snprintf(send_head.srvname, sizeof(send_head.srvname), "%s", PROJNAME);
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
            ROUTN("client[%s] ip[%s] message[%s]",
                    recv_head->srvname, inet_ntoa (cltaddr.sin_addr), (char*)(&recv_head[1]));
            // 你该干点什么了
            // (1) 插入，自己分辨出什么是否需要更新 d-a 数据
            // (2) 删除
            send_head.log_id = recv_head->log_id;
            send_head.detail_len = 0;
            if(0 != xsend(clientfd, &send_head, myConfig->UpdateReadTimeOutMS()))
            {
                ALARM("send socket error. ret[%d] detail_len[%u] timeoutMS[%u] msg[%m]",
                        ret, send_head.detail_len, myConfig->UpdateReadTimeOutMS());
                break;
            }
        }
        ALARM("recv socket error. ret[%d] buffsiz[%u] timeoutMS[%u] msg[%m]",
                ret, myConfig->UpdateReadBufferSize(), myConfig->UpdateReadTimeOutMS());
        close(clientfd);
    }
    free(recv_head);
    close(listenfd);
    return NULL;
}
