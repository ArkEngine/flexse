#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <map>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include "mylog.h"
#include "MyException.h"
#include "config.h"

using namespace std;

const char* const Config::m_StrLogLevel = "LogLevel";
const char* const Config::m_StrLogSize  = "LogSize";
const char* const Config::m_StrLogName  = "LogName";

const char* const Config::m_StrQuerySendTimeOut = "SendTimeOut_MS";
const char* const Config::m_StrQueryRecvTimeOut = "RecvTimeOut_MS";
const char* const Config::m_StrQueryPort   = "QueryPort";
const char* const Config::m_StrServiceThreadNum = "ServiceThreadNum";
const char* const Config::m_StrThreadBufferSize = "ThreadBufferSize";
const char* const Config::m_StrEpollSize = "EpollSize";

const char* const Config::m_StrUpdatePort = "UpdatePort";
const char* const Config::m_StrUpdateReadBufferSize = "ReadBufferSize";
const char* const Config::m_StrUpdateSocketTimeOutMS  = "SocketTimeOut_MS";

const char* const Config::m_StrCellSize = "PostingListCellSize";
const char* const Config::m_StrBucketSize = "PostingBucketSize";
const char* const Config::m_StrHeadListSize = "PostingHeadListSize";
const char* const Config::m_StrMemBlockNumList = "PostingMemBlockNumList";

Config::Config(const char* configpath)
{
    // DEFAULT CONFIG
    m_log_level = mylog :: ROUTN;
    m_log_size  = 1 << 31;
    m_log_name  = PROJNAME;
    SETLOG(m_log_level, m_log_size, m_log_name);

    m_pollsize           = 128;
    m_wtimeout_ms        = 1000;
    m_rtimeout_ms        = 1000;
    m_query_port         = 1983;
    m_service_thread_num = 20;
    m_thread_buffer_size = 10*1024*1024;

    m_update_port = 1984;
    m_update_read_buffer_size  = 10*1024*1024;
    m_update_socket_timeout_ms = 1000;

    snprintf(m_plugin_config_path, sizeof(m_plugin_config_path),
            "%s", "./conf/plugin.config.json");

    m_cell_size = 4;
    m_bucket_size = 20;
    m_headlist_size = 0x1000000;
    m_memblocknumlistsize = 8;
    for (uint32_t i=0; i<8; i++)
    {
        m_memblocknumlist[i] = (i==0)? 4096: m_memblocknumlist[i-1]/2;
    }

    // CUSTOMIZE CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(configpath);
    if (! reader.parse(in, root))
    {
        FATAL("json format error. USING DEFAULT CONFIG.");
        return;
        //        MyToolThrow("json format error.");
    }

    Json::Value logConfig = root["LOG"];
    if (! logConfig.isNull()) {
        m_log_level = logConfig[m_StrLogLevel].isNull() ? m_log_level : logConfig[m_StrLogLevel].asInt();
        m_log_size  = logConfig[m_StrLogSize].isNull()  ? m_log_size  : logConfig[m_StrLogSize].asInt();
        m_log_name  = logConfig[m_StrLogName].isNull()  ? m_log_name  : logConfig[m_StrLogName].asCString();
        SETLOG(m_log_level, m_log_size, m_log_name);
    }

    Json::Value qSrvConfig = root["QUERY_SERVER"];
    if (! qSrvConfig.isNull()) {
        m_pollsize    = qSrvConfig[m_StrEpollSize].isNull() ? m_pollsize : qSrvConfig[m_StrEpollSize].asInt();
        m_wtimeout_ms = qSrvConfig[m_StrQuerySendTimeOut].isNull() ? m_wtimeout_ms : qSrvConfig[m_StrQuerySendTimeOut].asInt();
        m_rtimeout_ms = qSrvConfig[m_StrQueryRecvTimeOut].isNull() ? m_rtimeout_ms : qSrvConfig[m_StrQueryRecvTimeOut].asInt();
        m_query_port  = qSrvConfig[m_StrQueryPort].isNull() ? m_query_port : qSrvConfig[m_StrQueryPort].asInt();
        m_service_thread_num = qSrvConfig[m_StrServiceThreadNum].isNull() ?
            m_service_thread_num : qSrvConfig[m_StrServiceThreadNum].asInt();
        m_thread_buffer_size = qSrvConfig[m_StrThreadBufferSize].isNull() ?
            m_thread_buffer_size : qSrvConfig[m_StrThreadBufferSize].asInt();
    }

    Json::Value uSrvConfig = root["UPDATE_SERVER"];
    if (! uSrvConfig.isNull()) {
        m_update_port = uSrvConfig[m_StrUpdatePort].isNull() ? m_update_port : uSrvConfig[m_StrUpdatePort].asInt();
        m_update_read_buffer_size = uSrvConfig[m_StrUpdateReadBufferSize].isNull() ?
            m_update_read_buffer_size : uSrvConfig[m_StrUpdateReadBufferSize].asInt();
        m_update_socket_timeout_ms = uSrvConfig[m_StrUpdateSocketTimeOutMS].isNull() ?
            m_update_socket_timeout_ms : uSrvConfig[m_StrUpdateSocketTimeOutMS].asInt();
    }

    Json::Value indexConfig = root["INDEX"];
    if (! indexConfig.isNull()) {
        m_cell_size   = indexConfig[m_StrCellSize].isNull() ? m_cell_size : indexConfig[m_StrCellSize].asInt();
        m_bucket_size = indexConfig[m_StrBucketSize].isNull() ? m_bucket_size : indexConfig[m_StrBucketSize].asInt();
        m_headlist_size = indexConfig[m_StrHeadListSize].isNull() ? m_headlist_size : indexConfig[m_StrHeadListSize].asInt();
        Json::Value memblocknumlist = indexConfig["MemBlockNumList"];
        if (!memblocknumlist.isNull() && !memblocknumlist.isArray())
        {
            m_memblocknumlistsize = memblocknumlist.size();
            for (uint32_t i=0; i<m_memblocknumlistsize; i++)
            {
                m_memblocknumlist[i] = memblocknumlist[i].asInt();
                ROUTN("[%u] num[%u]", i, m_memblocknumlist[i]);
            }
        }
    }

}
