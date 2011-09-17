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
using namespace flexse;

const char* const Config::m_StrQueuePath = "QueuePath";
const char* const Config::m_StrQueueName = "QueueName";

const char* const Config::m_StrLogLevel = "LogLevel";
const char* const Config::m_StrLogName  = "LogName";

const char* const Config::m_StrQuerySendTimeOut = "SendTimeOut_MS";
const char* const Config::m_StrQueryRecvTimeOut = "RecvTimeOut_MS";
const char* const Config::m_StrCiPort           = "CiPort";
const char* const Config::m_StrServiceThreadNum = "ServiceThreadNum";
const char* const Config::m_StrThreadBufferSize = "ThreadBufferSize";
const char* const Config::m_StrEpollSize        = "EpollSize";
const char* const Config::m_StrNeedIpControl    = "NeedIpControl";

Config::Config(const char* configpath)
{
    // DEFAULT CONFIG
    m_queue_path = "mqueue";
    m_queue_name = "mqueue";
    m_log_level = mylog :: ROUTN;
    m_log_name  = PROJNAME;

    m_pollsize           = 128;
    m_wtimeout_ms        = 1000;
    m_rtimeout_ms        = 1000;
    m_ci_port            = 1983;
    m_service_thread_num = 20;
    m_thread_buffer_size = 10*1024*1024;

    // ip set
//    m_cur_no = 0;
    // 默认关闭IP控制
    m_need_ip_control = false;
    snprintf(m_ip_file, sizeof(m_ip_file), "%s", "./conf/ip_list");

    // CUSTOMIZE CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(configpath);
    if (! reader.parse(in, root))
    {
        FATAL("json format error. USING DEFAULT CONFIG.");
        MyThrowAssert(0);
    }

    Json::Value logConfig = root["LOG"];
    if (! logConfig.isNull()) {
        m_log_level = logConfig[m_StrLogLevel].isNull() ? m_log_level : logConfig[m_StrLogLevel].asInt();
        m_log_name  = logConfig[m_StrLogName].isNull()  ? m_log_name  : logConfig[m_StrLogName].asCString();
    }
    SETLOG(m_log_level, m_log_name);

    Json::Value qSrvConfig = root["CI_SERVER"];
    if (! qSrvConfig.isNull()) {
        m_pollsize    = qSrvConfig[m_StrEpollSize].isNull() ? m_pollsize : qSrvConfig[m_StrEpollSize].asInt();
        m_wtimeout_ms = qSrvConfig[m_StrQuerySendTimeOut].isNull() ? m_wtimeout_ms : qSrvConfig[m_StrQuerySendTimeOut].asInt();
        m_rtimeout_ms = qSrvConfig[m_StrQueryRecvTimeOut].isNull() ? m_rtimeout_ms : qSrvConfig[m_StrQueryRecvTimeOut].asInt();
        m_ci_port  = qSrvConfig[m_StrCiPort].isNull() ? m_ci_port : qSrvConfig[m_StrCiPort].asInt();
        m_service_thread_num = qSrvConfig[m_StrServiceThreadNum].isNull() ?
            m_service_thread_num : qSrvConfig[m_StrServiceThreadNum].asInt();
        m_thread_buffer_size = qSrvConfig[m_StrThreadBufferSize].isNull() ?
            m_thread_buffer_size : qSrvConfig[m_StrThreadBufferSize].asInt();
        uint32_t tint = qSrvConfig[m_StrNeedIpControl].isNull() ? 0 : qSrvConfig[m_StrNeedIpControl].asInt();
        m_need_ip_control = (tint != 0);
    }

    Json::Value qQueueConfig = root["QUEUE"];
    if (! qQueueConfig.isNull()) {
        m_queue_path    = qQueueConfig[m_StrQueuePath].isNull() ? m_queue_path : qQueueConfig[m_StrQueuePath].asCString();
        m_queue_name    = qQueueConfig[m_StrQueueName].isNull() ? m_queue_name : qQueueConfig[m_StrQueueName].asCString();
    }
    m_queue = new filelinkblock(m_queue_path, m_queue_name, false);
}
uint32_t Config:: RTimeMS()  const
{
    return m_rtimeout_ms;
}
uint32_t Config::WTimeMS()  const
{
    return m_wtimeout_ms;
}
uint16_t Config::QueryPort() const
{
    return m_ci_port;
}
uint32_t Config::PollSize() const
{
    return m_pollsize;
}
uint32_t Config::ServiceThreadNum() const
{
    return m_service_thread_num;
}
uint32_t Config::ThreadBufferSize() const
{
    return m_thread_buffer_size;
}
filelinkblock* Config:: GetQueue() {
    return m_queue;
}
bool Config:: NeedIpControl() const
{
    return m_need_ip_control;
}
bool Config:: IpNotValid(const char* str_ip)
{
    return m_ip_set.end() == m_ip_set.find(str_ip);
}
void Config:: LoadIpSet()
{
    if (! m_need_ip_control)
    {
        return;
    }
    MyThrowAssert(0 == access(m_ip_file, F_OK));
    char tmpstr[128];
    FILE* fp = fopen(m_ip_file, "r");
    MyThrowAssert(fp != NULL);

    while(NULL != fgets(tmpstr, sizeof(tmpstr), fp))
    {
        if (is_comment(tmpstr))
        {
            continue;
        }
        else
        {
            char* ip = strip(tmpstr);
            ROUTN("ip[%s] ft[%u]", ip, m_ip_set.end() != m_ip_set.find(string("127.0.0.1")));
            MyThrowAssert(is_valid_ip(ip));
            m_ip_set.insert(string(ip));
        }
    }
    fclose(fp);
    ROUTN("ip set update done. from[%s] total[%u]", m_ip_file, m_ip_set.size());
}
//        void     TryToReloadIpSet()
//        {
//            // 如果 ip_list 和 ip_list.done 同时存在，则使用 ip_list
//            // 使用之后，mv 成 ip_list.done
//            char tmpstr[128];
//            if (m_ip_has_loaded)
//            {
//                if (0 == access(m_ip_file, F_OK))
//                {
//                    LoadIpSet(m_ip_file);
//                    m_cur_no = 1 - m_cur_no;
//
//                    snprintf(tmpstr, sizeof(tmpstr), "%s.done", m_ip_file);
//                    rename(m_ip_file, tmpstr);
//                    ROUTN("ip set update done. total[%u]",
//                            m_ip_file, m_ip_set[m_cur_no].size());
//                }
//            }
//            else
//            {
//                snprintf(tmpstr, sizeof(tmpstr), "%s.done", m_ip_file);
//                if (0 == access(tmpstr, F_OK))
//                {
//                    LoadIpSet(tmpstr);
//                    m_cur_no = 1 - m_cur_no;
//
//                    ROUTN("ip set update done. from[%s] total[%u]",
//                            tmpstr, m_ip_set[m_cur_no].size());
//                }
//            }
//            m_ip_has_loaded = true;
//        }

