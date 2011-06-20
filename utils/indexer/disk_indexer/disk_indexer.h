#ifndef _DISK_INDEXER_H_
#define _DISK_INDEXER_H_
#include <stdint.h>
#include "indexer.h"
#include "fileblock.h"
#include "diskv.h"
#include <vector>
using namespace std;

class disk_indexer : public indexer
{
    private:
        static const char* const FORMAT_SECOND_INDEX;
        static const uint32_t MAX_FILE_LENGTH = 128;
        fileblock m_fileblock;
        diskv     m_diskv;

        struct fb_index_t
        {
            ikey_t             ikey;
            diskv::diskv_idx_t idx;
        };

        struct second_index_t
        {
            uint32_t  milestone;
            ikey_t    ikey;
            bool operator < (const second_index_t& right)
            {
                    return this->ikey.sign64 < right.ikey.sign64;
            }
        };

        vector<second_index_t> second_index;

        disk_indexer();
        disk_indexer(const disk_indexer&);
    public:
        disk_indexer(const char* dir, const char* iname);
        ~disk_indexer();
        int32_t get_posting_list(const char* strTerm, char* buff, const uint32_t length);
        /*这个接口只能用于连续写入*/
        int32_t set_posting_list(const uint32_t id, const ikey_t& ikey, const char* buff, const uint32_t length);
};
#endif
