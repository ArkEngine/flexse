#include <pthread.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <dlfcn.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "config.h"
#include "mylog.h"
#include "xhead.h"
#include "bitmap.h"
#include "index_group.h"
#include "thread_data.h"
#include "MyException.h"
#include "equeue.h"
#include "ontime_thread.h"
#include "update_thread.h"
#include "day_merger_thread.h"
#include "his_merger_thread.h"

Config* myConfig;

int ServiceApp(thread_data_t* ptd);

void PrintHelp(void)
{
	printf("\nUsage:\n");
	printf("%s <options>\n", PROJNAME);
	printf("  options:\n");
	printf("    -c:  #conf path\n");
	printf("    -v:  #version\n");
	printf("    -h:  #This page\n");
	printf("\n\n");
}

void PrintVersion(void)
{
	printf("Project    :  %s\n", PROJNAME);
	printf("Version    :  %s\n", VERSION);
	printf("BuildDate  :  %s - %s\n", __DATE__, __TIME__);
}

	void*
ServiceThread(void* args)
{
	thread_data_t* ptd = (thread_data_t*)args;
	int sock = -1;
	int ret = -1;
	int netret = -1;
	socklen_t   caddr_len=0;

	ROUTN( "ServiceThread[%u] Create OK", ptd->tid);

	while( 1 )
	{
		netret = -1;
		ret = -1;
        sock = ptd->poll->fetch_socket();
		if( sock>0 ){
            sockaddr_in cltaddr;
            caddr_len = sizeof(cltaddr);
            getpeername(sock, (sockaddr*)&cltaddr, &caddr_len );
            inet_ntop(AF_INET, (void *)&cltaddr.sin_addr, ptd->cltip, sizeof(ptd->cltip));
        }
        else
        {
            ALARM( "sock:%d %m", sock );
            continue;
        }

        memset (ptd->RecvHead, 0, sizeof(ptd->RecvHead));
        netret = xrecv(sock, ptd->RecvHead, ptd->RecvBuffSize, myConfig->RTimeMS());
        if( netret <0 && errno != EAGAIN)
        {
            DEBUG( "xrecv error. sock:%d ret:%d name:%s body_len:%d to[%u] errno[%d] msg[%m]",
                    sock, netret, ptd->RecvHead->srvname,ptd->RecvHead->detail_len,
                    myConfig->RTimeMS(), errno);
            ptd->poll->free_socket(sock, 0);
            continue;
        }
        else
        {
            ret = (ptd->servapp)( ptd );
            netret = xsend(sock, ptd->SendHead, myConfig->WTimeMS());
            if(( netret<0 || ret<0 ) && errno != EAGAIN){
                DEBUG( "xsend error. sock:%d netret:%d ret:%d errno[%d] %m", sock, netret, ret, errno );
                ptd->poll->free_socket(sock, 0);
                continue;
            }
        }
        ptd->poll->free_socket(sock, 1);
    }
    return NULL;
}

/*
 * 多线程工作模型
 * (1) 建立监听端口
 * (2) 各个线程对 accept() 进行锁竞争
 * (3) 当连接到来时，某个线程得到了连接，返回后调用自己的处理函数
 * (4) 服务结束，继续阻塞于这个socket，等待一个超时时间后关闭
 * (5) 这个线程又回到了accept的锁竞争
 */
thread_data_t* ServiceThreadInit(equeue* myequeue, flexse_plugin* pflexse_plugin)
{
    thread_data_t* ptd = (thread_data_t*)malloc(sizeof(thread_data_t)*myConfig->ServiceThreadNum());
    if (ptd == NULL) {
        FATAL( "fucking funny, malloc failed [%m]");
        while(0!=raise(SIGKILL)){}
    }
//    memblocks* queryMemblocks = new memblocks(myConfig->QueryMemblockSizList(),
//            myConfig->QueryMemblockNumList(), myConfig->QueryMemblockCatNum());
//    MySuicideAssert(queryMemblocks != NULL);

    for (uint32_t i=0; i<myConfig->ServiceThreadNum(); i++)
    {
        ptd[i].tid = i;
        ptd[i].SendBuffSize = myConfig->ThreadBufferSize();
        ptd[i].RecvBuff = (char*)malloc(myConfig->ThreadBufferSize());
        ptd[i].SendBuff = (char*)malloc(myConfig->ThreadBufferSize());
        ptd[i].RecvBuffSize = myConfig->ThreadBufferSize();
        ptd[i].servapp = ServiceApp;
        ptd[i].plugin  = pflexse_plugin;
//        ptd[i].mempool = queryMemblocks;
        ptd[i].poll = myequeue;
        if (ptd[i].RecvBuff == NULL || ptd[i].SendBuff == NULL)
        {
            FATAL( "fucking funny, malloc failed [%m]");
            while(0!=raise(SIGKILL)){}
        }
        if (0 != pthread_create(&ptd[i].thandle, NULL, ServiceThread, (void*)&ptd[i]))
        {
            while(0 != raise(SIGKILL)) {}
        }
    }
    return ptd;
}

int main(int argc, char* argv[])
{
    signal(SIGPIPE, SIG_IGN);
    const char * strConfigPath = "./conf/"PROJNAME".config.json";
    char configPath[128];
    snprintf(configPath, sizeof(configPath), "%s", strConfigPath);

    char c = '\0';
    while ((c = getopt(argc, argv, "c:vh?")) != -1)
    {
        switch (c)
        {
            case 'c':
                snprintf(configPath, sizeof(configPath), "%s", optarg);
                strConfigPath = configPath;
                break;
            case 'v':
                PrintVersion();
                return 0;
            case 'h':
            case '?':
                PrintHelp();
                return 0;
            default:
                break;
        }
    }
    // read the config and init the process
    myConfig = new Config (configPath);
    if (myConfig == NULL) {
        while(0 != raise(SIGKILL)){}
    }
    ROUTN( "=====================================================================");
    secore* mysecore = new secore(myConfig->PluginConfigPath());
    flexse_plugin* pflexse_plugin = new flexse_plugin(myConfig->PluginConfigPath(), mysecore);
    pthread_t ontime_thread_id;
    pthread_t update_thread_id;
    pthread_t day_merger_thread_id;
    pthread_t his_merger_thread_id;
    // init ontime thread
    MySuicideAssert ( 0 == pthread_create(&ontime_thread_id, NULL, ontime_thread, pflexse_plugin));
    // init update thread
    MySuicideAssert ( 0 == pthread_create(&update_thread_id, NULL, update_thread, pflexse_plugin));
    // init merger thread
    MySuicideAssert ( 0 == pthread_create(&day_merger_thread_id, NULL, day_merger_thread, pflexse_plugin));
    MySuicideAssert ( 0 == pthread_create(&his_merger_thread_id, NULL, his_merger_thread, pflexse_plugin));

    equeue* myequeue = new equeue(myConfig->PollSize(), myConfig->QueryPort());
    // generate service-thread
    thread_data_t* ptd = ServiceThreadInit(myequeue, pflexse_plugin);
    ROUTN( "All Service Threads Init Ok");
    // hold the place and not quit
    myequeue->running();
    for( uint32_t i=0; i<myConfig->ServiceThreadNum(); i++ )
    {
        pthread_join( ptd[i].thandle,NULL );
    }
    pthread_join( ontime_thread_id, NULL );
    pthread_join( update_thread_id, NULL );
    pthread_join( day_merger_thread_id, NULL );
    pthread_join( his_merger_thread_id, NULL );
    delete myConfig;
    delete mysecore;
    delete pflexse_plugin;
    delete myequeue;
    return 0;
}
