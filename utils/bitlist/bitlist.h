/*
 * bitlist 处理结构体数组mmap的存取问题
 * 与structmask配合使用，疗效更好。
 */
#ifndef _BITLIST_H_
#define _BITLIST_H_
#include <stdint.h>

class bitlist
{
    private:
        static const uint32_t MAX_FILENAME_LENGTH = 128;
        int m_fd;
        bitlist();
        bitlist(const bitlist&);

    public:
        uint32_t* puint;
        uint32_t  m_filelength;
        uint32_t  m_cellsize;
		uint32_t  m_cellcount;

    public:
        bitlist(const char* dir, const char* file,
				const uint32_t cellsize, const uint32_t cellcount);
        ~bitlist();
};

#endif
