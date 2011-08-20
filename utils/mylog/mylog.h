#ifndef __MYLOG_H_
#define __MYLOG_H_
#include <pthread.h>
#include <stdint.h>
/*
 * Name: mylog - The Mutil-Thread Logger
 * Features:
 * (1) Mutil-Thread
 * (2) logfile maxsize limited to 1G
 * (3) 4 level
 *     -0- DEBUG
 *     -1- ROUTN
 *     -2- ALARM
 *     -3- FATAL
 * (4) auto switch into new file when file inc upto 1G
 * (5) switch by time?
 */
class mylog
{
    private:
        static const uint32_t LogNameMaxLen = 128;
        static const uint32_t LogFileMaxSize = 2*1024*1024*1000;
        static const uint32_t TimeBuffMaxLen = 32;
        static const uint32_t LogContentMaxLen = 2048;
        static const char* const strLogName;
        static const char* LevelTag[];
        uint32_t m_level;
        uint32_t m_LastDayTime;
        char m_path[LogNameMaxLen];
        int m_log_fd;
        int m_cur_logsize;
        char m_timebuff[TimeBuffMaxLen];
        pthread_mutex_t m_lock;

        uint32_t DayTimeNow();
        const char* GetTimeNow();
        void LogFileSwitchCheck();
        void LogFileCheckOpen(); // 如果不存在就创建
        void Mkdirs(const char* dir); // 递归的创建文件夹
        void WriteNByte(int fd, const char* buff, uint32_t size);
        mylog();
        mylog(const mylog&);
        static mylog* m_mylog;
    public:
        enum { PRINT = 0, DEBUG, ROUTN, ALARM, FATAL, };
        static mylog* getInstance();
        void setlog(const uint32_t level, const char* logname);
        ~mylog();
        void WriteLog(const uint32_t mylevel, const char* file,
                const uint32_t line, const char* func, const char *format, ...);
};

#define SETLOG(level, logname) \
        mylog::getInstance()->setlog(level, logname)

#define FATAL(fmt, msg ...) \
        mylog::getInstance()->WriteLog(mylog::FATAL, __FILE__, __LINE__, __func__, fmt, ##msg)

#define ALARM(fmt, msg ...) \
        mylog::getInstance()->WriteLog(mylog::ALARM, __FILE__, __LINE__, __func__, fmt, ##msg)

#define ROUTN(fmt, msg ...) \
        mylog::getInstance()->WriteLog(mylog::ROUTN, __FILE__, __LINE__, __func__, fmt, ##msg)

#define DEBUG(fmt, msg ...) \
        mylog::getInstance()->WriteLog(mylog::DEBUG, __FILE__, __LINE__, __func__, fmt, ##msg)

#define PRINT(fmt, msg ...) \
        mylog::getInstance()->WriteLog(mylog::PRINT, __FILE__, __LINE__, __func__, fmt, ##msg)

#endif
