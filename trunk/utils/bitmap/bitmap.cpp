#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include "mylog.h"
#include "bitmap.h"
#include "MyException.h"

bitmap :: bitmap (const char* dir, const char* file, const uint32_t filesize)
{
    m_filesize = filesize;
    MySuicideAssert(m_filesize > 0);
    char filename[MAX_FILENAME_LENGTH];
    snprintf(filename, sizeof(filename), "%s/%s", dir, file);
    mode_t amode = (0 == access(filename, F_OK)) ? O_RDWR : O_RDWR|O_CREAT ;
    m_fd = open(filename, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    MySuicideAssert(m_fd != -1);
    MySuicideAssert (-1 != lseek(m_fd, filesize-1, SEEK_SET));
    char tmpchar;
    if (1 != read(m_fd, &tmpchar, 1))
    {
        MySuicideAssert ( 1 == write(m_fd, "", 1));
        printf("char[null]\n");
    }
    else
    {
        MySuicideAssert (-1 != lseek(m_fd, filesize-1, SEEK_SET));
        MySuicideAssert ( 1 == write(m_fd, &tmpchar, 1));
        printf("char[%c]\n", tmpchar);
    }
    MySuicideAssert (MAP_FAILED != (puint = (uint32_t*)mmap(0, m_filesize,
                    PROT_READ | PROT_WRITE, MAP_SHARED, m_fd, 0)));
}

bitmap :: ~bitmap()
{
    msync(puint, m_filesize, MS_SYNC);
    close(m_fd);
}
