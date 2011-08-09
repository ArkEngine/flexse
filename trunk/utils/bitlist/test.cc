#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "bitlist.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        return 1;
    }
    const uint32_t SIZE = atoi(argv[1]);
    bitlist mybitlist("./", "mem.test", 1, SIZE);

    for (uint32_t i=0; i<SIZE; i++)
    {
        assert(0 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mybitlist, i);
        assert(1 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mybitlist, i);
        assert(0 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_1_(mybitlist, i);
        assert(1 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        assert(1 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=1; i<SIZE; i+=2)
    {
        _SET_BITMAP_0_(mybitlist, i);
        assert(0 == _GET_BITMAP_(mybitlist, i));
    }

    for (uint32_t i=0; i<SIZE; i++)
    {
        assert(0 == _GET_BITMAP_(mybitlist, i));
    }
    remove("mem.test");
    return 0;
}
