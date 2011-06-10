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
#include "FileOutputer.h"

FileOutputer :: FileOutputer(const char* logname)
{
    pthread_mutex_init(&m_lock, NULL);
    if (logname[0] == 0)
    {
        while(0 != raise(SIGKILL)) {}
    }
    snprintf(m_path, sizeof(m_path), "%s", logname);
    m_fd = -1;
    mLastDay = 0;
}

FileOutputer :: ~FileOutputer()
{}

void FileOutputer :: WriteNByte(const char* buff, const uint32_t size)
{
    int32_t  left   = size;
    uint32_t offset = size;
    pthread_mutex_lock(&m_lock);
    FileCheckOpen();
    while (left > 0)
    {
        int len = write(m_fd, buff+offset-left, left);
        if (len == -1 || len == 0)
        {
            break;
        }
        left -= len;
    }
    pthread_mutex_unlock(&m_lock);
}
void FileOutputer :: WriteFile(const char *format, ...)
{
    pthread_mutex_lock(&m_lock);
    FileCheckOpen();
    va_list args;
    va_start(args, format);
    char buff[ContentMaxLen];
    uint32_t pos = 0;
    vsnprintf(&buff[pos], ContentMaxLen -2 , format, args);
    va_end(args);
    pos = strlen(buff);
    buff[pos] = '\n';
    buff[pos+1] = '\0';
    WriteNByte(buff, pos+1);
    pthread_mutex_unlock(&m_lock);
}

void FileOutputer :: FileCheckOpen()
{
    char  curpath[FileNameMaxLen];
    uint32_t daynow = GetTimeNow();
    if (daynow != mLastDay)
    {
        close(m_fd);
        snprintf(curpath, sizeof(curpath), "%s.%d", m_path, daynow);
        m_fd = open(curpath, O_WRONLY|O_CREAT|O_APPEND, S_IRWXU|S_IRWXG|S_IRWXO);
        if (m_fd < 0)
        {
            fprintf(stderr, "FILE[%s:%u] create logfile[%s] fail. msg[%m]\n", __FILE__, __LINE__, curpath);
            while(0 != raise(SIGKILL)){}
        }
        mLastDay = daynow;
    }
}

uint32_t FileOutputer :: GetTimeNow()
{
    time_t now;
    struct tm mytm;
    uint32_t timenow = 0;

    time(&now);
    localtime_r(&now, &mytm);
    mytm.tm_year += 1900;
    // 2011年12月29日15时的时间串为
    // 2011122915
    timenow = mytm.tm_year*1000000 + (mytm.tm_mon+1)*10000 + mytm.tm_mday*100 + mytm.tm_hour;
    return  timenow;
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
