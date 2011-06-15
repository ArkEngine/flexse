#include <stdint.h>
#include "memblocks.h"

class postinglist
{
    private:
        uint32_t*  m_bucket;
        uint32_t*  m_list;
        memblocks* m_memblocks;
        int32_t    mem_merge();

		typedef struct _term_head_t
		{
			uint64_t sign64;
			void*    list_buffer;
			uint32_t list_num;
			uint32_t list_buffer_size;
			uint32_t next;
		}term_head_t;

		typedef struct _mem_link_t
		{
			uint32_t    left_size ;
			uint32_t    self_size;
			uint32_t    next_size;
			mem_link_t* next;
		}mem_link_t;
    public:
        int32_t get (const key_t& key, char* buff, const uint32_t length);
        int32_t set (const key_t& key, char* buff, const uint32_t length);
        int32_t sort();
        int32_t dump();
        int32_t merge();
        int32_t begin();
        int32_t next(key_t& key, char* buff, const uint32_t length);
        bool    isend();
};
