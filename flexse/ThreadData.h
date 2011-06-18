#ifndef  __THREADDATA_H_
#define  __THREADDATA_H_
#include <stdint.h>
#include "mylog.h"
#include "xHead.h"
#include "equeue.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct thread_data_t;
typedef int (*servfunc)(thread_data_t *); 

struct thread_data_t
{
	pthread_t thandle;
	uint32_t  tid;
	uint32_t  SendBuffSize;
	uint32_t  RecvBuffSize;
    equeue*   poll;
	servfunc  servapp;
	union{
		char*     RecvBuff;
		xhead_t*  RecvHead;
	};
	union{
		char*     SendBuff;
		xhead_t*  SendHead;
	};
	sockaddr_in cltaddr;
};

#endif  //__THREADDATA_H_
