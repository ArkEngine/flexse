#include <stdio.h>
#include <sys/time.h>
#include "ThreadData.h"
#include "Config.h"
#include "mylog.h"
#include "xHead.h"
#include <string.h>

#define TIME_US_COST(pre, cur) (((cur.tv_sec)-(pre.tv_sec))*1000000 + \
		        (cur.tv_usec) - (pre.tv_usec))
 
extern Config* myConfig;

int ServiceApp(thread_data_t* ptd)
{
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL );

	ptd->SendHead->log_id = ptd->RecvHead->log_id;
	snprintf(ptd->SendHead->srvname, sizeof(ptd->SendHead->srvname), "%s", PROJNAME);

    char* dststr = (char*) (ptd->SendHead+1);
    char* srcstr = (char*) (ptd->RecvHead+1);
	ptd->SendHead->detail_len = ptd->RecvHead->detail_len;
    memmove(dststr, srcstr, ptd->RecvHead->detail_len);
	gettimeofday(&tv2, NULL );
	uint32_t timecost = TIME_US_COST(tv1, tv2);
	ROUTN( "log_id[%u] name[%s] cltip[%s] timecost[%u] string[%s] detail_len[%d]",
			ptd->RecvHead->log_id, ptd->RecvHead->srvname, inet_ntoa (ptd->cltaddr.sin_addr), timecost,
			srcstr, ptd->SendHead->detail_len);

	return 0;
}
