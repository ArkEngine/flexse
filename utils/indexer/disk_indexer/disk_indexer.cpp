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
    if (fd == -1)
    {
        ALARM("a new disk_indexer, set freeze as FALSE.");
        m_freeze = false;
    }
    else
    {
        second_index_t sindex;
        while(sizeof(sindex) == read(fd, &sindex, sizeof(sindex)))
        {
            second_index.push_back(sindex);
        }
        m_freeze = (0 != second_index.size());
    }
    close(fd);
}

disk_indexer :: ~disk_indexer()
{
}

int32_t disk_indexer :: get_posting_list(const char* strTerm, char* buff, const uint32_t length)
{
    // 一旦调用get方法，则变为只读状态
    m_freeze = (m_freeze) ? true: false;

    int len = strlen(strTerm);
    if (0 == len || NULL == buff || length == 0)
    {
        ALARM("strTerm[%s] buff[%p] length[%u]", strTerm, buff, length);
        return -1;
    }

    // (1) 先读取二级索引，得到在fileblock中的第几块中
    second_index_t si;
    creat_sign_64(strTerm, len, &si.ikey.uint1, &si.ikey.uint2);
    vector<second_index_t>::iterator bounds;
    bounds = lower_bound (second_index.begin(), second_index.end(), si);
    if (bounds == second_index.end())
    {
        ALARM("strTerm[%s]'s sign[%llu] too big.", strTerm, si.ikey.sign64);
        return -1;
    }
    // (2) 根据milestone读取fileblock中的连续块
    // (3) 执行二分查找
    // (4) 得到了diskv中的diskv_idx_t
    // (5) 读取索引
    return 0;
}

int32_t disk_indexer :: set_posting_list(const uint32_t id, const ikey_t& ikey,
        const char* buff, const uint32_t length)
{
    if (m_freeze)
    {
        ALARM("disk_indexer is FREEZON, SO COLD.");
        return -1;
    }
    fb_index_t fi;
    MyThrowAssert(0 == m_diskv.set(fi.idx, buff, length));
    fi.ikey = ikey;
    MyThrowAssert(0 == m_fileblock.set(id, &fi));
    // 记住最后一个milestone
    m_last_si.milestone = id;
    m_last_si.ikey      = ikey;
    // set milestone
    if (0 == (id % TERM_MILESTONE))
    {
        second_index.push_back(m_last_si);
    }
    return 0;
}

void disk_indexer :: set_finish()
{
    m_freeze = true;
    if (0 != (m_last_si.milestone % TERM_MILESTONE))
    {
        second_index.push_back(m_last_si);
    }
}
