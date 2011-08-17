#include <stdio.h>
#include <json/json.h>
#include <sys/time.h>
#include "thread_data.h"
#include "config.h"
#include "mylog.h"
#include "index_group.h"
#include "xhead.h"
#include <string.h>

#define TIME_US_COST(pre, cur) (((cur.tv_sec)-(pre.tv_sec))*1000000 + \
		        (cur.tv_usec) - (pre.tv_usec))
 
extern Config* myConfig;

const char* get_query_string(Json::Value& root)
{
    if (root["QUERY"].isNull() || !root["QUERY"].isString())
    {
        ALARM("jsonstr NOT contain 'QUERY'.");
        return NULL;
    }
    else
    {
        return root["QUERY"].asCString();
    }
}

int ServiceApp(thread_data_t* ptd)
{
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL );

    memset(ptd->SendHead, 0, sizeof(*ptd->SendHead));
	ptd->SendHead->log_id     = ptd->RecvHead->log_id;
	ptd->SendHead->detail_len = 0;
	snprintf(ptd->SendHead->srvname, sizeof(ptd->SendHead->srvname), "%s", PROJNAME);

    char* dststr = (char*) (ptd->SendHead+1);
    char* srcstr = (char*) (ptd->RecvHead+1);

    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(srcstr, root))
    {
        // 不是json的格式
        ALARM("jsonstr format error. [%s]", srcstr);
        return 0;
    }

    const char* query = get_query_string(root);
    if (query == NULL)
    {
        // 无法取得query
        return 0;
    }

    index_group* myIndexGroup = ptd->plugin->mysecore->m_pindex_group;

    int list_num = myIndexGroup->get_posting_list(query, dststr, ptd->SendBuffSize - sizeof(xhead_t));
    for (int i=0; i<list_num; i++)
    {
        printf("[%u] ", ((uint32_t*)dststr)[i]);
    }
    printf("\n");

    ptd->SendHead->detail_len = list_num <= 0? 0 : list_num*sizeof(uint32_t);

    gettimeofday(&tv2, NULL );
    uint32_t timecost = TIME_US_COST(tv1, tv2);
    ROUTN( "log_id[%u] name[%s] cltip[%s] timecost[%u] string[%s] query[%s] list_num[%d] detail_len[%d]",
            ptd->RecvHead->log_id, ptd->RecvHead->srvname, ptd->cltip, timecost,
            srcstr, query, list_num, ptd->SendHead->detail_len);

    return 0;
}
