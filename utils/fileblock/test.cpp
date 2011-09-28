#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "fileblock.h"

struct diskv_idx_t
{
    uint32_t file_no  :  7;   ///< 1<<7 = 128 files
    uint32_t data_len : 25;   ///< max doc size is 1<<25 = 32M
    uint32_t offset   : 31;   ///< max file size is 2G
    uint32_t reserved :  1;   ///< reserved bit
};

struct fb_index_t
{
    uint64_t    key;
    diskv_idx_t idx;
};

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        exit(1);
    }
    //    const uint32_t SIZE = atoi(argv[1]);
    const char* filedir = argv[1];
    fileblock myfl(filedir, "index", sizeof(fb_index_t));
    //    myfl.begin();
    //    for (uint32_t i=0; i<SIZE; i++)
    //    {
    //        myfl.write_next((char*)&i);
    //    }
    myfl.begin();
    uint32_t count = 0;
    while(!myfl.is_end())
    {
        uint32_t v = 0;
        fb_index_t fi;
        int x = myfl.itget((char*)&fi, sizeof(fb_index_t));
        assert(x == sizeof(fb_index_t));
        printf("%08u - [f:%u] [o:%10u] [l:%4u] [k:%llu]\n", count++, fi.idx.file_no, fi.idx.offset, fi.idx.data_len, fi.key);
        myfl.next();
    }
    //
    //    const uint32_t milestone = 1500;
    //    uint32_t ulist[milestone];
    //    const uint32_t loop = SIZE/milestone;
    //    for (uint32_t i=0; i<loop; i++)
    //    {
    //        assert (milestone * sizeof(uint32_t) == myfl.get(i*milestone, milestone, &ulist, sizeof(ulist)));
    //        for (uint32_t j=0; j<milestone; j++)
    //        {
    //            assert(ulist[j] == i*milestone + j);
    //        }
    //    }
    //    assert ((SIZE%milestone) * sizeof(uint32_t) == myfl.get(loop*milestone, milestone, &ulist, sizeof(ulist)));
    //    for (uint32_t j=0; j<SIZE%milestone; j++)
    //    {
    //        assert(ulist[j] == loop*milestone + j);
    //    }
    return 0;
}
