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

const char* const fileblock :: FORMAT_FILE = "%s.idx.";
const char* const fileblock :: FORMAT_PATH = "%s/%s.idx.%u";

fileblock :: fileblock (const char* dir, const char* filename, const uint32_t cell_size)
    : m_cell_size(cell_size), m_cell_num_per_file(MAX_FILE_SIZE/cell_size)
{
    snprintf(m_fb_dir,  sizeof(m_fb_dir),  "%s", dir);
    snprintf(m_fb_name, sizeof(m_fb_name), "%s", filename);
    int32_t max_file_no = detect_file();
    m_max_file_no = (max_file_no < 0) ? 1 : max_file_no + 1;
    char tmpstr[128];
    for (uint32_t i=0; i<MAX_FILE_NO; i++)
    {
        m_fd[i] = -1;
    }
    for (uint32_t i=0; i<m_max_file_no; i++)
    {
        snprintf(tmpstr, sizeof(tmpstr), FORMAT_PATH, m_fb_dir, m_fb_name, i);
        mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
        m_fd[i] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (m_fd[i] != -1)
        {
            MyThrowAssert(0 == getfilesize(tmpstr)%m_cell_size);
        }
        if (i == m_max_file_no - 1)
        {
            m_last_file_offset = getfilesize(tmpstr);
        }
    }
}

int32_t fileblock :: set(const uint32_t offset, const void* buff)
{
    uint32_t file_no  = offset / m_cell_num_per_file;
    uint32_t inoffset = offset % m_cell_num_per_file;
    if (file_no > MAX_FILE_NO)
    {
        FATAL("offset[%u] too big. m_cell_num_per_file[%u] MAX_FILE_NO[%u]",
                offset, m_cell_num_per_file, MAX_FILE_NO);
        return -1;
    }
    if (m_fd[file_no] == -1)
    {
        char tmpstr[128];
        snprintf(tmpstr, sizeof(tmpstr), FORMAT_PATH, m_fb_dir, m_fb_name, file_no);
        mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
        m_fd[file_no] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        if (file_no == m_max_file_no)
        {
            m_max_file_no++;
            m_last_file_offset = (uint32_t)lseek(m_fd[file_no], 0, SEEK_END);
        }
    }
    MyThrowAssert(m_fd[file_no] != -1);
    if((file_no == m_max_file_no -1) && (inoffset * m_cell_size == m_last_file_offset))
    {
        m_last_file_offset += m_cell_size;
    }
    MyThrowAssert(m_cell_size == (uint32_t)pwrite(m_fd[file_no], buff, m_cell_size, inoffset * m_cell_size));
    return 0;
}
int32_t fileblock :: get(const uint32_t offset, void* buff, const uint32_t length)
{
    uint32_t file_no  = offset / m_cell_num_per_file;
    uint32_t inoffset = offset % m_cell_num_per_file;
    if (length < m_cell_size)
    {
        ALARM("length[%u] too short. m_cell_size[%u]",
                length, m_cell_size);
        return -1;
    }
    if (file_no >= m_max_file_no)
    {
        ALARM("offset[%u] too big. m_cell_num_per_file[%u] max_file_no[%u]",
                offset, m_cell_num_per_file, m_max_file_no);
        return -1;
    }

    if (m_fd[file_no] == -1)
    {
        char tmpstr[128];
        snprintf(tmpstr, sizeof(tmpstr), FORMAT_PATH, m_fb_dir, m_fb_name, file_no);
        mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
        m_fd[file_no] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    }

    MyThrowAssert(m_fd[file_no] != -1);
    return pread(m_fd[file_no], buff, m_cell_size, inoffset * m_cell_size);

}

int32_t fileblock :: get(const uint32_t offset, const uint32_t count, void* buff, const uint32_t length)
{
    uint32_t file_no  = offset / m_cell_num_per_file;
    uint32_t inoffset = offset % m_cell_num_per_file;
    if (length < count * m_cell_size || NULL == buff)
    {
        ALARM("length[%u] too short or buff[%p]. m_cell_size[%u] count[%u]",
                length, buff, m_cell_size, count);
        return -1;
    }
    if (file_no >= m_max_file_no)
    {
        ALARM("offset[%u] too big. m_cell_num_per_file[%u] max_file_no[%u]",
                offset, m_cell_num_per_file, m_max_file_no);
        return -1;
    }
    // 判断是否跨文件访问
    memset(buff, 0, count*m_cell_size);
    if ( inoffset + count > m_cell_num_per_file)
    {
        // 跨文件访问
        // 递归调用
        uint32_t count1 = m_cell_num_per_file - inoffset;
        get(offset, count1, buff, count1 * m_cell_size);

        int32_t retlen = get((file_no+1)*m_cell_num_per_file, count - count1,
                &(((char*)buff)[count1*m_cell_size]), (count-count1)*m_cell_size);
        ROUTN("offset1[%u] offset2[%u] count[%u] count1[%u] count2[%u]",
                offset, (file_no+1)*m_cell_num_per_file, count, count1, count - count1);
        return (retlen < 0) ? -1 : count1*m_cell_size + retlen;
    }
    else
    {
        if (m_fd[file_no] == -1)
        {
            char tmpstr[128];
            snprintf(tmpstr, sizeof(tmpstr), FORMAT_PATH, m_fb_dir, m_fb_name, file_no);
            mode_t amode = (0 == access(tmpstr, F_OK)) ? O_RDWR : O_RDWR|O_CREAT;
            m_fd[file_no] = open(tmpstr, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        }
        MyThrowAssert(m_fd[file_no] != -1);
        return pread(m_fd[file_no], buff, m_cell_size*count, inoffset * m_cell_size);
    }
}


int32_t fileblock :: clear()
{
    for (uint32_t i=0; i<MAX_FILE_NO; i++)
    {
        if (m_fd[i] >= 0)
        {
            char tmpstr[128];
            snprintf(tmpstr, sizeof(tmpstr), FORMAT_PATH, m_fb_dir, m_fb_name, i);
            unlink(tmpstr);
            close(m_fd[i]);
            m_fd[i] = -1;
        }
    }
    m_max_file_no = 0;
    m_last_file_offset = 0;
    return 0;
}
void fileblock :: begin()
{
    m_it = 0;
}
int32_t fileblock :: write_next(void* buff)
{
    uint32_t cur_it = m_it ++ ;
    return set(cur_it, buff);
}
int32_t fileblock :: read_next(void* buff, const uint32_t length)
{
    uint32_t cur_it = m_it ++ ;
    return get(cur_it, buff, length);
}

bool fileblock :: isend()
{
    return m_it == (m_max_file_no-1)*m_cell_num_per_file + m_last_file_offset/m_cell_size;
}

int32_t fileblock::detect_file()
{
    DIR *dp;
    struct  dirent  *dirp;

    char prefix[128];
    snprintf(prefix, sizeof(prefix), FORMAT_FILE, m_fb_name);

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

uint32_t fileblock :: getfilesize( const char* name )
{
    struct stat fs;
    MyThrowAssert( 0 == stat( name, &fs ) );
    return fs.st_size;
}

