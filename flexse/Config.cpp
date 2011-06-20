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
#include "Config.h"

using namespace std;

const char* Config::m_StrLogLevel = "LogLevel";
const char* Config::m_StrLogSize  = "LogSize";
const char* Config::m_StrLogName  = "LogName";
const char* Config::m_StrSendTimeOut = "SendTimeOut_MS";
const char* Config::m_StrRecvTimeOut = "RecvTimeOut_MS";
const char* Config::m_StrListenPort  = "ListenPort";
const char* Config::m_StrServiceThreadNum = "ServiceThreadNum";
const char* Config::m_StrThreadBufferSize = "ThreadBufferSize";
const char* Config::m_StrEpollSize = "EpollSize";

Config::Config(const char* configpath)
{
    Json::Value root;
    Json::Reader reader;
    ifstream in(configpath);
    if (! reader.parse(in, root))
    {
        FATAL("json format error.");
        MyToolThrow("json format error.");
    }

    m_log_level = mylog :: ROUTN;
    m_log_size  = 1 << 30;
    m_log_name  = "flexse";
    Json::Value logConfig = root["LOG"];
    if (! logConfig.isNull()) {
        m_log_level = logConfig[m_StrLogLevel].isNull() ? m_log_level : logConfig[m_StrLogLevel].asInt();
        m_log_size  = logConfig[m_StrLogSize].isNull()  ? m_log_size  : logConfig[m_StrLogSize].asInt();
        m_log_name  = logConfig[m_StrLogName].isNull()  ? m_log_name  : logConfig[m_StrLogName].asCString();
        SETLOG(m_log_level, m_log_size, m_log_name);
    }
    else
    {
        SETLOG(m_log_level, m_log_size, m_log_name);
    }

    Json::Value srvConfig = root["SERVER"];
    m_pollsize           = 128;
    m_wtimeout_ms        = 1000;
    m_rtimeout_ms        = 1000;
    m_listen_port        = 1983;
    m_service_thread_num = 20;
    m_thread_buffer_size = 10*1024*1024;

    if (! srvConfig.isNull()) {
        m_pollsize    = srvConfig[m_StrEpollSize].isNull() ? m_pollsize : srvConfig[m_StrEpollSize].asInt();
        m_wtimeout_ms = srvConfig[m_StrSendTimeOut].isNull() ? m_wtimeout_ms : srvConfig[m_StrSendTimeOut].asInt();
        m_rtimeout_ms = srvConfig[m_StrRecvTimeOut].isNull() ? m_rtimeout_ms : srvConfig[m_StrRecvTimeOut].asInt();
        m_listen_port = srvConfig[m_StrListenPort].isNull() ? m_listen_port : srvConfig[m_StrListenPort].asInt();
        m_service_thread_num = srvConfig[m_StrServiceThreadNum].isNull() ?
            m_service_thread_num : srvConfig[m_StrServiceThreadNum].asInt();
        m_thread_buffer_size = srvConfig[m_StrThreadBufferSize].isNull() ?
            m_thread_buffer_size : srvConfig[m_StrThreadBufferSize].asInt();
    }
}
