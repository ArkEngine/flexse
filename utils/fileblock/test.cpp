#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "fileblock.h"
#include "mylog.h"
#include "MyException.h"

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
    const uint32_t SIZE = atoi(argv[1]);
    const char* filedir = "./data/";
    fileblock* myfl = new fileblock(filedir, "test", sizeof(uint32_t));
    // -1- 清空数据
    myfl->clear();
    // -2- 设置数据
    for (uint32_t i=0; i<SIZE; i++)
    {
        MySuicideAssert( 0 == myfl->set(i, &i));
    }
    // -3- 读取数据
    for (uint32_t i=0; i<SIZE; i++)
    {
        uint32_t ii = 0;
        MySuicideAssert( sizeof(uint32_t) == myfl->get(i, &ii, sizeof(uint32_t)));
        MySuicideAssert( ii == i);
    }
    // -4- 批量读取数据(跨块读取)
    const uint32_t milestone = 1500;
    uint32_t ulist[milestone];
    const uint32_t loop = SIZE/milestone;
    for (uint32_t i=0; i<loop; i++)
    {
        assert (milestone * sizeof(uint32_t) == myfl->get(i*milestone, milestone, &ulist, sizeof(ulist)));
        for (uint32_t j=0; j<milestone; j++)
        {
            assert(ulist[j] == i*milestone + j);
        }
    }
    if (SIZE % milestone)
    {
        // 最后一段
        //        printf("b: %u, c: %u\n", loop*milestone, milestone);
        //        printf("%u == %u\n",
        //        (SIZE%milestone) * sizeof(uint32_t), myfl->get(loop*milestone, milestone, &ulist, sizeof(ulist)));
        assert ((SIZE%milestone) * sizeof(uint32_t) == myfl->get(loop*milestone, milestone, &ulist, sizeof(ulist)));
        for (uint32_t j=0; j<SIZE%milestone; j++)
        {
            assert(ulist[j] == loop*milestone + j);
        }
    }

    // -5- 清空数据
    myfl->clear();
    // -6- 批量设置数据
    myfl->clear();
    myfl->begin();
    for (uint32_t i=0; i<SIZE; i++)
    {
        myfl->set_and_next((char*)&i);
    }
    // -7- 读取数据
    myfl->begin();
    uint32_t cc = 0;
    while(!myfl->is_end())
    {
        uint32_t ii = 0;
        myfl->itget((char*)&ii, sizeof(uint32_t));
        myfl->next();
        MySuicideAssert(cc == ii);
        cc++;
    }
    // -8- 批量读取数据(整块读取)
    for (uint32_t i=0; i<loop; i++)
    {
        assert (milestone * sizeof(uint32_t) == myfl->get(i*milestone, milestone, &ulist, sizeof(ulist)));
        for (uint32_t j=0; j<milestone; j++)
        {
            assert(ulist[j] == i*milestone + j);
        }
    }
    if (SIZE % milestone)
    {
        // 最后一段
        //        printf("b: %u, c: %u\n", loop*milestone, milestone);
        //        printf("%u == %u\n",
        //        (SIZE%milestone) * sizeof(uint32_t), myfl->get(loop*milestone, milestone, &ulist, sizeof(ulist)));
        assert ((SIZE%milestone) * sizeof(uint32_t) == myfl->get(loop*milestone, milestone, &ulist, sizeof(ulist)));
        for (uint32_t j=0; j<SIZE%milestone; j++)
        {
            assert(ulist[j] == loop*milestone + j);
        }
    }

    // 检查句柄是否泄漏
    printf("sleep 100s.\n");
    myfl->clear();
    sleep(10);
    printf("wake up.\n");

    // -9- 小数据测试
    const uint32_t TINYSIZE = 1;
    for (uint32_t i=0; i<TINYSIZE; i++)
    {
        MySuicideAssert( 0 == myfl->set(i, &i));
    }
    for (uint32_t i=0; i<TINYSIZE; i++)
    {
        uint32_t ii = 0;
        MySuicideAssert( sizeof(uint32_t) == myfl->get(i, &ii, sizeof(uint32_t)));
        MySuicideAssert( ii == i);
    }

    // -0- 异常测试
    printf("sleep 100s.\n");
    delete myfl;
    sleep(10);
    printf("wake up.\n");
    return 0;






















    //    myfl->begin();
    //    uint32_t count = 0;
    //    while(!myfl->is_end())
    //    {
    //        uint32_t v = 0;
    //        fb_index_t fi;
    //        int x = myfl->itget((char*)&fi, sizeof(fb_index_t));
    //        assert(x == sizeof(fb_index_t));
    //        printf("%08u - [f:%u] [o:%10u] [l:%4u] [k:%llu]\n",
    //                count++, fi.idx.file_no, fi.idx.offset, fi.idx.data_len, fi.key);
    //        myfl->next();
    //    }
    //    myfl->begin();
    //    uint32_t count = 0;
    //    while(!myfl->is_end())
    //    {
    //        uint32_t v = 0;
    //        fb_index_t fi;
    //        int x = myfl->itget((char*)&fi, sizeof(fb_index_t));
    //        assert(x == sizeof(fb_index_t));
    //        printf("%08u - [f:%u] [o:%10u] [l:%4u] [k:%llu]\n",
    //        count++, fi.idx.file_no, fi.idx.offset, fi.idx.data_len, fi.key);
    //        myfl->next();
    //    }
    //
}
