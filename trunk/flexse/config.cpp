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

const char* const Config::m_StrQuerySendTimeOut = "SendTimeOut_MS";
const char* const Config::m_StrQueryRecvTimeOut = "RecvTimeOut_MS";
const char* const Config::m_StrQueryPort   = "QueryPort";
const char* const Config::m_StrServiceThreadNum = "ServiceThreadNum";
const char* const Config::m_StrThreadBufferSize = "ThreadBufferSize";
const char* const Config::m_StrEpollSize = "EpollSize";
//const char* const Config::m_StrQueryMemBlockNumList = "QueryMemBlockNumList";
//const char* const Config::m_StrQueryMemBlockSizList = "QueryMemBlockSizList";

const char* const Config::m_StrUpdatePort = "UpdatePort";
const char* const Config::m_StrUpdateReadBufferSize = "ReadBufferSize";
const char* const Config::m_StrUpdateSocketTimeOutMS  = "SocketTimeOut_MS";

Config::Config(const char* configpath)
{
    // DEFAULT CONFIG
    m_log_level = mylog :: ROUTN;

    m_pollsize            = 128;
    m_wtimeout_ms         = 1000;
    m_rtimeout_ms         = 1000;
    m_query_port          = 1983;
    m_service_thread_num  = 20;
    m_thread_buffer_size  = 1*1024*1024;
//    m_querymemblockcatnum = 4;
//    m_querymemblocknumlist[0] = 5;
//    m_querymemblocksizlist[0] = 10*1024*1024;
//    m_querymemblocknumlist[1] = 10;
//    m_querymemblocksizlist[1] = 1*1024*1024;
//    m_querymemblocknumlist[2] = 20;
//    m_querymemblocksizlist[2] = 512*1024;
//    m_querymemblocknumlist[3] = 40;
//    m_querymemblocksizlist[3] = 128*1024;

    m_update_port = 1984;
    m_update_read_buffer_size  = 1*1024*1024;
    m_update_socket_timeout_ms = 1000;

    snprintf(m_plugin_config_path, sizeof(m_plugin_config_path),
            "%s", "./conf/plugin.config.json");

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
    }
    SETLOG(m_log_level, PROJNAME);

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
//        Json::Value mblocknumlist = qSrvConfig[m_StrQueryMemBlockNumList];
//        Json::Value mblocksizlist = qSrvConfig[m_StrQueryMemBlockSizList];
//        uint32_t tmpcatnum = 0;
//        if (!mblocknumlist.isNull() && mblocknumlist.isArray())
//        {
//            tmpcatnum = mblocknumlist.size();
//            for (uint32_t i=0; i<mblocknumlist.size(); i++)
//            {
//                m_querymemblocknumlist[i] = mblocknumlist[i].asInt();
//            }
//            MySuicideAssert(!mblocksizlist.isNull() && mblocksizlist.isArray());
//            MySuicideAssert(tmpcatnum == mblocksizlist.size());
//            for (uint32_t i=0; i<mblocksizlist.size(); i++)
//            {
//                m_querymemblocksizlist[i] = mblocksizlist[i].asInt();
//            }
//            m_querymemblockcatnum = tmpcatnum;
//        }
    }

    Json::Value uSrvConfig = root["UPDATE_SERVER"];
    if (! uSrvConfig.isNull()) {
        m_update_port = uSrvConfig[m_StrUpdatePort].isNull() ? m_update_port : uSrvConfig[m_StrUpdatePort].asInt();
        m_update_read_buffer_size = uSrvConfig[m_StrUpdateReadBufferSize].isNull() ?
            m_update_read_buffer_size : uSrvConfig[m_StrUpdateReadBufferSize].asInt();
        m_update_socket_timeout_ms = uSrvConfig[m_StrUpdateSocketTimeOutMS].isNull() ?
            m_update_socket_timeout_ms : uSrvConfig[m_StrUpdateSocketTimeOutMS].asInt();
    }
}
