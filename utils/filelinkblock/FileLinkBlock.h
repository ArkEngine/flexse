#ifndef  __FILELINKBLOCK_H_
#define  __FILELINKBLOCK_H_
#include <stdint.h>
#include <pthread.h>

class FileLinkBlock
{
    private:
        static const uint32_t BLOCK_MAX_SIZE = (1<<22);     ///< 每块最多4M
        static const uint32_t FILE_MAX_SIZE  = (1<<30);     ///< 每个文件最大1G
        static const uint32_t MAGIC_NUM      = 0x19831201;  ///< 魔幻数字
        static const uint32_t CHECK_INTERVAL = 500000;      ///< 当文件无变化时的休眠间隔, 微秒单位
        static const char* const BASE_DATA_PATH;
        static const char* const BASE_OFFSET_PATH;
        struct file_link_block_head
        {
            uint32_t magic_num;       ///< 用于识别和校验head的开头
            uint32_t block_id;        ///< 本块中从0递增的序号
            uint32_t timestamp;       ///< 写入的时间点，可以用于回放时判断的依据
            uint32_t log_id;          ///< 用于追查问题
            uint32_t check_sum1;      ///< 对block_buff这段内存的校验值
            uint32_t check_sum2;      ///< 对block_buff这段内存的校验值
            uint32_t reserved[2];     ///< 预留位
            uint32_t block_size;      ///< block_buff的长度
            char     block_buff[0];   ///< 块的内容，二进制结构
        };

        int       m_flb_w_fd;         ///< 当前文件块链的写入句柄
        char      m_flb_path[128];    ///< 当前文件块链的文件夹
        char      m_flb_name[128];    ///< 当前文件块链的文件路径
        uint32_t  m_split_mode;       ///< 按照时间或者大小切分，如果某段时间内超过大小限制，则时间内的文件块也要切分
        uint32_t  m_last_file_no;     ///< 最后一个文件标号
        uint32_t  m_block_id;         ///< 当前块序号

        int       m_flb_r_fd;         ///< 消息队列的读取句柄
        uint32_t  m_flb_read_offset;
        uint32_t  m_flb_read_file_no;
        uint32_t  m_flb_read_block_id;
        char      m_read_file_name[128];
        char      m_channel_name[128];

        pthread_mutex_t m_mutex;     ///< 写锁

        FileLinkBlock();
        int __write_message(const uint32_t logid, const char* buff, const uint32_t buff_size);
        int newfile(const char* strfile);
    public:
        FileLinkBlock(const char* path, const char* name);
        ~FileLinkBlock();
        int check_and_repaire();
        int write_message(const uint32_t logid, const char* buff, const uint32_t buff_size);
        int seek_message(const uint32_t file_no, const uint32_t file_offset, const uint32_t block_id);
        int seek_message(const uint32_t file_no, const uint32_t block_id);
        int read_message(uint32_t& log_id, uint32_t& block_id, char* buff, const uint32_t buff_size);
        int detect_file( );
        void set_channel(const char* channel_name);
        void log_offset();
        void load_offset(uint32_t &file_no, uint32_t& offset, uint32_t& block_id);
        int readn(int fd, char* buf, const uint32_t size);
};














#endif  //__FILELINKBLOCK_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
