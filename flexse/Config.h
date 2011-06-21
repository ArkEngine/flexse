#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

class Config
{
	private:
		static const char* m_StrLogLevel;
		static const char* m_StrLogSize;
		static const char* m_StrLogName;

		static const char* m_StrQuerySendTimeOut;
		static const char* m_StrQueryRecvTimeOut;
		static const char* m_StrQueryPort;
		static const char* m_StrServiceThreadNum;
		static const char* m_StrThreadBufferSize;
		static const char* m_StrEpollSize;

        static const char* m_StrUpdatePort;
        static const char* m_StrUpdateReadBufferSize;
        static const char* m_StrUpdateReadTimeOutMS;

		// log config
		int  m_log_level; // DEBUG, ROUTN, ALARM, FATAL
		int  m_log_size;  // max log size
		const char* m_log_name; // logname

        // query server config
        int m_pollsize; // socket pool size
		int m_query_port;
		int m_thread_buffer_size;
		int m_service_thread_num;
		int m_rtimeout_ms;
		int m_wtimeout_ms;

        // update server config
        int m_update_port;
        int m_update_read_buffer_size;
        int m_update_read_time_out_ms;

		Config();
		Config(const Config&);
	public:
		Config(const char* configpath);
		~Config(){}

		uint32_t RTimeMS()  const { return m_rtimeout_ms; }
		uint32_t WTimeMS()  const { return m_wtimeout_ms; }
		uint16_t QueryPort() const { return m_query_port; }
		uint32_t PollSize() const { return m_pollsize; }
		uint32_t ServiceThreadNum() const { return m_service_thread_num; }
		uint32_t ThreadBufferSize() const { return m_thread_buffer_size; }

        uint16_t UpdatePort() { return m_update_port; } 
        uint32_t UpdateReadBufferSize() { return m_update_read_buffer_size; } 
        uint32_t UpdateReadTimeOutMS()  { return m_update_read_time_out_ms; } 
};

#endif
