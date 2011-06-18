#ifndef __MYLOG_H_
#define __MYLOG_H_
#include <pthread.h>
#include <stdint.h>
/*
 * Name: mylog - The Mutil-Thread Logger
 * Features:
 * (1) Mutil-Thread
 * (2) 真真真真真, 真 1G
 * (3) 真4真真�:
 *     -0- DEBUG, 真真
 *     -1- ROUTN, 真真
 *     -2- ALARM, 真真真真真真真�
 *     -3- FATAL, 真真真真真真�
 * (4) 真真真真真真真真真真真真真
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
        uint32_t m_level; // 晩崗雫艶
        uint32_t m_file_size; // 晩崗寄弌
        char m_path[LogNameMaxLen]; // 晩崗揃抄
        int m_log_fd; // 輝念晩崗議猟周宙峰憲
        pthread_mutex_t m_lock; // 亟猟周迄
        char m_timebuff[TimeBuffMaxLen];
        const char* GetTimeNow();
        void LogFileSizeCheck();
        void LogFileCheckOpen(); // 泌惚音贋壓祥幹秀
        void Mkdirs(const char* dir); // 弓拷議幹秀猟周斜
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
