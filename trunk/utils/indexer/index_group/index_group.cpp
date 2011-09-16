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
#include <errno.h>
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
    // 当dump到day的过程中，进程终止时，会导致索引不完整
    // 重启之后，读取到不完整索引时，disk_indexer会清空这个索引
    uint32_t day_cur = get_cur_no(STR_DAY_INDEX_DIR, STR_INDEX_CUR_NO_FILE);
    char index_dir[MAX_PATH_LENGTH];
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, day_cur);
    m_day[day_cur] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_index_list.push_back(m_day[day_cur]);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, 1 - day_cur);
    m_day[1-day_cur] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    snprintf(index_dir, sizeof(index_dir), STR_FMT_DAY_INDEX_PATH, 2);
    m_day[2] = new disk_indexer(index_dir, STR_INDEX_NAME, m_cell_size);
    m_index_list.push_back(m_day[2]);

    m_day[1 - day_cur]->clear();
    m_his_merge_ready = (false == m_day[2]->empty());
    m_can_dump_day2   = m_day[2]->empty();
    m_last_day_merge_timecost = 0;

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

    m_dump_hour_min = 1;
    m_dump_hour_max = 5;
    m_day2dump_day  = 0;
    m_daydump_interval = 3600;
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

    // -1- 调换位置
    _swap_mem_indexer();
    // -2- 通知merge线程可以把mem1持久化了
    pthread_cond_signal(&m_mem_dump_cond);
    pthread_mutex_unlock(&m_mutex);
    // -3- 返回一个新的mem
    return dynamic_cast<mem_indexer*>(m_index_list[MEM0]);
}


