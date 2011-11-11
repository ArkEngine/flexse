#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

class Config
{
	private:
		static const char* const m_StrLogLevel;

		static const char* const m_StrQuerySendTimeOut;
		static const char* const m_StrQueryRecvTimeOut;
		static const char* const m_StrQueryPort;
		static const char* const m_StrServiceThreadNum;
		static const char* const m_StrThreadBufferSize;
		static const char* const m_StrEpollSize;
//        static const char* const m_StrQueryMemBlockNumList;
//        static const char* const m_StrQueryMemBlockSizList;

        static const char* const m_StrUpdatePort;
        static const char* const m_StrUpdateReadBufferSize;
        static const char* const m_StrUpdateSocketTimeOutMS;

		// log config
		uint32_t  m_log_level; // DEBUG, ROUTN, ALARM, FATAL

        // query server config
        uint32_t m_pollsize; // socket pool size
		uint16_t m_query_port;
		uint32_t m_thread_buffer_size;
		uint32_t m_service_thread_num;
		uint32_t m_rtimeout_ms;
		uint32_t m_wtimeout_ms;
//        uint32_t m_querymemblocknumlist[8];
//        uint32_t m_querymemblocksizlist[8];
//        uint32_t m_querymemblockcatnum;

        // update server config
        uint16_t m_update_port;
        uint32_t m_update_read_buffer_size;
        uint32_t m_update_socket_timeout_ms;

        char m_plugin_config_path[128];

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
//		uint32_t QueryMemblockCatNum() const { return m_querymemblockcatnum; }
//		const uint32_t* QueryMemblockNumList() const { return m_querymemblocknumlist; }
//		const uint32_t* QueryMemblockSizList() const { return m_querymemblocksizlist; }

        uint16_t UpdatePort() { return m_update_port; } 
        uint32_t UpdateReadBufferSize() { return m_update_read_buffer_size; } 
        uint32_t UpdateSocketTimeOutMS()  { return m_update_socket_timeout_ms; } 

        const char* PluginConfigPath() { return m_plugin_config_path;}
};

#endif
