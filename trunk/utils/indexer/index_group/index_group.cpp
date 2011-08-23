#include "index_group.h"
#include "MyException.h"
#include "mylog.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>

const char* const index_group :: STR_INDEX_NAME = "index";
const char* const index_group :: STR_INDEX_CUR_NO_FILE = "cur";
const char* const index_group :: STR_DAY_INDEX_DIR = "./data/index/day/";
const char* const index_group :: STR_HIS_INDEX_DIR = "./data/index/his/";
const char* const index_group :: STR_FMT_DAY_INDEX_PATH = "./data/index/day/%u/";
const char* const index_group :: STR_FMT_HIS_INDEX_PATH = "./data/index/his/%u/";

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
    m_day[day_cur] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_index_list.push_back(m_day[day_cur]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, 1 - day_cur);
    m_day[1-day_cur] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_day[1 - day_cur]->clear();
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, 2);
    m_day[2] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_day2_ready = (false == m_day[2]->empty());
    m_index_list.push_back(m_day[2]);

    // his init
    // read cur file
    uint32_t his_cur = get_cur_no(STR_HIS_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_HIS_INDEX_PATH, his_cur);
    m_his[his_cur] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_index_list.push_back(m_his[his_cur]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_HIS_INDEX_PATH, 1 - his_cur);
    m_his[1 - his_cur] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_his[1 - his_cur]->clear();

    pthread_mutex_init(&m_mutex, NULL);
    pthread_cond_init(&m_mem_dump_cond, NULL);
    pthread_rwlock_init(&m_list_rwlock, NULL);

    m_dump_hour_min = 15;
    m_dump_hour_max = 23;
}

index_group :: ~index_group()
{
    for (uint32_t i=0; i<m_index_list.size(); i++)
    {
        delete m_index_list[i];
    }
}

