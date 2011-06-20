#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

class Config
{
	private:
		static const char* m_StrLogLevel;
		static const char* m_StrLogSize;
		static const char* m_StrLogName;
		static const char* m_StrSendTimeOut;
		static const char* m_StrRecvTimeOut;
		static const char* m_StrListenPort;
		static const char* m_StrServiceThreadNum;
		static const char* m_StrThreadBufferSize;
		static const char* m_StrEpollSize;

		// socket time out
		int m_rtimeout_ms;
		int m_wtimeout_ms;

		// log config
		int  m_log_level; // DEBUG, ROUTN, ALARM, FATAL
		int  m_log_size;  // max log size
		const char* m_log_name; // logname

        // server config
        int m_pollsize; // socket pool size
		int m_listen_port;
		int m_thread_buffer_size;
		int m_service_thread_num;

		Config();
		Config(const Config&);
	public:
		Config(const char* configpath);
		~Config(){}

		uint32_t RTimeMS()  const { return m_rtimeout_ms; }
		uint32_t WTimeMS()  const { return m_wtimeout_ms; }

		uint32_t ListenPort() const { return m_listen_port; }
		uint32_t PollSize() const { return m_pollsize; }
		uint32_t ServiceThreadNum() const { return m_service_thread_num; }
		uint32_t ThreadBufferSize() const { return m_thread_buffer_size; }
};

#endif
