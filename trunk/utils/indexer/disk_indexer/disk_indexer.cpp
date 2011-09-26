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
disk_indexer :: disk_indexer(const char* dir, const char* iname, const uint32_t posting_cell_size)
    : m_fileblock(dir, iname, sizeof(fb_index_t)),
      m_diskv(dir, iname),
      m_posting_cell_size(posting_cell_size)
{
    uint32_t first_index_count  = m_fileblock.get_cell_count();
    uint32_t second_index_count = 0;

    snprintf(m_second_index_file, sizeof(m_second_index_file), FORMAT_SECOND_INDEX, dir, iname);
    int fd = open(m_second_index_file, O_RDONLY);
    if (fd == -1)
    {
        ALARM("a new disk_indexer, set freeze as FALSE.");
        m_readonly = false;
        m_fileblock.clear();
        m_diskv.clear();
        remove(m_second_index_file);
    }
    else
    {
        uint32_t flen = getfilesize(m_second_index_file);
        MySuicideAssert(0 == (flen%sizeof(second_index_t)));

        second_index_t sindex;
        while(sizeof(sindex) == read(fd, &sindex, sizeof(sindex)))
        {
            m_second_index.push_back(sindex);
        }
    }
    second_index_count = (0 == m_second_index.size()) ? 0 : (m_second_index[m_second_index.size()-1].milestone + 1); 
    // 因为milestone表示已经写入的id(id从0开始)，因此需要+1
    if (0 == second_index_count || second_index_count != first_index_count)
    {
        // 判断2级索引是否已经完整的写入了
        ALARM("second index NOT complete first_index_count[%u] second_index_count[%u]. set freeze as FALSE.",
                first_index_count, second_index_count);
        m_readonly = false;
        m_fileblock.clear();
        m_diskv.clear();
        remove(m_second_index_file);
    }
    if (fd >= 0)
    {
        close(fd);
    }
    m_last_si.milestone = 0;
    m_last_si.ikey.sign64 = 0;
    m_readonly = (0 != m_second_index.size());
    ROUTN("second_index size[%u] count[%u] first_index_count[%u] set freeze as [%u].",
            m_second_index.size(),
            second_index_count,
            first_index_count,
            m_readonly);
    pthread_mutex_init(&m_map_mutex, NULL);
}

disk_indexer :: ~disk_indexer()
{
    for(m_map_it=m_cache_map.begin(); m_map_it!=m_cache_map.end(); m_map_it++)
    {
        free(m_map_it->second.pbuff);
    }
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
    //    // 一旦调用get方法，则变为只读状态
    //    set_readonly();

    if (m_readonly == false)
    {
//        ALARM("disk index can't be readed. term[%s]", strTerm);
        return -1;
    }

    int len = strlen(strTerm);
    if (0 == len || NULL == buff || length == 0)
    {
        ALARM("strTerm[%s] buff[%p] length[%u]", strTerm, buff, length);
        return -1;
    }
    if (0 == m_second_index.size())
    {
        return 0;
    }

    // (1) 先读取二级索引，得到在fileblock中的第几块中
    // 搞点cache能减少IO，那就搞起吧
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
    char cache_key[64];
    snprintf(cache_key, sizeof(cache_key), "%u:%u", block_no, rd_count);
    fb_index_t* pindex_block = NULL;
    uint32_t rt_count = 0;

    pthread_mutex_lock(&m_map_mutex);
    m_map_it = m_cache_map.find(string(cache_key));
    if (m_map_it == m_cache_map.end())
    {
        // cache没找到
        pindex_block = (fb_index_t*)malloc(TERM_MILESTONE*sizeof(fb_index_t));
        if (NULL == pindex_block)
        {
            FATAL("Can't Alloc Memory for index_block term[%s]!!", strTerm);
            pthread_mutex_unlock(&m_map_mutex);
            return -1;
        }
        else
        {
            // 之后也不释放pindex_block，析构的时候统一释放
            rt_count = (uint32_t)m_fileblock.get(block_no, rd_count, pindex_block, TERM_MILESTONE*sizeof(fb_index_t));
            DEBUG("rd_count[%u] rt_count[%u] block_no[%u]", rd_count, rt_count, block_no);
            MyThrowAssert(rd_count*sizeof(fb_index_t) == (uint32_t)m_fileblock.get(block_no, rd_count,
                        pindex_block, TERM_MILESTONE*sizeof(fb_index_t)));
            struct block_cache_t block_cache_item = {rd_count, pindex_block};
            m_cache_map[string(cache_key)] = block_cache_item;
        }
    }
    else
    {
        rt_count     = m_map_it->second.bufsiz;
        pindex_block = m_map_it->second.pbuff;
    }
    pthread_mutex_unlock(&m_map_mutex);

    // (3) 执行二分查找
    fb_index_t theOne;
    theOne.ikey = si.ikey;
    fb_index_t* pseRet = (fb_index_t*)bsearch(&theOne, pindex_block, rd_count, sizeof(fb_index_t), ikey_comp);
    if (pseRet == NULL)
    {
        ALARM("strTerm[%s]'s sign[%llu] NOT found in index blocks.",
                strTerm, si.ikey.sign64);
        return -1;
    }
    // (4) 得到了diskv中的diskv_idx_t, 读取索引
    int ret = m_diskv.get(pseRet->idx, buff, length);
    // ROUTN("diskv readret[%u] idx.data_len[%u]", ret, pseRet->idx.data_len);
    return (ret < 0) ? ret : ret/m_posting_cell_size;
}

