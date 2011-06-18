#ifndef __MYLOG_H_
#define __MYLOG_H_
#include <pthread.h>
#include <stdint.h>
/*
 * Name: mylog - The Mutil-Thread Logger
 * Features:
 * (1) Mutil-Thread
 * (2) ����������, �� 1G
 * (3) ��4�����:
 *     -0- DEBUG, ����
 *     -1- ROUTN, ����
 *     -2- ALARM, ���������������
 *     -3- FATAL, �������������
 * (4) ��������������������������
 */
class mylog
{
    private:
        static const uint32_t LogNameMaxLen = 128;
        static const uint32_t LogFileMaxSize = 1024*1024*1024;
        static const uint32_t TimeBuffMaxLen = 32;
        static const uint32_t LogContentMaxLen = 2048;
        static const char* const strLogName;
        static const char* LevelTag[];
        uint32_t m_level; // ��־����
        uint32_t m_file_size; // ��־��С
        char m_path[LogNameMaxLen]; // ��־·��
        int m_log_fd; // ��ǰ��־���ļ�������
        pthread_mutex_t m_lock; // д�ļ���
        char m_timebuff[TimeBuffMaxLen];
        const char* GetTimeNow();
        void LogFileSizeCheck();
        void LogFileCheckOpen(); // ��������ھʹ���
        void Mkdirs(const char* dir); // �ݹ�Ĵ����ļ���
        void WriteNByte(int fd, const char* buff, uint32_t size);
        mylog();
    public:
        enum { DEBUG = 0, ROUTN, ALARM, FATAL, };
        mylog(const uint32_t level, const uint32_t size, const char* logname);
        ~mylog();
        void WriteLog(const uint32_t mylevel, const char* file,
                const uint32_t line, const char* func, const char *format, ...);
};

#endif
