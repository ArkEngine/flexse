#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>
#include <string>
#include <set>
#include "filelinkblock.h"
#include "myutils.h"
#include "mylog.h"
#include "MyException.h"

using namespace std;
using namespace flexse;

class Config
{
	private:
		static const char* const m_StrQueuePath;
		static const char* const m_StrQueueName;

		static const char* const m_StrLogLevel;
		static const char* const m_StrLogSize;
		static const char* const m_StrLogName;

		static const char* const m_StrQuerySendTimeOut;
		static const char* const m_StrQueryRecvTimeOut;
		static const char* const m_StrCiPort;
		static const char* const m_StrServiceThreadNum;
		static const char* const m_StrThreadBufferSize;
		static const char* const m_StrEpollSize;
		static const char* const m_StrNeedIpControl;

        // queue congi
        const char* m_queue_path;
        const char* m_queue_name;

		// log config
		uint32_t  m_log_level; // DEBUG, ROUTN, ALARM, FATAL
		uint32_t  m_log_size;  // max log size
		const char* m_log_name; // logname

        // ci server config
        uint32_t m_pollsize; // socket pool size
		uint32_t m_ci_port;
		uint32_t m_thread_buffer_size;
		uint32_t m_service_thread_num;
		uint32_t m_rtimeout_ms;
		uint32_t m_wtimeout_ms;

        // queue handle
        filelinkblock* m_queue;
        
        // ip set
        set<string> m_ip_set;
        char        m_ip_file[128];
//        uint32_t    m_cur_no;
        bool        m_need_ip_control;

		Config();
		Config(const Config&);
	public:
		Config(const char* configpath);
		~Config(){}

		uint32_t RTimeMS()  const;
		uint32_t WTimeMS()  const;
		uint16_t QueryPort() const;
		uint32_t PollSize() const;
		uint32_t ServiceThreadNum() const;
		uint32_t ThreadBufferSize() const;
        filelinkblock* GetQueue();
        bool     IpNotValid(const char* str_ip);
        bool     NeedIpControl() const;
        void     LoadIpSet();
//        void     TryToReloadIpSet();
};

#endif
