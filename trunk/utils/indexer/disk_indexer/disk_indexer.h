#ifndef _DISK_INDEXER_H_
#define _DISK_INDEXER_H_
#include <stdint.h>
#include "base_indexer.h"
#include "fileblock.h"
#include "diskv.h"
#include <vector>
using namespace std;

class disk_indexer : public base_indexer
{
    private:
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

        static const char* const FORMAT_SECOND_INDEX;
        static const uint32_t MAX_FILE_LENGTH = 128;
        static const uint32_t TERM_MILESTONE  = 1000;
        fileblock m_fileblock;
        diskv     m_diskv;
        bool      m_freeze;                  ///< true时表示只读状态，初始化时，如果没有二级索引则置为false
        second_index_t m_last_si;            ///< set过程中记录最后一个二级索引
        vector<second_index_t> second_index; ///< 二级索引
        char m_second_index_file[MAX_FILE_LENGTH];
        fb_index_t* m_index_block;           ///< 用于每次存放索引分块
        ///< 考虑使用tcm作为索引分块的cache. TODO

        disk_indexer();
        disk_indexer(const disk_indexer&);
        static int ikey_comp (const void *m1, const void *m2);
    public:
        enum
        {
            OK = 0,
            READ_ONLY = -1, ///< 调用过set_finish()后，变为只读状态;
            NOT_READY = -2, ///< 正在写入过程中，不提供查询服务
        };
        disk_indexer(const char* dir, const char* iname);
        ~disk_indexer();
        /*
         * set和get接口是相当危险的，这里的默认时，当连续的set之后，调用set_finish()方法之后才可以调用get方法
         * 一旦get之后，set接口就失效了。
         * 这相当于在磁盘上把索引生成之后，才可以使用这个索引。
         */
        int32_t get_posting_list(const char* strTerm, void* buff, const uint32_t length);
        /*这个接口只能用于连续写入*/
        int32_t set_posting_list(const uint32_t id, const ikey_t& ikey, const void* buff, const uint32_t length);
        void    set_finish();
};
#endif
