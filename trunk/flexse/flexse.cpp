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
#include "Config.h"
#include "mylog.h"
#include "xHead.h"
#include "index_group.h"
#include "ThreadData.h"
#include "MyException.h"
#include "equeue.h"
#include "ontime_thread.h"
#include "update_thread.h"
#include "merger_thread.h"

Config* myConfig;
index_group* myIndexGroup;

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
	printf("BuildDate  :  %s\n", __DATE__);
}

int read_file_all(const char* file, unsigned char* buff, const uint32_t bufsize)
{
	if (file == NULL || buff == NULL || bufsize == 0) {
		ALARM("param error. file[%p] buff[%p] bufsize[%u]",
                file, buff, bufsize);
		return -1;
	}
	int fd = open(file, O_RDONLY);
	if (fd == -1) {
		ALARM( "open file[%s] failed. %m", file);
		return -1;
	}
    int32_t offset = lseek(fd, 0, SEEK_END);
	if (offset == -1) {
		ALARM( "lssek file[%s] failed. %m", file);
		return -1;
	}
	if ((uint32_t)offset > bufsize || offset == 0) {
		ALARM( "file[%s] size[%d] tooo big or small. bufsize[%d]", file, offset, bufsize);
		return -1;
	}
	lseek(fd, 0, SEEK_SET);
	int left = offset;  while (left > 0) {
		int len = read(fd, buff+offset-left, left);
		if (len == -1 || len == 0) {
			ALARM( "readfile failed. ERROR[%m]");
			return -1;
		}
		left -= len;
	}
	DEBUG( "read file[%s] all ok.", file);
	close(fd);
	return offset;
}

	void*
ServiceThread(void* args)
{
	thread_data_t *ptd = (thread_data_t*)args;
	int sock = -1;
	int ret = -1;
	int netret = -1;
	socklen_t   caddr_len=0;

	ROUTN( "ServiceThread[%u] Create OK", ptd->tid);

	while( 1 )
	{
		caddr_len=sizeof(ptd->cltaddr);
		netret = -1;
		ret = -1;
        sock = ptd->poll->fetch_socket();
		if( sock>0 ){
			getpeername(sock, (sockaddr*)&ptd->cltaddr, &caddr_len );
			DEBUG( "new task. sock:%d", sock );
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
 * ���̹߳���ģ��
 * (1) ���������˿�
 * (2) �����̶߳� accept() ����������
 * (3) �����ӵ���ʱ��ĳ���̵߳õ������ӣ����غ�����Լ��Ĵ�����
 * (4) ����������������������socket���ȴ�һ����ʱʱ���ر�
 * (5) ����߳��ֻص���accept��������
 */
thread_data_t* ServiceThreadInit(equeue* myequeue)
{
    thread_data_t* ptd = (thread_data_t*)malloc(sizeof(thread_data_t)*myConfig->ServiceThreadNum());
    if (ptd == NULL) {
        FATAL( "fucking funny, malloc failed [%m]");
        while(0!=raise(SIGKILL)){}
    }

    for (uint32_t i=0; i<myConfig->ServiceThreadNum(); i++)
    {
        ptd[i].tid = i;
        ptd[i].SendBuffSize = myConfig->ThreadBufferSize();
        ptd[i].RecvBuff = (char*)malloc(myConfig->ThreadBufferSize());
        ptd[i].SendBuff = (char*)malloc(myConfig->ThreadBufferSize());
        ptd[i].RecvBuffSize = myConfig->ThreadBufferSize();
        ptd[i].servapp = ServiceApp;
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
    myIndexGroup = new index_group(myConfig->CellSize(), myConfig->BucketSize(),
            myConfig->HeadListSize(), myConfig->MemBlockNumList(), myConfig->MemBlockNumListSize());
    pthread_t ontime_thread_id;
    pthread_t update_thread_id;
    pthread_t merger_thread_id;
    // init update thread
    MyThrowAssert ( 0 == pthread_create(&ontime_thread_id, NULL, ontime_thread, NULL));
    // init merger thread
    MyThrowAssert ( 0 == pthread_create(&update_thread_id, NULL, update_thread, NULL));
    // init ontime thread
    MyThrowAssert ( 0 == pthread_create(&merger_thread_id, NULL, merger_thread, NULL));

    equeue* myequeue = new equeue(myConfig->PollSize(), myConfig->QueryPort());
    // generate service-thread
    thread_data_t* ptd = ServiceThreadInit(myequeue);
    ROUTN( "All Service Threads Init Ok");
    // hold the place and not quit
    myequeue->running();
    for( uint32_t i=0; i<myConfig->ServiceThreadNum(); i++ )
    {
        pthread_join( ptd[i].thandle,NULL );
    }
    pthread_join( ontime_thread_id, NULL );
    pthread_join( update_thread_id, NULL );
    pthread_join( merger_thread_id, NULL );
    return 0;
}