void index_group :: _swap_mem_indexer()
{
    // 设置为只读状态
    // TODO如果你这时候设置的话，可能会导致update线程中正在使用的MEM0变成只读的了。
    pthread_rwlock_wrlock(&m_list_rwlock);
    m_index_list[MEM1] = m_index_list[MEM0];
    m_index_list[MEM1]->set_readonly();
    m_index_list[MEM0] = new mem_indexer(m_cell_size, m_bucket_size, m_headlist_size,
            m_blocknum_list, m_blocknum_list_size);
    pthread_rwlock_unlock(&m_list_rwlock);
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

bool index_group :: need_to_dump_day2()
{
    if (! m_can_dump_day2)
    {
        // ./data/index/day/2目录还没准备好，正在被用于与his的合并
        return false;
    }

    if (m_last_day_merge_timecost > 60)
    {
        // dump到day的时间超过了120s，还是合并到day/2吧，否则会把内存索引撑爆
        // 设置这个的原因是为了考虑到快速建立数据的过程中，不能一天合并一次his
        return true;
    }

    // 如果今天已经低峰期dump，就别再dump了
    if (get_cur_hour() >= m_dump_hour_min && get_cur_hour() <= m_dump_hour_max && get_cur_day() != m_day2dump_day)
    {
        // 低峰时刻，开始dump
        return true;
    }

    return false;
}

void index_group :: update_day_indexer()
{
    pthread_mutex_lock(&m_mutex);
    struct timespec timeout;           //定义时间点
    timeout.tv_sec = time(NULL) + m_daydump_interval; //time(0) 代表的是当前时间 而tv_sec 是指的是秒
    timeout.tv_nsec = 0;               //tv_nsec 代表的是纳秒时间
    // 等到m_index_list[1]不为NULL时，且这个dst_indexer(disk)为空闲时，执行合并过程
    int ret = 0;
    while(m_index_list[MEM1] == NULL && ret != ETIMEDOUT)
    {
        ret = pthread_cond_timedwait(&m_mem_dump_cond, &m_mutex, &timeout);
    }

    if (ret == ETIMEDOUT && m_index_list[MEM0]->empty())
    {
        // 既然MEM0是空的，那还merge个毛
        pthread_mutex_unlock(&m_mutex);
        return;
    }

    if (ret == ETIMEDOUT)
    {
        // 表示过了时间间隔了，需要dump了
        // 因为修改m_index_list[MEM1]的swap_mem_indexer已经被m_mutex保护
        // m_index_list[MEM1] == NULL是恒成立的。
        // 但是m_index_list[MEM0]->empty()这个条件却可能在m_mutex保护期间改变
        // 因为更新线程可能时刻在向MEM0中插入数据
        PRINT("ret[%d] erron[%d] ETIMEDOUT[%d] EINTR[%d] empty[%d] ------",
                ret, errno, ETIMEDOUT, EINTR, m_index_list[MEM0]->empty());
        _swap_mem_indexer();
    }
    else
    {
        PRINT("ret[%d] erron[%d] ETIMEDOUT[%d] EINTR[%d] empty[%d] ++++++",
                ret, errno, ETIMEDOUT, EINTR, m_index_list[MEM0]->empty());
    }
    pthread_mutex_unlock(&m_mutex);

    // 合并过程:
    // -1- 设置当前的disk_indexer和空闲的disk_indexer
    disk_indexer* psrc_indexer = dynamic_cast<disk_indexer*>(m_index_list[DAY]);
    // 当前时间为指定的时间区域时dump到day的2目录，并以条件变量通知his_merger_thread
    // 如果不是特定的时间区域，则正常的dump到0/1目录中
    // dump到day2时，要检查day2是否是空的
    bool dumpToday2 = need_to_dump_day2();
    disk_indexer* pdst_indexer = NULL;
    if (dumpToday2)
    {
        m_can_dump_day2  = false;
        m_day2dump_day   = get_cur_day();
        pdst_indexer = dynamic_cast<disk_indexer*>(m_day[2]);
    }
    else
    {
        pdst_indexer = dynamic_cast<disk_indexer*>((psrc_indexer == m_day[0]) ? m_day[1] : m_day[0]);
    }

    // -2- 执行合并
    struct   timeval btv;
    struct   timeval etv;
    PRINT ("DayMerger BEGIN DUMP2DAY2[%u] DAY2CANWRITE[%u] DAY[%u]",
            dumpToday2, m_can_dump_day2, m_day2dump_day);
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
    // 通知 m_index_list[1] == NULL 了
    if (dumpToday2)
    {
        m_his_merge_ready = true;
        m_last_day_merge_timecost = 0;
    }
    else
    {
        // 指向新的DAY 0/1 INDEX
        m_index_list[DAY] = pdst_indexer;
        m_last_day_merge_timecost = etv.tv_sec - btv.tv_sec;
    }
    // 通知等待swap的update线程
    if (ret != ETIMEDOUT)
    {
        // 例行的dump后，不需要通知这个条件变量
        pthread_cond_signal(&m_mem_dump_cond);
    }
    pthread_rwlock_unlock(&m_list_rwlock);
    return;
}

void index_group :: update_his_indexer()
{
    if (m_his_merge_ready == false)
    {
        return;
    }
    MyThrowAssert(m_index_list[DAY2] != NULL);
    if (m_index_list[DAY2]->empty())
    {
        return;
    }
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
    m_his_merge_ready = false;
    m_can_dump_day2   = true;
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

int32_t index_group :: set_posting_list(const uint32_t id, const vector<term_info_t>& termlist)
{
    pthread_mutex_lock(&m_mutex);

    mem_indexer* pindexer = get_cur_mem_indexer();

    bool have_swap = false;
    bool nearly_full = false;
    for (uint32_t i=0; i<termlist.size(); i++)
    {
        // &(termlist[i].id) 表示从id的地址开始，拷贝postlist_cell_size个大小的内存
        // 不是仅仅拷贝id
        int setret = pindexer->set_posting_list(termlist[i].strTerm.c_str(), &(termlist[i].id));
        if (postinglist::FULL == setret)
        {
            ALARM( "SET POSTING LIST ERROR. LIST FULL, GO TO SWITCH. ID[%u]", id);
            pindexer = swap_mem_indexer();
            ROUTN( "THANKS GOD, SWITCH OK. ID[%u]", termlist[i].id);
            MySuicideAssert(postinglist::OK == pindexer->set_posting_list(termlist[i].strTerm.c_str(), &(termlist[i].id)));
            have_swap = true;
        }
        // 为什么需要一个 NEARLY_FULL 的状态呢
        // 这是考虑到当接受一个文档，分词后，进行插入工作，如果mem_indexer完全满了
        // 就需要插入到另一个mem_indexer中
        if (postinglist::NEARLY_FULL == setret)
        {
            nearly_full = true;
        }
    }
    if (nearly_full && !have_swap)
    {
        ROUTN( "SET POSTING LIST NEARLY_FULL. GOTO SWAP. ID[%u]", id);
        pindexer = swap_mem_indexer();
        ROUTN( "SET POSTING LIST NEARLY_FULL. SWAPED. ID[%u]", id);
    }
    pthread_mutex_unlock(&m_mutex);

    return 0;
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

uint32_t index_group :: get_cur_hour()
{
    time_t now;
    struct tm mytm;

    time(&now);
    localtime_r(&now, &mytm);
    return mytm.tm_hour;
}

uint32_t index_group :: get_cur_day()
{
    time_t now;
    struct tm mytm;

    time(&now);
    localtime_r(&now, &mytm);
    return 10000*mytm.tm_year + 100*mytm.tm_mon + mytm.tm_mday;
}

int index_group :: set_dump2day2_timezone(const uint32_t min_hour, const uint32_t max_hour)
{
    if (min_hour > 23 || max_hour > 23 || min_hour > max_hour)
    {
        return -1;
    }
    m_dump_hour_min = min_hour;
    m_dump_hour_max = max_hour;

    return 0;
}

int index_group :: set_dump2day_interval(const uint32_t interval_seconds)
{
    if (interval_seconds < 60 || interval_seconds > 43200)
    {
        return -1;
    }

    m_daydump_interval = interval_seconds;

    return 0;
}