int32_t disk_indexer :: set_posting_list(const uint32_t id, const ikey_t& ikey,
        const void* buff, const uint32_t length)
{
    if (m_readonly)
    {
        ALARM("disk_indexer is FREEZON, SO COLD.");
        return -1;
    }
    if (m_last_si.milestone > 0)
    {
        MyThrowAssert(id > m_last_si.milestone);
        MyThrowAssert(ikey.sign64 > m_last_si.ikey.sign64);
    }
    m_last_si.milestone = id;
    m_last_si.ikey      = ikey;
    fb_index_t fi;
    MyThrowAssert(0 == m_diskv.set(fi.idx, buff, length));
    //    ROUTN("diskv idx.data_len[%u]", fi.idx.data_len);
    fi.ikey = ikey;
    MyThrowAssert(0 == m_fileblock.set(id, &fi));
    if (0 == (id % TERM_MILESTONE))
    {
        // set milestone
        m_second_index.push_back(m_last_si);
    }
    return 0;
}

void disk_indexer :: set_finish()
{
    // 记住最后一个milestone
    // 这个if判断是为了防止加入2次
    if (0 != (m_last_si.milestone % TERM_MILESTONE))
    {
        m_second_index.push_back(m_last_si);
    }
    if (m_second_index.size() == 0)
    {
        return;
    }
    // 把m_second_index写入磁盘

    mode_t amode = (0 == access(m_second_index_file, F_OK)) ? O_WRONLY|O_TRUNC : O_WRONLY|O_TRUNC|O_CREAT;
    int fd = open(m_second_index_file, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    MyThrowAssert (fd != -1);
    for (uint32_t i=0; i<m_second_index.size(); i++)
    {
        MyThrowAssert(sizeof(second_index_t) == write(fd, &m_second_index[i], sizeof(second_index_t)));
    }
    close(fd);
    m_readonly  = true;
}

void disk_indexer :: clear()
{
    // 清掉cache
    m_second_index.clear();
    // 清掉磁盘上的index_block;
    remove(m_second_index_file);
    m_fileblock.clear();
    m_diskv.clear();
    m_last_si.milestone = 0;
    m_last_si.ikey.sign64 = 0;
    m_readonly = false;
}

bool disk_indexer :: empty()
{
    return m_readonly == false;
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
    int ret = m_diskv.get(fbit.idx, buff, length);
    return (ret < 0) ? ret : ret/m_posting_cell_size;
}

void disk_indexer :: next()
{
    m_fileblock.next();
}

bool disk_indexer :: is_end()
{
    return m_fileblock.is_end();
}

uint32_t disk_indexer :: getfilesize( const char* name )
{
    struct stat fs;
    MyThrowAssert( 0 == stat( name, &fs ) );
    return fs.st_size;
}

