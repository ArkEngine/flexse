#ifndef _MEM_INDEXER_H_
#define _MEM_INDEXER_H_
#include <stdint.h>
#include "indexer.h"

class mem_indexer : public indexer
{
    private:
        // vars
        uint32_t m_posting_cell_size;

        // methods
        mem_indexer(const mem_indexer&);
    public:
        mem_indexer(const uint32_t posting_cell_size);
        ~mem_indexer();
        int32_t get_posting_list(const string& strterm, char* buff, const uint32_t buff_length);
        int32_t set_posting_list(const string& strterm, const char* buff);
};
#endif
