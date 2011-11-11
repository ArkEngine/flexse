#ifndef _INDEX_GROUP_H_
#define _INDEX_GROUP_H_

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500 /* 支持读写锁 */
#endif

#include "mem_indexer.h"
#include "disk_indexer.h"
#include "myutils.h"
#include <pthread.h>
#include <vector>
using namespace std;
using namespace flexse;

class index_group
{
    private:

        enum {MEM0 = 0, MEM1, DAY, DAY2, HIS,};

        static const uint32_t MAX_PATH_LENGTH = 128;
        static const uint32_t MAX_POSTINGLIST_SIZE = 1000000; ///> postinglist最多的cell个数

        static const char* const STR_INDEX_NAME;
        static const char* const STR_INDEX_CUR_NO_FILE;
        static const char* const STR_CHECK_POINT_FILE;
        static const char* const STR_DAY_INDEX_DIR;
        static const char* const STR_HIS_INDEX_DIR;
        static const char* const STR_FMT_DAY_INDEX_PATH;
        static const char* const STR_FMT_HIS_INDEX_PATH;

        // 消息队列的进度
        uint32_t m_file_no;
        uint32_t m_block_id;
        uint32_t m_dump_file_no;
        uint32_t m_dump_block_id;
        char m_check_point_file[128];

        uint32_t m_cell_size;
        uint32_t m_bucket_size;
        uint32_t m_headlist_size;
        uint32_t m_blocknum_list_size;
        uint32_t m_blocknum_list[32];

        base_indexer* m_mem[2];
        base_indexer* m_day[3]; // 0/1用于当mem满了时切换，2专门用于与his合并
        base_indexer* m_his[2];

        bool     m_his_merge_ready;
        bool     m_can_dump_day2;
        uint32_t m_dump_hour_min;
        uint32_t m_day2dump_day;  // 上一次dump到day2的日子
        uint32_t m_daydump_interval;  // daydump的时间间隔
        uint32_t m_dump_hour_max;
        uint32_t m_last_day_merge_timecost;

        // 读写锁效率更好一点
        pthread_mutex_t m_mutex;
        pthread_cond_t m_mem_dump_cond;
        pthread_rwlock_t m_list_rwlock;

        vector<base_indexer*> m_index_list;

        void     set_cur_no(const char* dir, const char* file, const uint32_t cur);
        uint32_t get_cur_no(const char* dir, const char* file);
        uint32_t merger(base_indexer* src1_indexer, base_indexer* src2_indexer, disk_indexer* dest_indexer);
        uint32_t get_cur_hour();
        uint32_t get_cur_day();
        bool     need_to_dump_day2();

        index_group (const index_group&);
        index_group();
        void _swap_mem_indexer();
        // 当update_thread写满(or定期?)一个mem时:
        // -1- 把这个mem放入group中，调换mem的位置(保证各个索引的ID顺序)
        // -2- 返回另一个空闲的mem(swap到头部了)给update_thread继续写
        // -3- 启动持久化过程(直接dump或merge)
        mem_indexer* swap_mem_indexer();
        mem_indexer*  get_cur_mem_indexer();
        void set_check_point(const uint32_t last_file_no, const uint32_t last_block_id);
    public:
        index_group(const uint32_t cell_size, const uint32_t bucket_size,
                const uint32_t headlist_size, const uint32_t* blocknum_list, const uint32_t blocknum_list_size);
        ~index_group();
        // 更新day_indexer时:
        // -1- 替换掉m_index_list[2]中的disk_indexer
        // -2- 把m_index_list[1]中的mem_index标记为free状态
        void update_day_indexer();
        // 更新his_indexer时:
        // -1- 替换掉m_index_list[3]中的disk_indexer
        // -2- 把m_index_list[2]中的disk_index标记为free状态
        // -3- 检查是否有mem需要直接写入day_indexer
        void update_his_indexer();

        int32_t get_posting_list(const char* strTerm, void* buff, const uint32_t length);
        int32_t set_posting_list(const uint32_t file_no, const uint32_t block_id, map<string, term_info_t>& term_map);

        // 设置dump到day2的时间段，min, max 在0-23之间，且min <= max，只会在这个时间段dump一次
        // 这也是每天例行启动his索引合并的设置时间段
        // 默认是凌晨1点到凌晨5点
        int set_dump2day2_timezone(const uint32_t min_hour, const uint32_t max_hour);
        // 设置间隔dump到day的时间间隔，默认是1小时, 3600s，最小不少于60，最多不多于43200(半天)
        // 单位是秒，到达这个时间间隔时，就启动dump，dump的磁盘索引，可能是day0,day1,day2
        int set_dump2day_interval(const uint32_t interval_seconds);
        // 消息队列进度点
        void get_check_point(uint32_t& last_file_no, uint32_t& last_block_id);
};

#endif
