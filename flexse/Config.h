#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

/*
 * Name : Config
 * Implement: 用std的map把形如 "key : value"的配置行读入
 * 去掉多余的空格字符，放入map<string, string>中
 */
class Config
{
	private:
		static const uint32_t MaxFilePathLen = 64;
		static const char  m_ChrConfigSeparator;
		static const char* m_StrLogLevel;
		static const char* m_StrLogSize;
		static const char* m_StrLogPath;
		static const char* m_StrSendTimeOut;
		static const char* m_StrRecvTimeOut;
		static const char* m_StrListenPort;
		static const char* m_StrServiceThreadNum;
		static const char* m_StrThreadBufferSize;
		static const char* m_StrEpollSize;

		// 套接口的读写超时
		int m_rtimeout_ms;
		int m_wtimeout_ms;

		// 日志设置
		int  m_log_level; // 日志级别
		int  m_log_size;  // 日志文件最大值
		char m_log_path[MaxFilePathLen]; // 日志路径

        // epoll靠wait靠靠
        int m_pollsize;
		// 监听端口
		int m_listen_port;
		// 线程中，用于读写buffer的最大值
		int m_thread_buffer_size;
		// 服务线程数目
		int m_service_thread_num;
		Config();
		// 是否是注释, 注释以 '#' 开头，前面可以有空格，tab
		bool IsComment(const char* line);
		// 剥掉字符串前后的空格，tab
		char* Strip(char* str);
	public:
		Config(const char* configpath);
		~Config(){};
		uint32_t RTimeMS()  const { return m_rtimeout_ms; }
		uint32_t WTimeMS()  const { return m_wtimeout_ms; }

		uint32_t LogSize()  const { return m_log_size; }
		uint32_t LogLevel() const { return m_log_level; }
		uint32_t ListenPort() const { return m_listen_port; }
		uint32_t PollSize() const { return m_pollsize; }
		uint32_t ServiceThreadNum() const { return m_service_thread_num; }
		uint32_t ThreadBufferSize() const { return m_thread_buffer_size; }

		const char* LogPath() const { return m_log_path; }
};

#endif
