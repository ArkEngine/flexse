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
    m_day[day_cur] = new disk_indexer(index_dir, STR_INDEX_NAME);
    m_index_list.push_back(m_day[day_cur]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, 1 - day_cur);
    m_day[1-day_cur] = new disk_indexer(index_dir, STR_INDEX_NAME);

    // his init
    // read cur file
    uint32_t his_cur = get_cur_no(STR_HIS_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_HIS_INDEX_PATH, his_cur);
    m_his[his_cur] = new disk_indexer(index_dir, STR_INDEX_NAME);
    m_index_list.push_back(m_his[his_cur]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_HIS_INDEX_PATH, 1 - his_cur);
    m_his[1 - his_cur] = new disk_indexer(index_dir, STR_INDEX_NAME);

    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_mem_dump_cond, NULL);
    pthread_rwlock_init(&m_list_rwlock, NULL);
}

index_group :: ~index_group()
{
}

mem_indexer* index_group :: swap_mem_indexer()
{
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

void index_group :: update_day_indexer()
{
    pthread_mutex_lock(&m_mutex);
    // 等到m_index_list[1]不为NULL时，执行合并过程
    while(m_index_list[1] == NULL)
    {
        pthread_cond_wait(&m_mem_dump_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);

    // 执行合并过程
    // -1- 找当当前的disk_indexer和空闲的disk_indexer
    disk_indexer* psrc_indexer = dynamic_cast<disk_indexer*>(m_index_list[2]);
    disk_indexer* pdst_indexer = dynamic_cast<disk_indexer*>((psrc_indexer == m_day[0]) ? m_day[1] : m_day[0]);
    const uint32_t length = MAX_POSTINGLIST_SIZE * m_cell_size;
    void* src1_list = malloc(length);
    void* src2_list = malloc(length);
    MyThrowAssert(src1_list != NULL && src2_list != NULL);
    ikey_t key1;
    ikey_t key2;
    m_index_list[1]->begin();
    m_index_list[2]->begin();
    uint32_t id = 0;
    while((!m_index_list[1]->is_end()) && (! m_index_list[2]->is_end()))
    {
        // get_and_next 并不适合迭代，把这两个操作分开
        int32_t num1 = m_index_list[1]->itget(key1.sign64, src1_list, length);
        int32_t num2 = m_index_list[2]->itget(key2.sign64, src1_list, length);
        if (key1.sign64 < key2.sign64)
        {
            if (num1 > 0)
            {
                pdst_indexer->set_posting_list(id, key1, src1_list, num1*m_cell_size);
                id++;
            }
            m_index_list[1]->next();
        }
        else if (key1.sign64 > key2.sign64)
        {
            if (num2 > 0)
            {
                pdst_indexer->set_posting_list(id, key2, src2_list, num2*m_cell_size);
                id++;
            }
            m_index_list[2]->next();
        }
        else
        {
            MyThrowAssert (num1 > 0 && num2 > 0);
            memmove(&(((char*)src1_list)[num1*m_cell_size]), src2_list, num2*m_cell_size);
            pdst_indexer->set_posting_list(id, key1, src1_list, (num1+num2)*m_cell_size);
        }
    }

    while (!m_index_list[1]->is_end())
    {
        int32_t num1 = m_index_list[1]->itget(key1.sign64, src1_list, length);
        if (num1 > 0)
        {
            pdst_indexer->set_posting_list(id, key1, src1_list, num1*m_cell_size);
        }
        m_index_list[1]->next();
        id++;
    }
    while (!m_index_list[2]->is_end())
    {
        int32_t num2 = m_index_list[2]->itget(key2.sign64, src2_list, length);
        if (num2 > 0)
        {
            pdst_indexer->set_posting_list(id, key2, src2_list, num2*m_cell_size);
        }
        id++;
        m_index_list[2]->next();
    }

    // 执行合并完毕, 设置cur文件
    uint32_t cur = (pdst_indexer == m_day[0]) ? 0 : 1;
    set_cur_no(STR_DAY_INDEX_DIR, STR_INDEX_CUR_NO_FILE, cur);
    free(src1_list);
    free(src2_list);

    pthread_rwlock_wrlock(&m_list_rwlock);
    delete m_index_list[1];
    m_index_list[1] = NULL;
    m_index_list[2]->clear();
    m_index_list[2] = pdst_indexer;
    // 通知 m_index_list[1] == NULL 了
    pthread_cond_signal(&m_mem_dump_cond);
    pthread_rwlock_unlock(&m_list_rwlock);
    return;
}

void index_group :: update_his_indexer()
{
    // 对day的索引进行一些判空，避免空合并 TODO

    // 执行合并过程
    // -1- 找当当前的disk_indexer和空闲的disk_indexer
    disk_indexer* psrc_indexer = dynamic_cast<disk_indexer*>(m_index_list[3]);
    disk_indexer* pdst_indexer = dynamic_cast<disk_indexer*>((psrc_indexer == m_his[0]) ? m_his[1] : m_his[0]);
    const uint32_t length = MAX_POSTINGLIST_SIZE * m_cell_size;
    void* src1_list = malloc(length);
    void* src2_list = malloc(length);
    MyThrowAssert(src1_list != NULL && src2_list != NULL);
    ikey_t key1;
    ikey_t key2;
    m_index_list[2]->begin();
    m_index_list[3]->begin();
    uint32_t id = 0;
    while((!m_index_list[2]->is_end()) && (! m_index_list[3]->is_end()))
    {
        // get_and_next 并不适合迭代，把这两个操作分开
        int32_t num1 = m_index_list[2]->itget(key1.sign64, src1_list, length);
        int32_t num2 = m_index_list[3]->itget(key2.sign64, src1_list, length);
        if (key1.sign64 < key2.sign64)
        {
            if (num1 > 0)
            {
                pdst_indexer->set_posting_list(id, key1, src1_list, num1*m_cell_size);
                id++;
            }
            m_index_list[2]->next();
        }
        else if (key1.sign64 > key2.sign64)
        {
            if (num2 > 0)
            {
                pdst_indexer->set_posting_list(id, key2, src2_list, num2*m_cell_size);
                id++;
            }
            m_index_list[3]->next();
        }
        else
        {
            MyThrowAssert (num1 > 0 && num2 > 0);
            memmove(&(((char*)src1_list)[num1*m_cell_size]), src2_list, num2*m_cell_size);
            pdst_indexer->set_posting_list(id, key1, src1_list, (num1+num2)*m_cell_size);
        }
    }

    while (!m_index_list[2]->is_end())
    {
        int32_t num1 = m_index_list[2]->itget(key1.sign64, src1_list, length);
        if (num1 > 0)
        {
            pdst_indexer->set_posting_list(id, key1, src1_list, num1*m_cell_size);
        }
        m_index_list[2]->next();
        id++;
    }
    while (!m_index_list[3]->is_end())
    {
        int32_t num2 = m_index_list[3]->itget(key2.sign64, src2_list, length);
        if (num2 > 0)
        {
            pdst_indexer->set_posting_list(id, key2, src2_list, num2*m_cell_size);
        }
        id++;
        m_index_list[3]->next();
    }

    // 执行合并完毕, 设置cur文件
    uint32_t cur = (pdst_indexer == m_his[0]) ? 0 : 1;
    set_cur_no(STR_HIS_INDEX_DIR, STR_INDEX_CUR_NO_FILE, cur);
    free(src1_list);
    free(src2_list);

    pthread_rwlock_wrlock(&m_list_rwlock);
    m_index_list[2]->clear();
    m_index_list[3]->clear();
    m_index_list[3] = pdst_indexer;
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
    int read_fd = open(filename, O_RDONLY);
    MyThrowAssert(read_fd != -1);
    memset(filename, 0, sizeof(filename));
    MyThrowAssert(1 <= read(read_fd, filename, sizeof(filename)));
    int cur = atoi(filename);
    MyThrowAssert(cur == 0 || cur == 1);
    close(read_fd);
    return cur;
}

void index_group :: set_cur_no(const char* dir, const char* name, const uint32_t cur)
{
    MyThrowAssert(cur == 0 || cur == 1);
    char filename[MAX_PATH_LENGTH];
    snprintf(filename, sizeof(filename), "%s/%s", dir, name);
    int write_fd = open(filename, O_WRONLY|O_TRUNC);
    MyThrowAssert(write_fd != -1);
    snprintf(filename, sizeof(filename), "%u", cur);
    MyThrowAssert(1 <= write(write_fd, filename, strlen(filename)));
    close(write_fd);
}
