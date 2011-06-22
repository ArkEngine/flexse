#include "index_group.h"
#include "MyException.h"
#include "mylog.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

const char* const index_group :: STR_INDEX_NAME = "index";
const char* const index_group :: STR_INDEX_CUR_NO_FILE = "cur";
const char* const index_group :: STR_DAY_INDEX_DIR = "./data/day/";
const char* const index_group :: STR_HIS_INDEX_DIR = "./data/his/";
const char* const index_group :: STR_FMT_DAY_INDEX_PATH = "./data/day/%u/";
const char* const index_group :: STR_FMT_HIS_INDEX_PATH = "./data/his/%u/";

index_group :: index_group(const uint32_t cell_size, const uint32_t bucket_size,
        const uint32_t headlist_size, const uint32_t* blocknum_list, const uint32_t blocknum_list_size)
{
    // mem init
    m_cell_size = cell_size;
    m_bucket_size = bucket_size;
    m_headlist_size = headlist_size;
    m_blocknum_list_size = blocknum_list_size;
    memmove(m_blocknum_list, blocknum_list, sizeof(uint32_t)*blocknum_list_size);

    m_mem[0] = new mem_indexer(m_cell_size, m_bucket_size, m_headlist_size,
            m_blocknum_list, m_blocknum_list_size);
    m_mem[1] = NULL;
    m_index_list.push_back(m_mem[0]);
    m_index_list.push_back(m_mem[1]);

    // day init
    // read cur file
    uint32_t day_cur = get_cur_no(STR_DAY_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    char index_dir[MAX_PATH_LENGTH];
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, day_cur);
    m_day[0] = new disk_indexer(index_dir, STR_INDEX_NAME);
    m_index_list.push_back(m_day[0]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, 1 - day_cur);
    m_day[1] = new disk_indexer(index_dir, STR_INDEX_NAME);

    // his init
    // read cur file
    uint32_t his_cur = get_cur_no(STR_HIS_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_HIS_INDEX_PATH, his_cur);
    m_his[0] = new disk_indexer(index_dir, STR_INDEX_NAME);
    m_index_list.push_back(m_his[0]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_HIS_INDEX_PATH, 1 - his_cur);
    m_his[1] = new disk_indexer(index_dir, STR_INDEX_NAME);

    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_mem_dump_cond, NULL);
    pthread_rwlock_init(&m_list_rwlock, NULL);
}

index_group :: ~index_group()
{
}

mem_indexer* index_group :: swap_mem_indexer(mem_indexer* pmem_indexer)
{
    MyThrowAssert(m_index_list[0] == pmem_indexer);
    // 阻塞于等待 m_index_list[1]清空
    pthread_mutex_lock(&m_mutex);
    while(m_index_list[1] != NULL)
    {
        pthread_cond_wait(&m_mem_dump_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);
    // -1- 调换位置
    pthread_rwlock_wrlock(&m_list_rwlock);
    // 设置为只读状态
    m_index_list[1] = m_index_list[0];
    m_index_list[1]->set_readonly();
    m_index_list[0] = new mem_indexer(m_cell_size, m_bucket_size, m_headlist_size,
            m_blocknum_list, m_blocknum_list_size);
    pthread_rwlock_unlock(&m_list_rwlock);
    // -2- 通知merge线程可以把mem持久化了
    pthread_cond_signal(&m_mem_dump_cond);
    // -3- 返回一个新的mem
    return dynamic_cast<mem_indexer*>(m_index_list[0]);
}

void index_group :: update_day_indexer(disk_indexer* pdisk_indexer)
{
    MyThrowAssert(pdisk_indexer == m_day[0] || pdisk_indexer == m_day[1]);
    MyThrowAssert(pdisk_indexer != m_index_list[2]);
    pthread_rwlock_wrlock(&m_list_rwlock);
    delete m_index_list[1];
    m_index_list[1] = NULL;
    m_index_list[2]->clear();
    m_index_list[2] = pdisk_indexer;
    // 通知 m_index_list[1] == NULL 了
    pthread_cond_signal(&m_mem_dump_cond);
    pthread_rwlock_unlock(&m_list_rwlock);
    return;
}

void index_group :: update_his_indexer(disk_indexer* pdisk_indexer)
{
    MyThrowAssert(pdisk_indexer == m_his[0] || pdisk_indexer == m_his[1]);
    MyThrowAssert(pdisk_indexer != m_index_list[3]);
    pthread_rwlock_wrlock(&m_list_rwlock);
    m_index_list[2]->clear();
    m_index_list[3]->clear();
    m_index_list[3] = pdisk_indexer;
    pthread_rwlock_unlock(&m_list_rwlock);
    return;
}

mem_indexer*  index_group :: get_cur_mem_indexer()
{
    return dynamic_cast<mem_indexer*>(m_index_list[0]);
}
disk_indexer* index_group :: get_cur_day_indexer()
{
    return dynamic_cast<disk_indexer*>(m_index_list[2]);
}
disk_indexer* index_group :: get_cur_his_indexer()
{
    return dynamic_cast<disk_indexer*>(m_index_list[3]);
}

int32_t index_group :: get_posting_list(const char* strTerm, void* buff, const uint32_t length)
{
    int32_t offset = 0;
    int32_t lstnum = 0;
    pthread_rwlock_rdlock(&m_list_rwlock);
    for (uint32_t i=0; i<m_index_list.size(); i++)
    {
        if (m_index_list[i] != NULL)
        {
            lstnum += m_index_list[i]->get_posting_list(strTerm, &((char*)buff)[offset], length-offset);
            offset = lstnum * m_cell_size;
        }
    }
    pthread_rwlock_unlock(&m_list_rwlock);
    return lstnum;
}

uint32_t index_group :: get_cur_no(const char* dir, const char* name)
{
    char filename[MAX_PATH_LENGTH];
    snprintf(filename, sizeof(filename), "%s/%s", dir, name);
    int read_rd = open(filename, O_RDONLY);
    MyThrowAssert(read_rd != -1);
    memset(filename, 0, sizeof(filename));
    MyThrowAssert(1 <= read(read_rd, filename, sizeof(filename)));
    int cur = atoi(filename);
    MyThrowAssert(cur == 0 || cur == 1);
    return cur;
}
