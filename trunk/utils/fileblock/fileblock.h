#include <stdint.h>

class fileblock
{
    private:
        static const uint32_t MAX_FILE_SIZE = 2*1024*1024*1000; 
        static const uint32_t MAX_FILE_NO   = 1024; 
        const uint32_t m_cell_size;
        int32_t        m_fd[MAX_FILE_NO];
        uint32_t       m_max_file_no;
        uint32_t       m_max_num_per_file;
        char           m_fb_dir[128];
        char           m_fb_name[128];
        int32_t        m_it;

        fileblock();
        fileblock(const fileblock &);
        int32_t detect_file();
    public:
        fileblock(const char* dir, const char* filename, const uint32_t cell_size);
        int32_t write(const uint32_t offset, const char* buff);
        int32_t read(const uint32_t offset, char* buff, const uint32_t length);
        int32_t remove();
        void begin();
        int32_t write_next(char* buff);
        int32_t read_next(char* buff, const uint32_t length);
};