#include <stdio.h>
#include <json/json.h>
#include <sys/time.h>
#include <string.h>
#include "thread_data.h"
#include "config.h"
#include "mylog.h"
#include "xhead.h"
#include "service_app.h"
#include <signal.h>

#define TIME_US_COST(pre, cur) (((cur.tv_sec)-(pre.tv_sec))*1000000 + \
		        (cur.tv_usec) - (pre.tv_usec))
 
extern Config* myConfig;

const char* const STR_QUEUE_NAME     = "__QUEUE_NAME__";
const char* const STR_OPERATION      = "__OPERATION__";
const char* const STR_OPERATION_BODY = "__OPERATION_BODY__";

bool is_valid(Json::Value& root)
{
    if (root[STR_QUEUE_NAME].isNull() || !root[STR_QUEUE_NAME].isString())
    {
        ALARM("jsonstr NOT contain '%s' OR type error [%u]", STR_QUEUE_NAME, root.type());
        return false;
    }
    if (root[STR_OPERATION].isNull() || !root[STR_OPERATION].isString())
    {
        ALARM("jsonstr NOT contain '%s'. [%u]", STR_OPERATION, root.type());
        return false;
    }
    if (root[STR_OPERATION_BODY].isNull() || !root[STR_OPERATION_BODY].isObject())
    {
        ALARM("jsonstr NOT contain '%s'. [%u]", STR_OPERATION_BODY, root.type());
        return false;
    }
    return true;
}

int ServiceApp(thread_data_t* ptd) try
{
	struct timeval tv1, tv2;
	gettimeofday(&tv1, NULL );

	ptd->SendHead->log_id = ptd->RecvHead->log_id;
	snprintf(ptd->SendHead->srvname, sizeof(ptd->SendHead->srvname), "%s", PROJNAME);

//    char* dststr = (char*) (ptd->SendHead+1);
    char* srcstr = (char*) (ptd->RecvHead+1);
	srcstr[ptd->RecvHead->detail_len] = 0;

    ptd->SendHead->reserved = RET_OK;
    ptd->SendHead->detail_len = 0;

    if (ptd->RecvHead->detail_len == 0)
    {
        ROUTN("empty message. log_id[%u] host[%s] cltname[%s]",
                ptd->RecvHead->log_id, ptd->cltip,
                ptd->RecvHead->srvname);
        ptd->SendHead->reserved = RET_EMPTY_MESSAGE;
        return 0;
    }

    if (myConfig->NeedIpControl() && myConfig->IpNotValid(ptd->cltip))
    {
        ALARM("ip invalid. log_id[%u] host[%s] cltname[%s]",
                ptd->RecvHead->log_id, ptd->cltip,
                ptd->RecvHead->srvname);
        ptd->SendHead->reserved = RET_IP_INVALID;
        return 0;
    }

    Json::Value  root;
    Json::Reader reader;
    if (! reader.parse(srcstr, root))
    {
        ALARM("jsonstr format error. log_id[%u] host[%s] cltname[%s] jsonstr[%s]",
                ptd->RecvHead->log_id, ptd->cltip,
                ptd->RecvHead->srvname, srcstr);
        ptd->SendHead->reserved = RET_JSON_FORMAT_INVALID;
        return 0;
    }

    if (is_valid(root))
    {
        FileLinkBlock* pflb = myConfig->GetQueue();
        if (0 != pflb->write_message(ptd->SendHead->log_id, srcstr, ptd->RecvHead->detail_len))
        {
            ptd->SendHead->reserved = RET_WRITE_MQUEUE_FAIL;
        }
    }
    else
    {
        ptd->SendHead->reserved = RET_PROTOCOL_ERROR;
    }

    gettimeofday(&tv2, NULL );
    ROUTN( "log_id[%u] err[%d] cltname[%s] cltip[%s] timecost[%u] string[%s]",
            ptd->RecvHead->log_id, ptd->SendHead->reserved,
            ptd->RecvHead->srvname, ptd->cltip,
            TIME_US_COST(tv1, tv2), srcstr);

    return 0;
}
catch (...)
{
    FATAL("service_app exception caught. quit.");
    while(0 != raise(SIGKILL)){}
    return 0;
}
