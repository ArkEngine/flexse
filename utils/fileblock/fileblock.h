#ifndef _FILEBLOCK_H_
#define _FILEBLOCK_H_
#include <stdint.h>

class fileblock
{
    private:
        static const char* const FORMAT_FILE;
        static const char* const FORMAT_PATH;
        static const uint32_t MAX_FILE_SIZE = 2*1024*1024*1000; 
//        static const uint32_t MAX_FILE_SIZE = 4000; 
        static const uint32_t MAX_FILE_NO   = 1024; 
        const uint32_t m_cell_size;
        const uint32_t m_cell_num_per_file;
        int32_t        m_fd[MAX_FILE_NO];
        uint32_t       m_max_file_no;
        uint32_t       m_last_file_offset;
        char           m_fb_dir[128];
        char           m_fb_name[128];
        uint32_t       m_it;

        fileblock();
        fileblock(const fileblock &);
        fileblock& operator =(const fileblock& p);
        int32_t  detect_file();
        uint32_t getfilesize( const char* name );
    public:
        fileblock(const char* dir, const char* filename, const uint32_t cell_size);
        ~fileblock();
        int32_t set(const uint32_t offset, const void* buff);
        int32_t get(const uint32_t offset, void* buff, const uint32_t length);
        int32_t get(const uint32_t offset, const uint32_t count, void* buff, const uint32_t length);
        int32_t clear();
        int32_t get_cell_count(); // TODO const

        void begin();
        void next();
        int32_t set_and_next(void* buff);
        int32_t itget(void* buff, const uint32_t length);
        bool is_end();
};
#endif
