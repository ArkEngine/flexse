#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

class Config
{
	private:
		static const char* const m_StrLogLevel;
		static const char* const m_StrLogSize;
		static const char* const m_StrLogName;

		static const char* const m_StrQuerySendTimeOut;
		static const char* const m_StrQueryRecvTimeOut;
		static const char* const m_StrQueryPort;
		static const char* const m_StrServiceThreadNum;
		static const char* const m_StrThreadBufferSize;
		static const char* const m_StrEpollSize;

        static const char* const m_StrUpdatePort;
        static const char* const m_StrUpdateReadBufferSize;
        static const char* const m_StrUpdateSocketTimeOutMS;

        static const char* const m_StrCellSize;
        static const char* const m_StrBucketSize;
        static const char* const m_StrHeadListSize;
        static const char* const m_StrMemBlockNumList;

		// log config
		uint32_t  m_log_level; // DEBUG, ROUTN, ALARM, FATAL
		uint32_t  m_log_size;  // max log size
		const char* m_log_name; // logname

        // query server config
        uint32_t m_pollsize; // socket pool size
		uint16_t m_query_port;
		uint32_t m_thread_buffer_size;
		uint32_t m_service_thread_num;
		uint32_t m_rtimeout_ms;
		uint32_t m_wtimeout_ms;

        // update server config
        uint16_t m_update_port;
        uint32_t m_update_read_buffer_size;
        uint32_t m_update_socket_timeout_ms;

        // index config
        uint32_t m_cell_size;     // postinglist cell size
        uint32_t m_bucket_size;   // postinglist hash bucket size = 1 << m_bucket_size;
        uint32_t m_headlist_size; // postinglist headlist size;
        uint32_t m_memblocknumlist[32]; // postinglist memblocks config
        uint32_t m_memblocknumlistsize;

        char m_plugin_config_path[128];

		Config();
		Config(const Config&);
	public:
		Config(const char* configpath);
		~Config(){}

        const char* PluginConfigPath() { return m_plugin_config_path;}

		uint32_t RTimeMS()  const { return m_rtimeout_ms; }
		uint32_t WTimeMS()  const { return m_wtimeout_ms; }
		uint16_t QueryPort() const { return m_query_port; }
		uint32_t PollSize() const { return m_pollsize; }
		uint32_t ServiceThreadNum() const { return m_service_thread_num; }
		uint32_t ThreadBufferSize() const { return m_thread_buffer_size; }

        uint16_t UpdatePort() { return m_update_port; } 
        uint32_t UpdateReadBufferSize() { return m_update_read_buffer_size; } 
        uint32_t UpdateSocketTimeOutMS()  { return m_update_socket_timeout_ms; } 

        uint32_t CellSize() { return m_cell_size; }
        uint32_t BucketSize() { return m_bucket_size; }
        uint32_t HeadListSize() { return m_headlist_size; }
        const uint32_t* MemBlockNumList() { return m_memblocknumlist; }
        uint32_t MemBlockNumListSize() { return m_memblocknumlistsize; }
};

#endif
