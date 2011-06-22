#ifndef _MEM_INDEXER_H_
#define _MEM_INDEXER_H_
#include <stdint.h>
#include "base_indexer.h"
#include "postinglist.h"

class mem_indexer : public base_indexer
{
    private:
        // vars
        uint32_t m_posting_cell_size;
        postinglist m_postinglist;

        // methods
        mem_indexer(const mem_indexer&);
        mem_indexer();
    public:
        mem_indexer(const uint32_t posting_cell_size, const uint32_t  bucket_size,
                const uint32_t  headlist_size, const uint32_t* mblklist, const uint32_t  mblklist_size);
        ~mem_indexer();
        int32_t get_posting_list(const char* strTerm, void* buff, const uint32_t length);
        int32_t set_posting_list(const char* strTerm, const void* buff);
        void clear();
        void set_readonly();

        void begin();
        int32_t get_and_next(uint64_t& key, void* buff, const uint32_t length);
        bool is_end();
};
#endif
