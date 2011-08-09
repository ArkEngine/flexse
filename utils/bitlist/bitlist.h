/*
 * bitlist 处理结构体数组mmap的存取问题
 * 与structmask配合使用，疗效更好。
 */
#ifndef _BITLIST_H_
#define _BITLIST_H_
#include <stdint.h>

#define BIT_COUNT 32
#define BIT_MASK  31

#define _GET_BITMAP_(mybitlist, offset) \
        (((mybitlist.puint[(offset)/BIT_COUNT]) >> (offset&BIT_MASK)) & 1)
#define _SET_BITMAP_1_(mybitlist, offset) \
        ((mybitlist.puint[(offset)/BIT_COUNT]) |= (1<<((offset)&BIT_MASK)))
#define _SET_BITMAP_0_(mybitlist, offset) \
        ((mybitlist.puint[(offset)/BIT_COUNT]) &= (~(1<<((offset)&BIT_MASK))))

class bitlist
{
    private:
        static const uint32_t MAX_FILENAME_LENGTH = 128;
        int m_fd;
        int m_filesize;
        bitlist();
        bitlist(const bitlist&);

    public:
        uint32_t* puint;
        uint32_t  m_filelength;
        uint32_t  m_cellsize;
		uint32_t  m_cellcount;

    public:
        bitlist(const char* dir, const char* file,
				const uint32_t cellsize, const uint32_t filesize);
        ~bitlist();
};

#endif
