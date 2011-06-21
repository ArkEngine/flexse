#ifndef _INDEX_GROUP_H_
#define _INDEX_GROUP_H_
#include "mem_indexer.h"
#include "disk_indexer.h"
#include <pthread.h>

class index_group
{
    private:
        enum{
            MEM = 0,
            DAY = 1,
            MON = 2
        };
        mem_indexer  m_mem[2];
        disk_indexer m_day[2];
        disk_indexer m_his[2];
        struct index_info_t
        {
            base_index* pindex;
            bool        empty;
        };

        // TODO 读写锁效率更好一点
        pthread_mutex_t m_mutex;

        vector<index_info_t> m_index_list[4];

        uint32_t get_cur_no(const char* dir, const char* file);
    public:
        index_group();
        ~index_group();
        // 当update_thread写满(or定期?)一个mem时:
        // -1- 把这个mem放入group中，调换mem的位置(保证各个索引的ID顺序)
        // -2- 返回另一个空闲的mem(swap到头部了)给update_thread继续写
        // -3- 启动持久化过程(直接dump或merge)
        mem_indexer*  swap_mem_indexer(mem_indexer*);
        // 更新day_indexer时:
        // -1- 替换掉m_index_list[2]中的disk_indexer
        // -2- 把m_index_list[1]中的mem_index标记为free状态
        void          update_day_indexer(disk_indexer*);
        // 更新his_indexer时:
        // -1- 替换掉m_index_list[3]中的disk_indexer
        // -2- 把m_index_list[2]中的disk_index标记为free状态
        // -3- 检查是否有mem需要直接写入day_indexer
        void          update_his_indexer(disk_indexer*);
        mem_indexer*  get_cur_mem_indexer();
        disk_indexer* get_cur_day_indexer();
        disk_indexer* get_cur_his_indexer();

        int32_t get_posting_list(const char* strTerm, void* buff, const uint32_t length);
};

#endif
