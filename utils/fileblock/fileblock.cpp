#include <stdint.h>
#include <signal.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include "fileblock.h"
#include "mylog.h"
#include "MyException.h"

fileblock :: fileblock (const char* dir, const char* filename, const uint32_t cell_size)
    : m_cell_size(cell_size)
{
    snprintf(m_fb_dir,  sizeof(m_fb_dir),  "%s", dir);
    snprintf(m_fb_name, sizeof(m_fb_name), "%s.idx", filename);
    int32_t max_file_no = detect_file();
    m_max_file_no = (max_file_no < 0) ? 1 : max_file_no + 1;
    char tmpstr[128];
    for (uint32_t i=0; i<MAX_FILE_NO; i++)
    {
        m_fd[i] = -1;
    }
    for (uint32_t i=0; i<m_max_file_no; i++)
    {
        snprintf(tmpstr, sizeof(tmpstr), "%s/%s.%u", m_fb_dir, m_fb_name, i);
        mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
        m_fd[i] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        MyThrowAssert(m_fd[i] != -1);
    }
    m_max_num_per_file = MAX_FILE_SIZE / m_cell_size;
}

int32_t fileblock :: write(const uint32_t offset, const char* buff)
{
    uint32_t file_no  = offset / m_max_num_per_file;
    uint32_t inoffset = offset % m_max_num_per_file;
    if (file_no > MAX_FILE_NO)
    {
        FATAL("offset[%u] too big. m_max_num_per_file[%u] MAX_FILE_NO[%u]",
                offset, m_max_num_per_file, MAX_FILE_NO);
        return -1;
    }
    if (m_fd[file_no] == -1)
    {
        char tmpstr[128];
        snprintf(tmpstr, sizeof(tmpstr), "%s/%s.%d", m_fb_dir, m_fb_name, file_no);
        mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
        m_fd[file_no] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        MyThrowAssert(m_fd[file_no] != -1);
    }
    return pwrite(m_fd[file_no], buff, m_cell_size, inoffset * m_cell_size);
}
int32_t fileblock :: read(const uint32_t offset, char* buff, const uint32_t length)
{
    uint32_t file_no  = offset / m_max_num_per_file;
    uint32_t inoffset = offset % m_max_num_per_file;
    if (length < m_cell_size)
    {
        ALARM("length[%u] too short. m_cell_size[%u]",
                length, m_cell_size);
        return -1;
    }
    if (file_no > MAX_FILE_NO)
    {
        FATAL("offset[%u] too big. m_max_num_per_file[%u] MAX_FILE_NO[%u]",
                offset, m_max_num_per_file, MAX_FILE_NO);
        return -1;
    }
    if (m_fd[file_no] == -1)
    {
        char tmpstr[128];
        snprintf(tmpstr, sizeof(tmpstr), "%s/%s.%d", m_fb_dir, m_fb_name, file_no);
        mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
        m_fd[file_no] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        MyThrowAssert(m_fd[file_no] != -1);
        if (file_no >= m_max_file_no)
        {
            m_max_file_no = file_no + 1;
        }
    }
    return pread(m_fd[file_no], buff, m_cell_size, inoffset * m_cell_size);

}
int32_t fileblock :: remove()
{
    for (uint32_t i=0; i<MAX_FILE_NO; i++)
    {
        if (m_fd[i] >= 0)
        {
            char tmpstr[128];
            snprintf(tmpstr, sizeof(tmpstr), "%s/%s.%u", m_fb_dir, m_fb_name, i);
            unlink(tmpstr);
            close(m_fd[i]);
            m_fd[i] = -1;
        }
    }
    return 0;
}
void fileblock :: begin()
{
    m_it = 0;
}
int32_t fileblock :: write_next(char* buff)
{
    uint32_t cur_it = m_it ++ ;
    return write(cur_it, buff);
}
int32_t fileblock :: read_next(char* buff, const uint32_t length)
{
    uint32_t cur_it = m_it ++ ;
    return read(cur_it, buff, length);
}

int32_t fileblock::detect_file()
{
    DIR *dp;
    struct  dirent  *dirp;

    char prefix[128];
    snprintf(prefix, sizeof(prefix), "%s.", m_fb_name);

    if((dp = opendir(m_fb_dir)) == NULL)
    {
        FATAL( "can't open %s! msg[%m] ", m_fb_dir);
        MySuicideAssert(0);
    }

    int len = strlen(prefix);
    int max = -1;

    while((dirp = readdir(dp)) != NULL)
    {
        DEBUG( "%s", dirp->d_name);
        char* pn = strstr(dirp->d_name, prefix);
        if (pn != NULL)
        {
            const char* pp = &pn[len];
            const char* cc = pp;
            // 检查后缀是否全是数字
            while (isdigit(*cc)) { cc++; }
            if (*cc == '\0')
            {
                int n = atoi(&pn[len]);
                DEBUG( "-- %d", n);
                if (n > max)
                {
                    max = n;
                }
            }
            else
            {
                DEBUG( "^^ invalid file name : %s", dirp->d_name);
            }
        }
    }

    closedir(dp);
    return max;
}