mem_indexer* index_group :: swap_mem_indexer()
{
    // 阻塞于等待 m_index_list[1]清空
    pthread_mutex_lock(&m_mutex);
    while(m_index_list[MEM1] != NULL)
    {
        pthread_cond_wait(&m_mem_dump_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);
    // -1- 调换位置
    pthread_rwlock_wrlock(&m_list_rwlock);
    // 设置为只读状态
    m_index_list[MEM1] = m_index_list[MEM0];
    m_index_list[MEM1]->set_readonly();
    m_index_list[MEM0] = new mem_indexer(m_cell_size, m_bucket_size, m_headlist_size,
            m_blocknum_list, m_blocknum_list_size);
    pthread_rwlock_unlock(&m_list_rwlock);
    // -2- 通知merge线程可以把mem1持久化了
    pthread_cond_signal(&m_mem_dump_cond);
    // -3- 返回一个新的mem
    return dynamic_cast<mem_indexer*>(m_index_list[MEM0]);
}

uint32_t index_group :: merger(base_indexer* src1_indexer, base_indexer*
        src2_indexer, disk_indexer* dest_indexer)
{
    // 按照key的升序写入磁盘索引
    const uint32_t length = MAX_POSTINGLIST_SIZE * m_cell_size;
    void* src1_list = malloc(length);
    void* src2_list = malloc(length);
    MyThrowAssert(src1_list != NULL && src2_list != NULL && dest_indexer != NULL);
    ikey_t key1;
    ikey_t key2;
    src1_indexer->begin();
    src2_indexer->begin();
    dest_indexer->clear();
    dest_indexer->begin();
    uint32_t id = 0;
    while((!src1_indexer->is_end()) && (! src2_indexer->is_end()))
    {
        int32_t num1 = src1_indexer->itget(key1.sign64, src1_list, length);
        int32_t num2 = src2_indexer->itget(key2.sign64, src2_list, length);
//        ROUTN("num1[%u] num2[%u] key1[%llu] key2[%llu]", num1, num2, key1.sign64, key2.sign64);
        MyThrowAssert (num1 > 0 && num2 > 0);
        if (key1.sign64 < key2.sign64)
        {
            if (num1 > 0)
            {
                dest_indexer->set_posting_list(id, key1, src1_list, num1*m_cell_size);
                id++;
            }
            src1_indexer->next();
        }
        else if (key1.sign64 > key2.sign64)
        {
            if (num2 > 0)
            {
                dest_indexer->set_posting_list(id, key2, src2_list, num2*m_cell_size);
                id++;
            }
            src2_indexer->next();
        }
        else
        {
            memmove(&(((char*)src1_list)[num1*m_cell_size]), src2_list, num2*m_cell_size);
            dest_indexer->set_posting_list(id, key1, src1_list, (num1+num2)*m_cell_size);
            id++;
            src1_indexer->next();
            src2_indexer->next();
        }
    }

    while (!src1_indexer->is_end())
    {
        int32_t num1 = src1_indexer->itget(key1.sign64, src1_list, length);
//        ROUTN("num1[%u] key1[%llu]", num1, key1.sign64);
        if (num1 > 0)
        {
            dest_indexer->set_posting_list(id, key1, src1_list, num1*m_cell_size);
            id++;
        }
        src1_indexer->next();
    }
    while (!src2_indexer->is_end())
    {
        int32_t num2 = src2_indexer->itget(key2.sign64, src2_list, length);
//        ROUTN("num2[%u] key2[%llu]", num2, key2.sign64);
        if (num2 > 0)
        {
            dest_indexer->set_posting_list(id, key2, src2_list, num2*m_cell_size);
            id++;
        }
        src2_indexer->next();
    }

    dest_indexer->set_finish();
    free(src1_list);
    free(src2_list);
    return id;
}

void index_group :: update_day_indexer()
{
    pthread_mutex_lock(&m_mutex);
    // 等到m_index_list[1]不为NULL时，且这个dst_indexer(disk)为空闲时，执行合并过程
    while(m_index_list[MEM1] == NULL)
    {
        pthread_cond_wait(&m_mem_dump_cond, &m_mutex);
    }
    pthread_mutex_unlock(&m_mutex);

    // 合并过程:
    // -1- 设置当前的disk_indexer和空闲的disk_indexer
    disk_indexer* psrc_indexer = dynamic_cast<disk_indexer*>(m_index_list[DAY]);
    // 当前时间为指定的时间区域时dump到day的2目录，并以条件变量通知his_merger_thread
    // 如果不是特定的时间区域，则正常的dump到0/1目录中
    // dump到day2时，要检查day2是否是空的
    bool dumpToday2 = false;
    disk_indexer* pdst_indexer = NULL;
    if (get_cur_hour() >= m_dump_hour_min && get_cur_hour() <= m_dump_hour_max && m_day2_ready == false)
    {
        pdst_indexer = dynamic_cast<disk_indexer*>(m_day[2]);
        dumpToday2 = true;
    }
    else
    {
        pdst_indexer = dynamic_cast<disk_indexer*>((psrc_indexer == m_day[0]) ? m_day[1] : m_day[0]);
    }

    // -2- 执行合并
    struct   timeval btv;
    struct   timeval etv;
    PRINT ("DayMerger BEGIN DUMP2DAY2[%u] DAY2READY[%u] DUMP_MIN_HOUR[%u] DUMP_MAX_HOUR[%u] NOW_HOUR[%u].",
            dumpToday2, m_day2_ready, m_dump_hour_min, m_dump_hour_max, get_cur_hour());
    gettimeofday(&btv, NULL);
    uint32_t id_count_merged = merger(m_index_list[MEM1], psrc_indexer, pdst_indexer);
    gettimeofday(&etv, NULL);
    PRINT ("DayMerger FINISH. termCount[%u] time-consumed[%u]s", id_count_merged, (etv.tv_sec - btv.tv_sec));
    ROUTN ("DayMerger FINISH. termCount[%u] time-consumed[%u]s", id_count_merged, (etv.tv_sec - btv.tv_sec));

    // -3- 设置cur文件
    uint32_t cur = (pdst_indexer == m_day[0]) ? 0 : 1;
    set_cur_no(STR_DAY_INDEX_DIR, STR_INDEX_CUR_NO_FILE, cur);

    pthread_rwlock_wrlock(&m_list_rwlock);
    delete m_index_list[MEM1];
    m_index_list[MEM1] = NULL;
    // 把已经合并的数据清理掉
    m_index_list[DAY]->clear();
    // 指向新的DAY 0/1 INDEX
    m_index_list[DAY] = pdst_indexer;
    // 通知 m_index_list[1] == NULL 了
    if (dumpToday2)
    {
        m_day2_ready = true;
//        m_index_list[DAY2] = pdst_indexer;
    }
    else
    {
        m_index_list[DAY] = pdst_indexer;
    }
    // 通知等待swap的update线程
    pthread_cond_signal(&m_mem_dump_cond);
    pthread_rwlock_unlock(&m_list_rwlock);
    return;
}

void index_group :: update_his_indexer()
{
    if (get_cur_hour() < m_dump_hour_min || get_cur_hour() > m_dump_hour_max || m_day2_ready == false)
    {
        ROUTN("DAY2 is NOT READY.");
        return;
    }
    MyThrowAssert(m_index_list[DAY2] != NULL);
    // 合并过程:
    // -1- 设置当前的disk_indexer和空闲的disk_indexer
    disk_indexer* psrc_indexer = dynamic_cast<disk_indexer*>(m_index_list[HIS]);
    disk_indexer* pdst_indexer = dynamic_cast<disk_indexer*>((psrc_indexer == m_his[0]) ? m_his[1] : m_his[0]);

    // -2- 执行合并
    struct   timeval btv;
    struct   timeval etv;
    PRINT ("HisMerger BEGIN.");
    gettimeofday(&btv, NULL);
    uint32_t id_count_merged = merger(m_index_list[DAY2], psrc_indexer, pdst_indexer);
    gettimeofday(&etv, NULL);
    PRINT ("HisMerger FINISH. termCount[%u] time-consumed[%u]s", id_count_merged, (etv.tv_sec - btv.tv_sec));
    ROUTN ("HisMerger FINISH. termCount[%u] time-consumed[%u]s", id_count_merged, (etv.tv_sec - btv.tv_sec));

    // -3- 设置cur文件
    uint32_t cur = (pdst_indexer == m_his[0]) ? 0 : 1;
    set_cur_no(STR_HIS_INDEX_DIR, STR_INDEX_CUR_NO_FILE, cur);

    pthread_rwlock_wrlock(&m_list_rwlock);
    m_index_list[DAY2]->clear();
    m_index_list[HIS]->clear();
    m_index_list[HIS] = pdst_indexer;
    m_day2_ready = false;
    pthread_rwlock_unlock(&m_list_rwlock);
    return;
}

mem_indexer*  index_group :: get_cur_mem_indexer()
{
    return dynamic_cast<mem_indexer*>(m_index_list[MEM0]);
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
            int tmpnum = m_index_list[i]->get_posting_list(strTerm, &((char*)buff)[offset], length-offset);
            if (tmpnum > 0)
            {
                lstnum += tmpnum;
                offset = lstnum * m_cell_size;
                printf("-- index_no[%u] lstnum[%u] --\n", i, tmpnum);
                for (int32_t k=0; k<lstnum; k++)
                {
                    printf("[%u] ", ((uint32_t*)buff)[k]);
                }
                printf("\n");
            }
        }
    }
    pthread_rwlock_unlock(&m_list_rwlock);
    return lstnum;
}

//int32_t index_group :: set_posting_list(const char* strTerm, const void* buff)
//{
//    mem_indexer* pindexer = get_cur_mem_indexer();
//    int setret = pindexer->set_posting_list(strTerm, buff);
//    if (postinglist::FULL == setret)
//    {
//        ALARM( "SET POSTING LIST ERROR. LIST FULL, GO TO SWITCH. ID[%u]", id);
//        pindexer = myIndexGroup->swap_mem_indexer();
//        ROUTN( "THANKS GOD, SWITCH OK. ID[%u]", id);
//        MySuicideAssert(postinglist::OK == pindexer->set_posting_list(vstr[i].c_str(), &id));
//        have_swap = true;
//    }
//    return 0;
//}


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

uint32_t index_group :: get_cur_hour()
{
    time_t now;
    struct tm mytm;

    time(&now);
    localtime_r(&now, &mytm);
    return mytm.tm_hour;
}
