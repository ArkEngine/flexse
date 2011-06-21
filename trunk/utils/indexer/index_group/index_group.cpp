#include "index_group.h"
#include "MyException.h"
#include "mylog.h"
#include <stdio.h>

const char* const STR_INDEX_NAME = "index";
const char* const STR_INDEX_CUR_NO_FILE = "cur";
const char* const STR_DAY_INDEX_DIR = "./data/day/";
const char* const STR_HIS_INDEX_DIR = "./data/his/";
const char* const STR_FMT_DAY_INDEX_PATH = "./data/day/%u/";
const char* const STR_FMT_HIS_INDEX_PATH = "./data/his/%u/";

index_group :: index_group()
{
    index_info_t index_info;

    // mem init
    index_info.empty  = true;
    index_info.pindex = &m_mem[m_mem_cur];
    m_index_list[0] = index_info;
    index_info.empty  = true;
    index_info.pindex = &m_mem[1 - m_mem_cur];
    m_index_list[1] = index_info;

    // day init
    // read cur file
    uint32_t day_cur = get_cur_no(STR_DAY_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    index_info.empty  = true;
    index_info.pindex = &m_day[day_cur];
    m_index_list[2] = index_info;

    // mon init
    // read cur file
    uint32_t his_cur = get_cur_no(STR_HIS_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    index_info.empty  = true;
    index_info.pindex = &m_his[his_cur];
    m_index_list[3] = index_info;

    pthread_mutex_init(&m_mutex);
}

index_group :: ~index_group()
{
}

mem_indexer* index_group :: swap_mem_indexer(mem_indexer* pmem_indexer)
{
    MyThrowAssert(m_index_list[0].pindex == pmem_indexer);
    // 阻塞于等待 m_index_list[1].empty == true;
    while(m_index_list[1].empty == true)
    {
        pthread_cond_wait(m_mem_dump_cond);
    }
    // -1- 调换位置
    pthread_mutex_lock(&m_mutex);
    m_index_list[0].empty = false;
    index_info_t tmp_index_info = m_index_list[0];
    m_index_list[0] = m_index_list[1];
    m_index_list[1] = tmp_index_info;
    pthread_mutex_unlock(&m_mutex);
    // -2- 通知merge线程可以把mem持久化了
    pthread_cond_signal(m_mem_dump_cond);
    // -3- 返回一个新的mem
    return m_index_list[0].pindex;
}

void index_group :: update_day_indexer(disk_indexer* pdisk_indexer)
{
    pthread_mutex_lock(&m_mutex);
    MyThrowAssert(pdisk_indexer == &m_day[0] || pdisk_indexer == &m_day[1]);
    MyThrowAssert(pdisk_indexer != &m_index_list[2].pindex);
    m_index_list[1].reset();
    m_index_list[1].empty = true;
    m_index_list[2].reset();
    m_index_list[2].pindex = pdisk_indexer;
    pthread_mutex_unlock(&m_mutex);
}

void index_group :: update_his_indexer(disk_indexer* pdisk_indexer)
{
    pthread_mutex_lock(&m_mutex);
    MyThrowAssert(pdisk_indexer == &m_his[0] || pdisk_indexer == &m_his[1]);
    MyThrowAssert(pdisk_indexer != &m_index_list[3].pindex);
    m_index_list[2].reset();
    m_index_list[2].empty = true;
    m_index_list[3].reset();
    m_index_list[3].pindex = pdisk_indexer;
    pthread_mutex_unlock(&m_mutex);
}

mem_indexer*  index_group :: get_cur_mem_indexer()
{
    return m_index_list[0].pindex;
}
disk_indexer* index_group :: get_cur_day_indexer()
{
    return m_index_list[2].pindex;
}
disk_indexer* index_group :: get_cur_his_indexer()
{
    return m_index_list[3].pindex;
}

int32_t index_group :: get_posting_list(const char* strTerm, void* buff, const uint32_t length)
{
    int32_t offset = 0;
    int32_t lstnum = 0;
    pthread_mutex_lock(&m_mutex);
    for (uint32_t i=0; i<m_index_list.size(); i++)
    {
        if (m_index_list[i].empty)
        {
            continue;
        }
        lstnum += m_index_list[i].pindex->get_posting_list(strTerm, &((char*)buff)[offset], length-offset);
        offset = lstnum * m_cell_size;
    }
    pthread_mutex_unlock(&m_mutex);
    return lstnum;
}
