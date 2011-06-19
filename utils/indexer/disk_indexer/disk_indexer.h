#ifndef _DISK_INDEXER_H_
#define _DISK_INDEXER_H_
#include <stdint.h>
#include "indexer.h"
#include "fileblock.h"
#include "diskv.h"

class disk_indexer : public indexer
{
    private:
        // vars
        fileblock m_fileblock;
        diskv     m_diskv;

        struct fb_index_t
        {
            ikey_t             ikey;
            diskv::diskv_idx_t idx;
        };

        // methods
        disk_indexer(const disk_indexer&);
    public:
        disk_indexer(const char* dir, const char* iname);
        ~disk_indexer();
        int32_t get_posting_list(const char* strTerm, char* buff, const uint32_t length);
        /*这个接口只能用于连续写入*/
        int32_t set_posting_list(const uint32_t id, const ikey_t& ikey, const char* buff, const uint32_t length);
};
#endif
