#ifndef _DETAILDB_H_
#define _DETAILDB_H_
#include <stdint.h>
#include "fileblock.h"
#include "diskv.h"

class detaildb
{
    private:
        // vars
        fileblock m_fileblock;
        diskv     m_diskv;

        struct dt_index_t
        {
            uint32_t           ino;
            diskv::diskv_idx_t idx;
        };

        // methods
        detaildb(const detaildb&);
        detaildb();
    public:
        detaildb(const char* dir, const char* iname);
        ~detaildb();
        int32_t get(const uint32_t ino, void* buff, const uint32_t length);
        int32_t set(const uint32_t ino, const void* buff, const uint32_t length);
};
#endif
