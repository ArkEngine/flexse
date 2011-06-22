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
    snprintf(m_second_index_file, sizeof(m_second_index_file), FORMAT_SECOND_INDEX, dir, iname);
    int fd = open(m_second_index_file, O_RDONLY);
    if (fd == -1)
    {
        ALARM("a new disk_indexer, set freeze as FALSE.");
        m_readonly = false;
    }
    else
    {
        second_index_t sindex;
        while(sizeof(sindex) == read(fd, &sindex, sizeof(sindex)))
        {
            m_second_index.push_back(sindex);
        }
        m_readonly = (0 != m_second_index.size());
    }
    close(fd);
    m_index_block = (fb_index_t*)malloc(TERM_MILESTONE*sizeof(fb_index_t));
    MyThrowAssert(NULL != m_index_block);
}

disk_indexer :: ~disk_indexer()
{
    free(m_index_block);
    m_index_block = NULL;
}

int disk_indexer :: ikey_comp (const void *m1, const void *m2)
{
    int64_t ret = ((fb_index_t*)m1)->ikey.sign64 - ((fb_index_t*)m2)->ikey.sign64;
    if (ret < 0)
    {
        return -1;
    }
    else if (ret > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

int32_t disk_indexer :: get_posting_list(const char* strTerm, void* buff, const uint32_t length)
{
    // 一旦调用get方法，则变为只读状态
    set_readonly();

    int len = strlen(strTerm);
    if (0 == len || NULL == buff || length == 0)
    {
        ALARM("strTerm[%s] buff[%p] length[%u]", strTerm, buff, length);
        return -1;
    }

    // (1) 先读取二级索引，得到在fileblock中的第几块中
    uint32_t last_offset = m_second_index.size()-1;
    second_index_t si;
    creat_sign_64(strTerm, len, &si.ikey.uint1, &si.ikey.uint2);
    if (si.ikey.sign64 < m_second_index[0].ikey.sign64
            || si.ikey.sign64 > m_second_index[last_offset].ikey.sign64)
    {
        ALARM("strTerm[%s]'s sign[%llu] NOT in range[%llu : %llu].",
                strTerm, si.ikey.sign64, m_second_index[0].ikey.sign64,
                m_second_index[last_offset].ikey.sign64);
        return -1;
    }
    vector<second_index_t>::iterator bounds;
    bounds = lower_bound (m_second_index.begin(), m_second_index.end(), si);
    // (2) 根据milestone读取fileblock中的连续块
    uint32_t block_no = (bounds->milestone == 0) ? 0 : bounds->milestone-TERM_MILESTONE;
    uint32_t rd_count = (bounds->milestone == m_last_si.milestone) ? m_last_si.milestone%TERM_MILESTONE : TERM_MILESTONE;

    MyThrowAssert(rd_count*sizeof(fb_index_t) == (uint32_t)m_fileblock.get(block_no, rd_count,
                m_index_block, TERM_MILESTONE*sizeof(fb_index_t)));
    // (3) 执行二分查找
    fb_index_t theOne;
    theOne.ikey = si.ikey;
    fb_index_t* pseRet = (fb_index_t*)bsearch(&theOne, m_index_block, rd_count, sizeof(fb_index_t), ikey_comp);
    if (pseRet == NULL)
    {
        ALARM("strTerm[%s]'s sign[%llu] NOT found in index blocks.",
                strTerm, si.ikey.sign64);
        return -1;
    }
    // (4) 得到了diskv中的diskv_idx_t, 读取索引
    return m_diskv.get(pseRet->idx, buff, length);
}

int32_t disk_indexer :: set_posting_list(const uint32_t id, const ikey_t& ikey,
        const void* buff, const uint32_t length)
{
    if (m_readonly)
    {
        ALARM("disk_indexer is FREEZON, SO COLD.");
        return -1;
    }
    MyThrowAssert(id   > m_last_si.milestone);
    MyThrowAssert(ikey.sign64 > m_last_si.ikey.sign64);
    m_last_si.milestone = id;
    m_last_si.ikey      = ikey;
    fb_index_t fi;
    MyThrowAssert(0 == m_diskv.set(fi.idx, buff, length));
    fi.ikey = ikey;
    MyThrowAssert(0 == m_fileblock.set(id, &fi));
    // 记住最后一个milestone
    if (0 == (id % TERM_MILESTONE))
    {
        // set milestone
        m_second_index.push_back(m_last_si);
    }
    return 0;
}

void disk_indexer :: set_finish()
{
    if (0 != (m_last_si.milestone % TERM_MILESTONE))
    {
        m_second_index.push_back(m_last_si);
    }
    // 把m_second_index写入磁盘
    int fd = open(m_second_index_file, O_WRONLY|O_CREAT|O_TRUNC);
    MyThrowAssert (fd != -1);
    for (uint32_t i=0; i<m_second_index.size(); i++)
    {
        MyThrowAssert(sizeof(second_index_t) == write(fd, &m_second_index[i], sizeof(second_index_t)));
    }
    close(fd);
    m_readonly = true;
}

void disk_indexer :: clear()
{
    // 清掉cache
    // 清掉m_index_block;
    m_second_index.clear();
    // 清掉磁盘上的index_block;
    remove(m_second_index_file);
    m_fileblock.clear();
    m_diskv.clear();
    m_readonly = false;
}

void disk_indexer :: set_readonly()
{
    m_readonly = true;
}

void disk_indexer :: begin()
{
    m_fileblock.begin();
}

int32_t disk_indexer :: itget(uint64_t& key, void* buff, const uint32_t length)
{
    fb_index_t fbit;
    MyThrowAssert ( sizeof(fbit) == m_fileblock.itget(&fbit, sizeof(fbit)));
    key = fbit.ikey.sign64;
    return m_diskv.get(fbit.idx, buff, length);
}

void disk_indexer :: next()
{
    m_fileblock.next();
}

bool disk_indexer :: is_end()
{
    return m_fileblock.is_end();
}
