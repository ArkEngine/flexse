#include <stdarg.h>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include "MyException.h"
#include "mylog.h"

const char* const mylog::strLogName = "default";
const char* mylog::LevelTag[] = { "PRINT", "DEBUG", "ROUTN", "ALARM", "FATAL", };

mylog* mylog::m_mylog = new mylog();

mylog :: mylog()
{
    pthread_mutex_init(&m_lock, NULL);
    m_level = ROUTN;
    snprintf(m_path, sizeof(m_path), "./log/%s.log", strLogName);
    m_log_fd = -1;
    m_LastDayTime = DayTimeNow();
    LogFileCheckOpen();
}

mylog* mylog::getInstance()
{
    return m_mylog;
}

void mylog :: setlog(const uint32_t level, const char* logname)
{
    m_level = (level > (uint32_t)FATAL) ? (uint32_t)ROUTN : level;
    const char* mylogname = (logname != NULL && logname[0] != 0 ) ? logname : strLogName;
    MySuicideAssert(NULL == strchr(mylogname, '/'));

    char  defaultlogname[LogNameMaxLen];
    snprintf(defaultlogname, sizeof(defaultlogname), "./log/%s.log", strLogName);
    if (0 != strcmp(mylogname, strLogName))
    {
        // 把默认的日志删除
        close(m_log_fd);
        m_log_fd = -1;
        char  logpath[LogNameMaxLen];
        snprintf(logpath, sizeof(logpath), "%s.%d", defaultlogname, m_LastDayTime);
        remove(logpath);
    }
    snprintf(m_path, sizeof(m_path), "./log/%s.log", mylogname);
    LogFileCheckOpen();
    fprintf(stderr, "START %s mylog Level[%u] SetLevel[%u] Path[%s] Size[%u]\n",
            GetTimeNow(), m_level, level, m_path, LogFileMaxSize);
    return;
}

void mylog :: WriteNByte(const int fd, const char* buff, const uint32_t size)
{
    int32_t  left   = size;
    uint32_t offset = size;
    while (left > 0)
    {
        int len = (int)write(fd, buff+offset-left, left);
        if (len == -1 || len == 0)
        {
            return;
        }
        left -= len;
    }
}
void mylog :: WriteLog(const uint32_t mylevel, const char* file,
        const uint32_t line, const char* func, const char *format, ...)
{
    pthread_mutex_lock(&m_lock);
    //    LogFileCheckOpen();
    LogFileSwitchCheck();
    char buff[LogContentMaxLen];
    uint32_t pos = 0;
    if (mylevel < m_level) {
        pthread_mutex_unlock(&m_lock);
        return;
    }
    uint32_t  level = (mylevel > (uint32_t)FATAL) ? (uint32_t)FATAL : mylevel;
    pos =  snprintf(&buff[pos], LogContentMaxLen-pos, "%s %s %lu file[%s:%u] func[%s] ",
            LevelTag[level], GetTimeNow(), pthread_self(), file, line, func);
    va_list args;
    va_start(args, format);
    pos += vsnprintf(&buff[pos], LogContentMaxLen-pos-1, format, args);
    va_end(args);
    pos = (pos > (LogContentMaxLen - 2)) ? LogContentMaxLen - 2 : pos;
    buff[pos] = '\n';
    buff[++pos] = '\0';
    MySuicideAssert(pos < LogContentMaxLen);
//    fprintf(stdout, "%s", buff);
    if (mylevel == PRINT)
    {
        fprintf(stdout, "%s", buff);
        pthread_mutex_unlock(&m_lock);
        return;
    }
    if ((m_cur_logsize + pos) > LogFileMaxSize)
    {
         m_cur_logsize = ( 0 == ftruncate(m_log_fd, 0)) ? 0 : m_cur_logsize;
    }
    WriteNByte(m_log_fd, buff, pos);
    m_cur_logsize += pos;
    pthread_mutex_unlock(&m_lock);
}

void mylog :: LogFileCheckOpen()
{
    char  tmppath[LogNameMaxLen];
    char  curpath[LogNameMaxLen];
    if (NULL == getcwd(curpath, sizeof(curpath))) {
        fprintf(stderr, "Get Current Dir Fail.\n");
        return;
    }
    char  logpath[LogNameMaxLen];
    snprintf(logpath, sizeof(logpath), "%s.%d", m_path, DayTimeNow());
    char* p = NULL;
    if (0 != access(logpath,F_OK)) {
        snprintf(tmppath, sizeof(tmppath), "%s", logpath);
        p = strrchr(tmppath, '/');
        if (p) {
            *p = '\0';
            Mkdirs(tmppath);
        }
    }
    if (m_log_fd >= 0){
        close(m_log_fd);
        m_log_fd = -1;
    }
    chdir(curpath);
    m_log_fd = open(logpath, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    if (m_log_fd < 0) {
        fprintf(stderr, "FILE[%s:%u] create logfile[%s] fail. msg[%m]\n",
                __FILE__, __LINE__, logpath);
        while(0 != raise(SIGKILL)){}
    }
    m_cur_logsize = (int)lseek(m_log_fd, SEEK_END, 0);;
}

void mylog :: LogFileSwitchCheck()
{
    uint32_t timenow = DayTimeNow();
    if (timenow != m_LastDayTime)
    {
        char  curpath[LogNameMaxLen];
        close(m_log_fd);
        snprintf(curpath, sizeof(curpath), "%s.%d", m_path, timenow);
        m_log_fd = open(curpath, O_WRONLY|O_CREAT|O_APPEND, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (m_log_fd < 0)
        {
            fprintf(stderr, "FILE[%s:%u] create logfile[%s] fail. msg[%m]\n", __FILE__, __LINE__, curpath);
            while(0 != raise(SIGKILL)){}
        }
        m_LastDayTime = timenow;
    }
}

const char* mylog :: GetTimeNow()
{
    time_t now;
    struct tm mytm;

    m_timebuff[0] = 0;
    time(&now);
    localtime_r(&now, &mytm);
    mytm.tm_year += 1900;
    sprintf(m_timebuff, "%02d-%02d %02d:%02d:%02d",
            mytm.tm_mon+1, mytm.tm_mday, mytm.tm_hour, mytm.tm_min, mytm.tm_sec);

    return  m_timebuff;
}

void mylog :: Mkdirs(const char* dir)
{
    char tmp[1024];
    char *p = NULL;
    if (strlen(dir) == 0 || dir == NULL) {
        return;
    }
    memset(tmp, '\0', sizeof(tmp));
    strncpy(tmp, dir, strlen(dir));
    if (tmp[0] == '/') {
        p = strchr(tmp + 1, '/');
    } else {
        p = strchr(tmp, '/');
    }

    if (p) {
        *p = '\0';
        mkdir(tmp, 0777);
        chdir(tmp);
    } else {
        mkdir(tmp, 0777);
        chdir(tmp);
        return;
    }
    Mkdirs(p + 1);
}

uint32_t mylog :: DayTimeNow()
{
    time_t now;
    struct tm mytm;
    uint32_t timenow = 0;

    time(&now);
    localtime_r(&now, &mytm);
    mytm.tm_year += 1900;
    // 2011122915
    timenow = mytm.tm_year*1000000 + (mytm.tm_mon+1)*10000 + mytm.tm_mday*100 + mytm.tm_hour;
    return  timenow;
}
