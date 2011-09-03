#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bitlist.h"
#include "structmask.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        return 1;
    }
    const uint32_t SIZE = atoi(argv[1]);
    bitlist mybitlist("./", "mem.test", 1, SIZE);

    mask_item_t uint_item = {0xFFFFFFFF, 0, 0, 1, 0};

    for (uint32_t i=0; i<SIZE/sizeof(uint32_t); i++)
    {
        _SET_LIST_VALUE_(mybitlist.puint, i, uint_item, i);
    }

    for (uint32_t i=0; i<SIZE/sizeof(uint32_t); i++)
    {
        assert ( i == _GET_LIST_VALUE_(mybitlist.puint, i, uint_item));
    }

//    for (uint32_t i=0; i<SIZE; i+=2)
//    {
//        _SET_BITMAP_1_(mybitlist, i);
//        assert(1 == _GET_BITMAP_(mybitlist, i));
//    }
//
//    for (uint32_t i=0; i<SIZE; i+=2)
//    {
//        assert(1 == _GET_BITMAP_(mybitlist, i));
//    }
//
//    for (uint32_t i=0; i<SIZE; i+=2)
//    {
//        _SET_BITMAP_0_(mybitlist, i);
//        assert(0 == _GET_BITMAP_(mybitlist, i));
//    }
//
//    for (uint32_t i=1; i<SIZE; i+=2)
//    {
//        _SET_BITMAP_1_(mybitlist, i);
//        assert(1 == _GET_BITMAP_(mybitlist, i));
//    }
//
//    for (uint32_t i=1; i<SIZE; i+=2)
//    {
//        assert(1 == _GET_BITMAP_(mybitlist, i));
//    }
//
//    for (uint32_t i=1; i<SIZE; i+=2)
//    {
//        _SET_BITMAP_0_(mybitlist, i);
//        assert(0 == _GET_BITMAP_(mybitlist, i));
//    }
//
//    for (uint32_t i=0; i<SIZE; i++)
//    {
//        assert(0 == _GET_BITMAP_(mybitlist, i));
//    }
//    remove("mem.test");
    return 0;
}
