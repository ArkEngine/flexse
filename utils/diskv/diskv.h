#ifndef	_DISKV_H_
#define	_DISKV_H_
#include <stdint.h>
#include <stdio.h>

class diskv
{
	public:
		typedef struct diskv_idx_t {
            union
            {
                uint64_t sign64;
                struct {
                    uint32_t id1;
                    uint32_t id2;
                };
            };                        ///< this is index
            uint32_t fno      :  7;   ///< 1<<7 = 128 files
            uint32_t dlen     : 25;   ///< max doc size is 1<<25 = 32M
            uint32_t off      : 31;   ///< max file size is 2G
            uint32_t reserved :  1;   ///< reserved bit
        };

    private:
        static const uint32_t FASTDI_FD_MAXNUM    = 128;
        static const uint32_t FASTDI_PATH_MAXSIZE = 1024;
        static const uint32_t FASTDI_FILE_MAXSIZE = 2*1024*1024*1000;
        static const uint32_t FASTDI_ITEM_MAXSIZE = 1<<25;
        static const uint32_t FASTDI_DATABUF_SIZE = 1024*1024*10;

        static const char* const FASTDI_IDX_FILE_FMT;
        static const char* const FASTDI_DATA_FILE_FMT;
        static const char* const FASTDI_STATUS_FILE_FMT;

        int       m_read_fd[FASTDI_FD_MAXNUM];
        int       m_append_fd;
        fileblock m_fileblock;                       ///< fileblock for operate the index
        uint32_t  m_fnum;                            ///< data files number >=1
        bool      m_iswritesync;                     ///< need sync into system disk cache
        bool      m_isreadonly;
        char      m_module[FASTDI_PATH_MAXSIZE];     ///< data file name
        char      m_di_path [FASTDI_PATH_MAXSIZE];   ///< data file dir
        char      m_idx_name[FASTDI_PATH_MAXSIZE];   ///< index file name

        // iterator
        bool m_done;                                ///< is done?
        char* m_pdatabuf_idx;                       ///< setvbuf for read index
        char* m_pdatabuf_di;                        ///< setvbuf for read data
        uint32_t m_pdatabuf_idx_size;               ///< index buffer
        uint32_t m_pdatabuf_di_size;                ///< data  buffer
        FILE* m_idx_pfile;                          ///< FILE* for index
        FILE* m_di_pfile;                           ///< FILE* for data
        uint32_t m_idx_curfno;                      ///< current index file no
        uint32_t m_di_curfno;                       ///< current data file no
        diskv_idx_t m_curidx;                       ///< current index info
        uint32_t m_iterator_id;                     ///< iterator id

        diskv();
        bool isneednewfile(const uint32_t appendsize);
        bool isfileexist( const char* filename );
        uint32_t getfilesize( const char* name );
        uint32_t getfilesize( FILE* fp );
        int openfile(const uint32_t fno, const bool readonly );
        int iterator_read(void* buff, const uint32_t buffsize);

    public:

        diskv(const char* di_path, const char* idx_path, const char* module, bool readonly, bool writesync);
        ~diskv();

        int getdata(const diskv_idx_t* idx, char* data, const uint32_t datasize);

        int append(diskv_idx_t* idx,const char* data, const uint32_t datasize);
        int append(const uint32_t id,const char* data, const uint32_t datasize);

        int sync();

        int remove();

        int readidx(const uint32_t idx_id, diskv_idx_t* idx);

        int writeidx(const uint32_t idx_id,const diskv_idx_t* idx);

        void begin();

        int next(uint32_t& id, diskv_idx_t& idx, void* buff, const uint32_t buffsize);

        bool end();
};

#endif
