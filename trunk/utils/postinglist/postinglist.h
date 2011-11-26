#include <stdint.h>
#include <pthread.h>
#include "memblocks.h"

#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 500 /* 支持读写锁 */
#endif

class postinglist
{
    private:
        static const uint32_t END_OF_LIST = 0xFFFFFFFF;
        // postinglist 的危险水位线，当还剩下 HEAD_LIST_WATER_LINE 个
        // term空闲块时，返回 NEARLYFULL，用户可以选择继续插入，但是
        // 已经快满了，需要考虑换个新的了。
        static const uint32_t HEAD_LIST_WATER_LINE = 10000;
        uint32_t*  m_bucket;
        uint32_t*  m_sortlist;
        memblocks* m_memblocks;
        int32_t    mem_merge();
        uint32_t   m_postinglist_cell_size;
        uint32_t   m_bucket_size;
        uint32_t   m_bucket_mask;
        uint32_t   m_mem_base_size;
        bool       m_readonly;
        pthread_rwlock_t   m_mutex;

		struct mem_link_t
		{
			uint32_t    used_size;   ///< 已经使用的内存大小，包括头部mem_link_t的大小
			uint32_t    self_size;   ///< 这个内存块本身大小
			mem_link_t* next;
//			uint32_t    reserved[3]; ///< 用来站位的，防止m_postinglist_cell_size超过mem_link_t，我知道这么做很愚蠢，这是暂时的。
		};

		struct term_head_t
		{
			uint64_t     sign64;
			uint32_t     next;
			mem_link_t*  mem_link;
		};

        term_head_t* m_headlist;
        term_head_t* m_headlist_sort;
        uint32_t   m_headlist_sort_it;
		uint32_t   m_headlist_size;
		uint32_t   m_headlist_used;

        postinglist();
        postinglist(const postinglist&);

        void memlinkcopy(mem_link_t* mem_link, const void* buff, const uint32_t length);
        mem_link_t* memlinknew(const uint32_t memsiz, mem_link_t* next);
        static int key_compare(const void *p1, const void *p2);

    public:
        enum
        {
            NEARLY_FULL = -2,
            FULL = -1,
            OK = 0,
        };

        postinglist( const uint32_t  posting_cell_size, const uint32_t  bucket_size,
                const uint32_t  headlist_size, const uint32_t* mblklist, const uint32_t  mblklist_size);
        ~postinglist();
        int32_t get (const uint64_t& key, void* buff, const uint32_t length);
        int32_t set (const uint64_t& key, const void* buff);
        void    begin();
        void    next();
        int32_t itget(uint64_t& key, void* buff, const uint32_t length);
        bool    is_end();
        bool    empty();
        void    set_readonly(bool readonly);
        bool    iswritable();
        void    clear(); // 清理掉postinglist中的数据，恢复到初始化的状态
};
