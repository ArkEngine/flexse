#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <string.h>
#include "mylog.h"
#include "creat_sign.h"
#include "MyException.h"
#include "disk_indexer.h"

const char* const disk_indexer :: FORMAT_SECOND_INDEX = "%s/%s.second_idx";
disk_indexer :: disk_indexer(const char* dir, const char* iname)
    : m_fileblock(dir, iname, sizeof(fb_index_t)), m_diskv(dir, iname)
{
    char filename[MAX_FILE_LENGTH];
    snprintf(filename, sizeof(filename), FORMAT_SECOND_INDEX, dir, iname);
    int fd = open(filename, O_RDONLY);
    MyThrowAssert(fd != -1);
    second_index_t sindex;
    while(sizeof(sindex) == read(fd, &sindex, sizeof(sindex)))
    {
        second_index.push_back(sindex);
    }
    close(fd);
}

disk_indexer :: ~disk_indexer()
{
}

int32_t disk_indexer :: get_posting_list(const char* strTerm, char* buff, const uint32_t length)
{
    int len = strlen(strTerm);
    if (0 == len || NULL == buff || length == 0)
    {
        ALARM("strTerm[%s] buff[%p] length[%u]", strTerm, buff, length);
        return -1;
    }

    second_index_t si;
    creat_sign_64(strTerm, len, &si.ikey.uint1, &si.ikey.uint2);
    vector<second_index_t>::iterator bounds;
    bounds = lower_bound (second_index.begin(), second_index.end(), si);
    return 0;
}

int32_t disk_indexer :: set_posting_list(const uint32_t id, const ikey_t& ikey,
        const char* buff, const uint32_t length)
{
    fb_index_t fi;
    MyThrowAssert(0 == m_diskv.set(fi.idx, buff, length));
    fi.ikey = ikey;
    MyThrowAssert(0 == m_fileblock.set(id, &fi));
    return 0;
}
