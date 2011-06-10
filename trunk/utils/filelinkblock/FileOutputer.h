#ifndef  __FILEOUTPUTER_H_
#define  __FILEOUTPUTER_H_

#include <pthread.h>
#include <stdint.h>

class FileOutputer
{
    private:
        static const uint32_t FileNameMaxLen  = 128;
        static const uint32_t ContentMaxLen = 8192;
        static const char* const strFileName;
        char m_path[FileNameMaxLen];
        int m_fd;
        uint32_t mLastDay;
        pthread_mutex_t m_lock;
        uint32_t GetTimeNow();
        void FileCheckOpen();
        FileOutputer();
        FileOutputer(const FileOutputer&);
    public:
        FileOutputer(const char* logname);
        ~FileOutputer();
        void WriteFile(const char *format, ...);
        void WriteNByte(const char* buff, const uint32_t size);
};

#endif  //__FILEOUTPUTER_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
