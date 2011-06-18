#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <stdint.h>

/*
 * Name : Config
 * Implement: ��std��map������ "key : value"�������ж���
 * ȥ������Ŀո��ַ�������map<string, string>��
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

		// �׽ӿڵĶ�д��ʱ
		int m_rtimeout_ms;
		int m_wtimeout_ms;

		// ��־����
		int  m_log_level; // ��־����
		int  m_log_size;  // ��־�ļ����ֵ
		char m_log_path[MaxFilePathLen]; // ��־·��

        // epoll��wait����
        int m_pollsize;
		// �����˿�
		int m_listen_port;
		// �߳��У����ڶ�дbuffer�����ֵ
		int m_thread_buffer_size;
		// �����߳���Ŀ
		int m_service_thread_num;
		Config();
		// �Ƿ���ע��, ע���� '#' ��ͷ��ǰ������пո�tab
		bool IsComment(const char* line);
		// �����ַ���ǰ��Ŀո�tab
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
