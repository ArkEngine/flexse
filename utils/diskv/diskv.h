#ifndef _DISKV_H_
#define _DISKV_H_
#include <stdint.h>

// 危险的是，没有fsync.

class diskv
{
    private:
        static const char* const FORMAT_FILE;
        static const char* const FORMAT_PATH;
        static const uint32_t MAX_PATH_LENGTH  = 128;
        static const uint32_t MAX_FILE_NO      = 128;
        static const uint32_t MAX_FILE_SIZE    = 2*1024*1024*1000;
        static const uint32_t MAX_DATA_SIZE    = 1<<25;

        int      m_read_fd[MAX_FILE_NO];
        int      m_append_fd;
        char     m_dir[MAX_PATH_LENGTH];
        char     m_module[MAX_PATH_LENGTH];
        uint32_t m_max_file_no;
        uint32_t m_last_file_offset;

        diskv(const diskv&);
        diskv();

        int      detect_file();
        void     check_new_file(const uint32_t length);
        uint32_t getfilesize(const char* filename);
    public:
        struct diskv_idx_t
        {
            uint32_t file_no  :  7;   ///< 1<<7 = 128 files
            uint32_t data_len : 25;   ///< max doc size is 1<<25 = 32M
            uint32_t offset   : 31;   ///< max file size is 2G
            uint32_t reserved :  1;   ///< reserved bit
        };

        diskv(const char* dir, const char* module);
        ~diskv();
        int set(diskv_idx_t& idx, const void* buff, const uint32_t length);
        int get(const diskv_idx_t& idx, void* buff, const uint32_t length);
        void clear();
};
#endif
