#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "postinglist.h"

int main(const int argc, char** argv)
{
    if (argc != 3)
    {
        printf("./test KEYSIZE VALUESIZE\n");
        return 1;
    }
    const uint32_t KEYSIZE = atoi(argv[1]);
    const uint32_t VLUSIZE = atoi(argv[2]);
    const uint32_t cell_size     = sizeof(uint32_t);
    const uint32_t bucket_size   = 20;
    const uint32_t headlist_size = 0x1000000;
    uint32_t blocknum_list[8];
    for (uint32_t i=0; i<8; i++)
    {
        blocknum_list[i] = (i==0)? 4096: blocknum_list[i-1]/2;
    }
    postinglist mypostinglist(cell_size, bucket_size, headlist_size, blocknum_list, 8);
//    for (uint32_t i=0; i<SIZE; i++)
//    {
//        if (0 != mypostinglist.set(i, (char*)&i))
//        {
//            printf("id[%u]\n", i);
//            break;
//        }
//    }
//    for (uint32_t i=0; i<SIZE; i++)
//    {
//        uint32_t t=0;
//        mypostinglist.get(i, (char*)&t, sizeof(uint32_t));
//        assert(i == t);
//    }

    uint32_t* ubuff = (uint32_t*)malloc(VLUSIZE* 2 * sizeof(uint32_t));
    for (uint32_t ukey=0; ukey<KEYSIZE; ukey++)
    {
        for (uint32_t i=0; i<VLUSIZE; i++)
        {
            mypostinglist.set(ukey, (char*)&i);
        }
    }
//    printf("----------\n");
    for (uint32_t ukey=0; ukey<KEYSIZE; ukey++)
    {
        int32_t rnum = mypostinglist.get(ukey, (char*)ubuff, VLUSIZE * 2 * sizeof(uint32_t));
//        printf("rnum: %u\n", rnum);
        for (uint32_t i=0; i<VLUSIZE; i++)
        {
            assert(ubuff[i] == (VLUSIZE - 1 - i));
        }
    }
    free(ubuff);

//    const uint32_t ukey = 0;
//    uint32_t* ubuff = (uint32_t*)malloc(SIZE* 2 * sizeof(uint32_t));
//    for (uint32_t i=0; i<SIZE; i++)
//    {
//        mypostinglist.set(ukey, (char*)&i);
//    }
//    printf("----------\n");
//    int32_t rnum = mypostinglist.get(ukey, (char*)ubuff, SIZE * 2 * sizeof(uint32_t));
//    printf("rnum: %u\n", rnum);
//    for (uint32_t i=0; i<SIZE; i++)
//    {
//        assert(ubuff[i] == (SIZE - 1 - i));
//    }
//    free(ubuff);
    return 0;
}
