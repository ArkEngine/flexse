#ifndef  __FILELINKBLOCK_H_
#define  __FILELINKBLOCK_H_
#include <stdint.h>
#include <pthread.h>

class FileLinkBlock
{
    private:
        static const uint32_t BLOCK_MAX_SIZE = (1<<22);     ///< ÿ�����4M
        static const uint32_t FILE_MAX_SIZE  = (1<<30);     ///< ÿ���ļ����1G
        static const uint32_t MAGIC_NUM      = 0x19831201;  ///< ħ������
        static const uint32_t CHECK_INTERVAL = 500000;      ///< ���ļ��ޱ仯ʱ�����߼��, ΢�뵥λ
        static const char* const BASE_DATA_PATH;
        static const char* const BASE_OFFSET_PATH;
        struct file_link_block_head
        {
            uint32_t magic_num;       ///< ����ʶ���У��head�Ŀ�ͷ
            uint32_t block_id;        ///< �����д�0���������
            uint32_t timestamp;       ///< д���ʱ��㣬�������ڻط�ʱ�жϵ�����
            uint32_t log_id;          ///< ����׷������
            uint32_t check_sum1;      ///< ��block_buff����ڴ��У��ֵ
            uint32_t check_sum2;      ///< ��block_buff����ڴ��У��ֵ
            uint32_t reserved[2];     ///< Ԥ��λ
            uint32_t block_size;      ///< block_buff�ĳ���
            char     block_buff[0];   ///< ������ݣ������ƽṹ
        };

        int       m_flb_w_fd;         ///< ��ǰ�ļ�������д����
        char      m_flb_path[128];    ///< ��ǰ�ļ��������ļ���
        char      m_flb_name[128];    ///< ��ǰ�ļ��������ļ�·��
        uint32_t  m_split_mode;       ///< ����ʱ����ߴ�С�з֣����ĳ��ʱ���ڳ�����С���ƣ���ʱ���ڵ��ļ���ҲҪ�з�
        uint32_t  m_last_file_no;     ///< ���һ���ļ����
        uint32_t  m_block_id;         ///< ��ǰ�����

        int       m_flb_r_fd;         ///< ��Ϣ���еĶ�ȡ���
        uint32_t  m_flb_read_offset;
        uint32_t  m_flb_read_file_no;
        uint32_t  m_flb_read_block_id;
        char      m_read_file_name[128];
        char      m_channel_name[128];

        pthread_mutex_t m_mutex;     ///< д��

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
