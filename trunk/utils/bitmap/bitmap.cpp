#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
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
    }
    else
    {
        MySuicideAssert (-1 != lseek(m_fd, filesize-1, SEEK_SET));
        MySuicideAssert ( 1 == write(m_fd, &tmpchar, 1));
    }
    m_prot = PROT_READ | PROT_WRITE;
    m_flags = MAP_SHARED;
    MySuicideAssert (MAP_FAILED != (puint = (uint32_t*)mmap(0, m_filesize,
                    m_prot, m_flags, m_fd, 0)));
}

/* fd 要可写，因此需要使用O_RDWR打开 */
bitmap :: bitmap(int fd, int prot, int flags)
{
    m_fd = -1;
    m_prot = prot;
    m_flags = flags;
    struct stat fs;
    MySuicideAssert ( 0 == fstat(fd, &fs));
    MySuicideAssert(fs.st_size > 0);
    m_filesize = (uint32_t)(fs.st_size);
    MySuicideAssert (-1 != lseek(fd, m_filesize-1, SEEK_SET));
    char tmpchar;
    if (1 != read(fd, &tmpchar, 1))
    {
        MySuicideAssert ( 1 == write(fd, "", 1));
    }
    else
    {
        MySuicideAssert (-1 != lseek(fd, m_filesize-1, SEEK_SET));
        MySuicideAssert ( 1 == write(fd, &tmpchar, 1));
    }
    MySuicideAssert (MAP_FAILED != (puint = (uint32_t*)mmap(0, m_filesize,
                    m_prot, m_flags, fd, 0)));

}

bitmap :: ~bitmap()
{
    if (PROT_WRITE & m_prot)
    {
        msync(puint, m_filesize, MS_SYNC);
    }
    if (m_fd != -1)
    {
        close(m_fd);
        m_fd = -1;
    }
    munmap(puint, m_filesize);
}
