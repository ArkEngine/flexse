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
        uint32_t   m_memsize[8];

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
		uint32_t   m_headlist_size;
		uint32_t   m_headlist_used;

        postinglist();
        postinglist(const postinglist&);

    public:
        postinglist(const uint32_t posting_cell_size);
        ~postinglist();
        int32_t get (const uint64_t& key, char* buff, const uint32_t length);
        int32_t set (const uint64_t& key, char* buff);
        int32_t sort();
        int32_t dump();
        int32_t merge();
        int32_t begin();
        int32_t next(const uint64_t& key, char* buff, const uint32_t length);
        bool    isend();
};
