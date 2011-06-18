#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string>
#include <string.h>
#include <map>
#include <sys/types.h>
#include "Config.h"
#include "mylog.h"

using namespace std;

const char  Config::m_ChrConfigSeparator = ':';
const char* Config::m_StrLogLevel = "LogLevel";
const char* Config::m_StrLogSize  = "LogSize";
const char* Config::m_StrLogPath  = "LogPath";
const char* Config::m_StrSendTimeOut = "SendTimeOut_MS";
const char* Config::m_StrRecvTimeOut = "RecvTimeOut_MS";
const char* Config::m_StrListenPort  = "ListenPort";
const char* Config::m_StrServiceThreadNum = "ServiceThreadNum";
const char* Config::m_StrThreadBufferSize = "ThreadBufferSize";
const char* Config::m_StrEpollSize = "EpollSize";

bool Config::IsComment(const char* str)
{
	while (*str != '\t' && *str != ' ')
	{
		str++;
	}
	return (*str == '#') ? true : false;
}

char* Config::Strip(char* str)
{
	char* b = str;
	char* e = NULL;
	while (*b == '\t' || *b == ' ') {
		b++;
	}
	memmove(str, b, strlen(b));
	if (NULL != (e = strchr(str, ' '))) {
		*e = 0;
	}
	if (NULL != (e = strchr(str, '\t'))) {
		*e = 0;
	}
	if (NULL != (e = strchr(str, '\n'))) {
		*e = 0;
	}
	return str;
}

Config::Config(const char* configpath)
{
	FILE* fp = fopen(configpath, "r");
	if (fp == NULL)
	{
		fprintf(stderr, "FILE[%s] LINE[%u] read config[%s] fail. msg[%m]\n",
				__FILE__, __LINE__, configpath);
		while(0!=raise(SIGKILL)){}
	}
	char tmpstr[1024];
	map<string, string> configmap;
	map<string, string>::iterator mit;
	while (fgets(tmpstr, sizeof(tmpstr), fp))
	{
		if (IsComment(tmpstr)) // 是否是注释, 注释以 # 开头
		{
			continue;
		}
		char* p = NULL;
		p = strchr(tmpstr, m_ChrConfigSeparator);
		if (p != NULL) {
			*p = 0;
			++p;
			configmap[string(Strip(tmpstr))] = string(Strip(p));
			fprintf(stderr, "key[%s] value[%s]\n", tmpstr, p);
		}
	}
	fclose (fp);

	if (configmap.end() == (mit = configmap.find(string(m_StrLogLevel)))) {
		m_log_level = mylog::ROUTN;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrLogLevel, m_log_level);
	}
	else {
		m_log_level = atoi(mit->second.c_str());
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrLogSize)))) {
		m_log_size = 1024*1024*1024;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrLogSize, m_log_size);
	}
	else {
		m_log_size = atoi(mit->second.c_str());
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrLogPath)))) {
		snprintf (m_log_path, sizeof(m_log_path), "%s", "./log/"PROJNAME".log");
		fprintf(stderr, "Use Default: key[%s] value[%s]\n",
				m_StrLogPath, m_log_path);
	}
	else {
		snprintf (m_log_path, sizeof(m_log_path), "%s", mit->second.c_str());
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrEpollSize)))) {
		m_pollsize = 128;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrEpollSize, m_pollsize);
	}
	else {
		m_pollsize = atoi(mit->second.c_str());
	}


	if (configmap.end() == (mit = configmap.find(string(m_StrSendTimeOut)))) {
		m_wtimeout_ms = 1000;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrSendTimeOut, m_wtimeout_ms);
	}
	else {
		m_wtimeout_ms = atoi(mit->second.c_str());
		m_wtimeout_ms = m_wtimeout_ms < 50? 1000 : m_wtimeout_ms;
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrRecvTimeOut)))) {
		m_rtimeout_ms = 1000;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrRecvTimeOut, m_rtimeout_ms);
	}
	else {
		m_rtimeout_ms = atoi(mit->second.c_str());
		m_rtimeout_ms = m_rtimeout_ms < 50? 1000 : m_rtimeout_ms;
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrListenPort)))) {
		m_listen_port = 4231;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrListenPort, m_listen_port);
	}
	else {
		m_listen_port = atoi(mit->second.c_str());
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrServiceThreadNum)))) {
		m_service_thread_num = 10;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrServiceThreadNum, m_service_thread_num);
	}
	else {
		m_service_thread_num = atoi(mit->second.c_str());
		m_service_thread_num = m_service_thread_num < 1 ? 10 : m_service_thread_num; 
	}

	if (configmap.end() == (mit = configmap.find(string(m_StrThreadBufferSize)))) {
		m_thread_buffer_size = 1024*1024;
		fprintf(stderr, "Use Default: key[%s] value[%u]\n",
				m_StrThreadBufferSize, m_thread_buffer_size);
	}
	else {
		m_thread_buffer_size = atoi(mit->second.c_str());
		m_thread_buffer_size = m_thread_buffer_size < 1024*1024 ? 1024*1024 : m_thread_buffer_size; 
		m_thread_buffer_size = m_thread_buffer_size > 10*1024*1024 ? 1024*1024 : m_thread_buffer_size; 
	}
}
