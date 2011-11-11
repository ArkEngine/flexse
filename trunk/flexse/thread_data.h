#ifndef  __THREADDATA_H_
#define  __THREADDATA_H_
#include <stdint.h>
#include "mylog.h"
#include "flexse_plugin.h"
#include "xhead.h"
#include "equeue.h"
//#include "memblocks.h"
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct thread_data_t;
typedef int (*servfunc)(thread_data_t *); 

struct thread_data_t
{
	pthread_t      thandle;
	uint32_t       tid;
	uint32_t       SendBuffSize;
	uint32_t       RecvBuffSize;
    equeue*        poll;
	servfunc       servapp;
	flexse_plugin* plugin;
//    memblocks*     mempool;
	union{
		char*      RecvBuff;
		xhead_t*   RecvHead;
	};
	union{
		char*      SendBuff;
		xhead_t*   SendHead;
	};
    char           cltip[32];
};

#endif  //__THREADDATA_H_
