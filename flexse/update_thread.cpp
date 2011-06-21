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
    xhead_t* read_head = (xhead_t*)malloc(myConfig->UpdateReadBufferSize());
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
        setnonblock(clientfd);
        int tcp_nodelay = 1;
        setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int));
        int ret = 0;
        while(0 == (ret = xrecv(clientfd, (xhead_t*)read_head,
                        myConfig->UpdateReadBufferSize(), myConfig->UpdateReadTimeOutMS())))
        {
            ROUTN("client message[%s]", (char*)(&read_head[1]));
            // 你该干点什么了
            // (1) 插入，自己分辨出什么是否需要更新 d-a 数据
            // (2) 删除
        }
        ALARM("read socket error. ret[%d] buffsiz[%u] timeoutMS[%u] msg[%m]",
                ret, myConfig->UpdateReadBufferSize(), myConfig->UpdateReadTimeOutMS());
        close(clientfd);
    }
    free(read_head);
    close(listenfd);
    return NULL;
}
