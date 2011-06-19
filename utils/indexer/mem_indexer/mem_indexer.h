#ifndef _MEM_INDEXER_H_
#define _MEM_INDEXER_H_
#include <stdint.h>
#include "indexer.h"
#include "postinglist.h"

class mem_indexer : public indexer
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
        int32_t get_posting_list(const char* strTerm, char* buff, const uint32_t length);
        int32_t set_posting_list(const char* strTerm, const char* buff);
};
#endif
