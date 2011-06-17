#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "postinglist.h"

int main(const int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        return 1;
    }
    const uint32_t SIZE = atoi(argv[1]);
    postinglist mypostinglist(sizeof(uint32_t));
//    for (uint32_t i=1; i<SIZE; i++)
//    {
//        mypostinglist.set(i, (char*)&i);
//    }
//    for (uint32_t i=1; i<SIZE; i++)
//    {
//        uint32_t t=0;
//        mypostinglist.get(i, (char*)&t, sizeof(uint32_t));
//        assert(i == t);
//    }

    const uint64_t ukey = 4;
    uint32_t* ubuff = (uint32_t*)malloc(SIZE* 2 * sizeof(uint32_t));
    for (uint32_t i=0; i<SIZE; i++)
    {
        mypostinglist.set(ukey, (char*)&i);
    }
    int32_t rnum = mypostinglist.get(ukey, (char*)ubuff, SIZE * 2 * sizeof(uint32_t));
    printf("rnum: %u\n", rnum);
    for (uint32_t i=0; i<SIZE; i++)
    {
        assert(ubuff[i] == (SIZE - 1 - i));
    }
    free(ubuff);
    return 0;
}
