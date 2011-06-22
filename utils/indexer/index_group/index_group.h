#ifndef _INDEX_GROUP_H_
#define _INDEX_GROUP_H_

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500 /* 支持读写锁 */
#endif

#include "mem_indexer.h"
#include "disk_indexer.h"
#include <pthread.h>
#include <vector>
using namespace std;

class index_group
{
    private:

        static const uint32_t MAX_PATH_LENGTH = 128;

        static const char* const STR_INDEX_NAME;
        static const char* const STR_INDEX_CUR_NO_FILE;
        static const char* const STR_DAY_INDEX_DIR;
        static const char* const STR_HIS_INDEX_DIR;
        static const char* const STR_FMT_DAY_INDEX_PATH;
        static const char* const STR_FMT_HIS_INDEX_PATH;

        uint32_t m_cell_size;
        uint32_t m_bucket_size;
        uint32_t m_headlist_size;
        uint32_t m_blocknum_list_size;
        uint32_t m_blocknum_list[32];

        base_indexer* m_mem[2];
        base_indexer* m_day[2];
        base_indexer* m_his[2];

        // TODO 读写锁效率更好一点
        pthread_mutex_t m_mutex;
        pthread_cond_t m_mem_dump_cond;
        pthread_rwlock_t m_list_rwlock;

        vector<base_indexer*> m_index_list;

        uint32_t get_cur_no(const char* dir, const char* file);

        index_group (const index_group&);
        index_group();
    public:
        index_group(const uint32_t cell_size, const uint32_t bucket_size,
                const uint32_t headlist_size, const uint32_t* blocknum_list, const uint32_t blocknum_list_size);
        ~index_group();
        // 当update_thread写满(or定期?)一个mem时:
        // -1- 把这个mem放入group中，调换mem的位置(保证各个索引的ID顺序)
        // -2- 返回另一个空闲的mem(swap到头部了)给update_thread继续写
        // -3- 启动持久化过程(直接dump或merge)
        mem_indexer* swap_mem_indexer();
        // 更新day_indexer时:
        // -1- 替换掉m_index_list[2]中的disk_indexer
        // -2- 把m_index_list[1]中的mem_index标记为free状态
        void update_day_indexer();
        // 更新his_indexer时:
        // -1- 替换掉m_index_list[3]中的disk_indexer
        // -2- 把m_index_list[2]中的disk_index标记为free状态
        // -3- 检查是否有mem需要直接写入day_indexer
        void update_his_indexer();
        mem_indexer*  get_cur_mem_indexer();
        disk_indexer* get_cur_day_indexer();
        disk_indexer* get_cur_his_indexer();

        int32_t get_posting_list(const char* strTerm, void* buff, const uint32_t length);
};

#endif
