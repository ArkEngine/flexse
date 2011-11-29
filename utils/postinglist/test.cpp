#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include "postinglist.h"

// 省去格外headlist的代价是，有好几秒无法服务。。
// ./test 16000000 4
// termCount[16000000] set_time-consumed[7673880]us
// termCount[16000000] get_time-consumed[3756470]us
// termCount[16000000] srt_time-consumed[6508016]us
// termCount[16000000] get_time-consumed[6124646]us
// termCount[16000000] itg_time-consumed[6297213]us

int main(const int argc, char** argv)
{
    if (argc != 3)
    {
        printf("./test KEYSIZE VALUESIZE\n");
        return 1;
    }

    uint32_t vv[6];
    const uint32_t post_cell_size = 5;
    const uint32_t KEYSIZE = atoi(argv[1]);
    const uint32_t VLUSIZE = atoi(argv[2]);
    const uint32_t cell_size     = post_cell_size*sizeof(uint32_t);
    const uint32_t bucket_size   = 20;
    const uint32_t headlist_size = 0x1000000;
    uint32_t blocknum_list[8];
    for (uint32_t i=0; i<8; i++)
    {
        blocknum_list[i] = (i==0)? 1048576 : blocknum_list[i-1]/2;
    }
    postinglist mypostinglist(cell_size, bucket_size, headlist_size, blocknum_list, sizeof(blocknum_list)/sizeof(blocknum_list[0]));

    struct   timeval btv;
    struct   timeval etv;
    gettimeofday(&btv, NULL);

    uint32_t* ubuff = (uint32_t*)malloc(VLUSIZE * post_cell_size * sizeof(uint32_t));
    const uint32_t ubuffsize = (uint32_t)(VLUSIZE * post_cell_size * sizeof(uint32_t));
    for (uint32_t ukey=0; ukey<KEYSIZE; ukey+=2)
    {
        for (uint32_t i=0; i<VLUSIZE; i++)
        {
            vv[0] = i;
            mypostinglist.set(ukey, vv);
            //            int ret = mypostinglist.set(ukey, (char*)&i);
            //            printf("%d\n", ret);
        }
    }
    for (uint32_t ukey=1; ukey<KEYSIZE; ukey+=2)
    {
        for (uint32_t i=0; i<VLUSIZE; i++)
        {
            vv[0] = i;
            mypostinglist.set(ukey, vv);
            //            int ret = mypostinglist.set(ukey, (char*)&i);
            //            printf("%d\n", ret);
        }
    }

    gettimeofday(&etv, NULL);
    ROUTN ("termCount[%u] set_time-consumed[%u]us\n", KEYSIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    gettimeofday(&btv, NULL);
    for (uint32_t ukey=0; ukey<KEYSIZE; ukey++)
    {
        int32_t rnum = mypostinglist.get(ukey, (char*)ubuff, ubuffsize);
//        printf("%u - %u\n", rnum, VLUSIZE);
        assert ((uint32_t)rnum == VLUSIZE);
        for (uint32_t i=0; i<VLUSIZE; i++)
        {
            assert(ubuff[i*post_cell_size] == (VLUSIZE - 1 - i));
        }
    }
    gettimeofday(&etv, NULL);
    ROUTN ("termCount[%u] get_time-consumed[%u]us\n", KEYSIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    gettimeofday(&btv, NULL);
    mypostinglist.begin();
    uint32_t count = 0;
    while(!mypostinglist.is_end())
    {
        uint64_t ukey;
        int32_t rnum = mypostinglist.itget(ukey, (char*)ubuff, ubuffsize);
        mypostinglist.next();
        assert(ukey == count);
        count ++;
        assert ((uint32_t)rnum == VLUSIZE);
        for (uint32_t i=0; i<VLUSIZE; i++)
        {
            assert(ubuff[i*post_cell_size] == (VLUSIZE - 1 - i));
        }
    }
    gettimeofday(&etv, NULL);
    ROUTN ("termCount[%u] itg_time-consumed[%u]us\n", KEYSIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    free(ubuff);

    return 0;
}
