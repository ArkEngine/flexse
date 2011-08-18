#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#include "MyException.h"
#include "mylog.h"
#include "diskv.h"

const char* const diskv :: FORMAT_FILE = "%s.dat.";
const char* const diskv :: FORMAT_PATH = "%s/%s.dat.%u";

diskv :: diskv(const char* dir, const char* module)
{
    snprintf(m_dir,    sizeof(m_dir),    "%s", dir);
    snprintf(m_module, sizeof(m_module), "%s", module);
    for (uint32_t i=0; i<MAX_FILE_NO; i++)
    {
        m_read_fd[i] = -1;
    }
    int detect_no = detect_file();
    m_max_file_no = (detect_no <= 0) ? 1 : detect_no + 1;
    char full_name[MAX_PATH_LENGTH];

    snprintf(full_name, sizeof(full_name), FORMAT_PATH, m_dir, m_module, m_max_file_no - 1);
    m_last_file_offset = (0 == access(full_name, F_OK)) ? getfilesize(full_name) : 0;
    mode_t amode = (0 == access(full_name, F_OK)) ? O_WRONLY|O_APPEND : O_WRONLY|O_APPEND|O_CREAT;
    m_append_fd = open(full_name, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    MyThrowAssert(m_append_fd != -1);
}

diskv :: ~diskv()
{
    for (uint32_t i=0; i<m_max_file_no; i++)
    {
        if (m_read_fd[i] >= 0)
        {
            close(m_read_fd[i]);
            m_read_fd[i] = -1;
        }
    }
    close (m_append_fd);
    m_append_fd = -1;
}

uint32_t diskv :: getfilesize( const char* name )
{
    struct stat fs;
    MyThrowAssert( 0 == stat( name, &fs ) );
    return fs.st_size;
}

int diskv :: detect_file( )
{
    DIR *dp;
    struct  dirent  *dirp;

    char prefix[128];
    snprintf(prefix, sizeof(prefix), FORMAT_FILE, m_module);

    if((dp = opendir(m_dir)) == NULL)
    {
        FATAL( "can't open %s! msg[%m] ", m_dir);
        MySuicideAssert(0);
    }

    int len = strlen(prefix);
    int max = -1;

    while((dirp = readdir(dp)) != NULL)
    {
//        DEBUG( "%s", dirp->d_name);
        char* pn = strstr(dirp->d_name, prefix);
        if (pn != NULL)
        {
            const char* pp = &pn[len];
            const char* cc = pp;
            while (isdigit(*cc)) { cc++; }
            if (*cc == '\0')
            {
                int n = atoi(&pn[len]);
//                DEBUG( "-- %d", n);
                if (n > max)
                {
                    max = n;
                }
            }
//            else
//            {
//                DEBUG( "^^ invalid file name : %s", dirp->d_name);
//            }
        }
    }

    closedir(dp);
    return max;
}

void diskv :: check_new_file(const uint32_t length)
{
    if (length + m_last_file_offset > MAX_FILE_SIZE 
            || m_append_fd < 0)
    {
        // 开辟新的文件
        char full_name[MAX_PATH_LENGTH];
        snprintf(full_name, sizeof(full_name), FORMAT_PATH, m_dir, m_module, m_max_file_no);
        mode_t amode = (0 == access(full_name, F_OK)) ? O_WRONLY|O_APPEND : O_WRONLY|O_APPEND|O_CREAT;
        if (m_append_fd >= 0)
        {
            close(m_append_fd);
        }
        m_append_fd = open(full_name, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        MyThrowAssert(m_append_fd != -1);
        m_max_file_no ++;
        m_last_file_offset = (uint32_t)lseek(m_append_fd, 0, SEEK_END);
    }
}

int diskv :: set(diskv_idx_t& idx, const void* buff, const uint32_t length)
{
    if (length > MAX_DATA_SIZE || NULL == buff)
    {
        ALARM ("params error. length[%u] length_limit[%u] buff[%p]",
                length, MAX_DATA_SIZE, buff);
        return -1;
    }

    check_new_file(length);
    MyThrowAssert(m_last_file_offset == (uint32_t)lseek(m_append_fd, 0, SEEK_END));
    MyThrowAssert(length == (uint32_t)write(m_append_fd, buff, length));
    idx.offset   = m_last_file_offset;
    idx.file_no  = m_max_file_no - 1;
    idx.data_len = length;
    m_last_file_offset += length;
    return 0;
}

int diskv :: get(const diskv_idx_t& idx, void* buff, const uint32_t length)
{
    if (idx.file_no >= m_max_file_no
            || idx.offset > MAX_FILE_SIZE
            || idx.data_len > length
            || NULL == buff
            || ((idx.file_no == m_max_file_no - 1) && (idx.offset + idx.data_len > m_last_file_offset))
       )
    {
        ALARM ("params error. idx.file_no[%u] idx.offset[%u] idx.data_len[%u] "
                "max_file_no[%u] buff[%p] length[%u] limit_length[%u] cur_offset[%u]",
                idx.file_no, idx.offset, idx.data_len,
                m_max_file_no - 1, buff, length, MAX_FILE_SIZE, m_last_file_offset);
        return -1;
    }

    if (m_read_fd[idx.file_no] == -1)
    {
        char full_name[MAX_PATH_LENGTH];
        snprintf(full_name, sizeof(full_name), FORMAT_PATH, m_dir, m_module, idx.file_no);
        m_read_fd[idx.file_no] = open(full_name, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    }
    MyThrowAssert(m_read_fd[idx.file_no] != -1);
    return pread(m_read_fd[idx.file_no], buff, idx.data_len, idx.offset);
}

void diskv :: clear()
{
    close(m_append_fd);
    m_append_fd = -1;
    for (uint32_t i=0; i<m_max_file_no; i++)
    {
        char full_name[MAX_PATH_LENGTH];
        snprintf(full_name, sizeof(full_name), FORMAT_PATH, m_dir, m_module, i);
        close(m_read_fd[i]);
        m_read_fd[i] = -1;
        remove(full_name);
    }
    m_max_file_no = 0;
    m_last_file_offset = 0;
}
