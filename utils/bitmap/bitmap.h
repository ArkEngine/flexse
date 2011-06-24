#ifndef _BITMAP_H_
#define _BITMAP_H_
#include <stdint.h>

#define BIT_COUNT 32
#define BIT_MASK  31

#define _GET_BITMAP_(mybitmap, offset) \
        (((mybitmap.puint[(offset)/BIT_COUNT]) >> (offset&BIT_MASK)) & 1)
#define _SET_BITMAP_1_(mybitmap, offset) \
        ((mybitmap.puint[(offset)/BIT_COUNT]) |= (1<<((offset)&BIT_MASK)))
#define _SET_BITMAP_0_(mybitmap, offset) \
        ((mybitmap.puint[(offset)/BIT_COUNT]) &= (~(1<<((offset)&BIT_MASK))))

class bitmap
{
    private:
        static const uint32_t MAX_FILENAME_LENGTH = 128;
        int m_fd;
        int m_filesize;
        bitmap();
        bitmap(const bitmap&);

    public:
        uint32_t* puint;
        uint32_t  m_filelength;

    public:
        bitmap(const char* dir, const char* file, const uint32_t filesize);
        ~bitmap();
};

#endif
