#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <time.h>
#include <sys/time.h>
#include "mem_indexer.h"

// 省去格外headlist的代价是，有好几秒无法服务。。
// ./test 16000000 4
// termCount[16000000] set_time-consumed[7673880]us
// termCount[16000000] get_time-consumed[3756470]us
// termCount[16000000] srt_time-consumed[6508016]us
// termCount[16000000] get_time-consumed[6124646]us
// termCount[16000000] itg_time-consumed[6297213]us

int main(const int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test KEYSIZE\n");
        return 1;
    }
    const uint32_t KEYSIZE = atoi(argv[1]);
    const uint32_t cell_size     = sizeof(uint32_t);
    const uint32_t bucket_size   = 20;
    const uint32_t headlist_size = 1<<23;
    uint32_t blocknum_list[1];
    blocknum_list[0] = 20000000;
    mem_indexer mi(cell_size, bucket_size, headlist_size, blocknum_list, sizeof(blocknum_list)/sizeof(blocknum_list[0]));

    struct   timeval btv;
    struct   timeval etv;
    gettimeofday(&btv, NULL);
    for (uint32_t i=0; i<KEYSIZE; i++)
    {
        char c = 'A';
        for (uint32_t k=0; k<20; k++)
        {
            char str[20];
            snprintf(str, sizeof(str), "%c[%u]", c+k, i);
            if (postinglist :: OK != mi.set_posting_list(str, &i))
            {
                gettimeofday(&etv, NULL);
                ROUTN ("termCount[%u] set_time-consumed[%u]us", i*20,
                        (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));
                return 0;
            }
        }
    }

    return 0;
}
