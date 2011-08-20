#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bitmap.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        return 1;
    }
    const uint32_t SIZE = atoi(argv[1]);
    bitmap* pmybitmap = NULL;

    pmybitmap = new bitmap("./", "mem.test", SIZE);
    bitmap& mybitmap = *pmybitmap;

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mybitmap, i);
        assert(0 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i++)
    {
        assert(0 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mybitmap, i);
        assert(1 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mybitmap, i);
        assert(0 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mybitmap, i);
        assert(1 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mybitmap, i);
        assert(0 == _GET_BITMAP_(mybitmap, i));
    }

    // delete前的设置0101010101...
    for (uint32_t i=0; i<SIZE; i++)
    {
        _SET_BITMAP_0_(mybitmap, i);
        assert(0 == _GET_BITMAP_(mybitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mybitmap, i);
        assert(1 == _GET_BITMAP_(mybitmap, i));
    }


    delete pmybitmap;

    pmybitmap = new bitmap("./", "mem.test", SIZE);
    bitmap& mmbitmap = *pmybitmap;

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        assert(0 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mmbitmap, i);
        assert(1 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mmbitmap, i);
        assert(0 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mmbitmap, i);
        assert(1 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mmbitmap, i);
        assert(0 == _GET_BITMAP_(mmbitmap, i));
    }

    for (uint32_t i=0; i<SIZE; i++)
    {
        _SET_BITMAP_0_(mmbitmap, i);
        assert(0 == _GET_BITMAP_(mmbitmap, i));
    }

    // test over bound
//    pmybitmap->puint[SIZE] = 1;

    delete pmybitmap;

    return 0;
}
