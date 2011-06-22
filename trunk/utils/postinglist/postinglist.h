#include <stdint.h>
#include "memblocks.h"

class postinglist
{
    private:
        static const uint32_t END_OF_LIST = 0xFFFFFFFF;
        uint32_t*  m_bucket;
        uint32_t*  m_sortlist;
        memblocks* m_memblocks;
        int32_t    mem_merge();
        uint32_t   m_postinglist_cell_size;
        uint32_t   m_bucket_size;
        uint32_t   m_bucket_mask;
        uint32_t   m_mem_base_size;
        bool       m_isfree;

		struct mem_link_t
		{
			uint32_t    used_size; ///< 已经使用的内存大小，包括头部mem_link_t的大小
			uint32_t    self_size; ///< 这个内存块本身大小
			uint32_t    next_size; ///< 下个内存块的本身大小
			mem_link_t* next;
		};

		struct term_head_t
		{
			uint64_t     sign64;
			uint32_t     list_num;
			uint32_t     next;
			uint32_t     list_buffer_size;
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
            FULL = -1,
            OK = 0,
        };

        postinglist( const uint32_t  posting_cell_size, const uint32_t  bucket_size,
                const uint32_t  headlist_size, const uint32_t* mblklist, const uint32_t  mblklist_size);
        ~postinglist();
        int32_t get (const uint64_t& key, void* buff, const uint32_t length);
        int32_t set (const uint64_t& key, const void* buff);
        int32_t begin();
        int32_t get_and_next(uint64_t& key, char* buff, const uint32_t length);
        bool    isend();
        int32_t finish();
        void    set_free(bool free_status);
        bool    isfree();
        int32_t reset(); // 清理掉postinglist中的数据，恢复到初始化的状态
};
