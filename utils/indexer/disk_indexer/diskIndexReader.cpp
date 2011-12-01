#include "structmask.h"
#include "disk_indexer.h"
#include "json/json.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/types.h>
#include "mylog.h"
#include "MyException.h"

int main(int argc, char** argv) try
{
    if (argc != 4)
    {
        printf ("./diskIndexReader dir filename term\n");
        exit(1);
    }

    const char* pdir  = argv[1];
    const char* pfile = argv[2];
    const char* pterm = argv[3];

    // read json config
    Json::Value root;
    Json::Reader reader;
    ifstream in("./conf/test.conf");
    if (in.is_open())
    {
        FATAL("config[%s] open FAIL.", "./conf/test.conf");
        MySuicideAssert(0);
    }
    if (! reader.parse(in, root))
    {
        FATAL("json format error.");
        MySuicideAssert(0);
    }

    Json::Value field_array = root["posting_list_cell"];
    structmask mymask(field_array);
    printf ("uint32_t size[%u]\n", mymask.get_section_size());
    printf ("segment  size[%u]\n", mymask.get_segment_size());

    // 初始化 disk_indexer
    const uint32_t post_cell_size = (uint32_t)(sizeof(uint32_t)*mymask.get_section_size());
    disk_indexer mydi(pdir, pfile, post_cell_size);

    char key[128];
    mask_item_t key_mask;
    for(mymask.begin(); !mymask.is_end(); mymask.next())
    {
        assert(mymask.itget(key, sizeof(key), &key_mask));
        printf("key[%s] uint_off[%u] item_mask[0x%08x] move_count[%02u] uint32_count[%02u]\n",
                key,
                key_mask.uint_offset, 
                key_mask.item_mask,
                key_mask.move_count,
                key_mask.uint32_count);
    }
    printf("--------it end---------\n");

    const uint32_t buffsize = 10000000*post_cell_size;
    char* buff = (char*)malloc(buffsize);
    MySuicideAssert(buff != NULL);

    int ret = mydi.get_posting_list(pterm, buff, buffsize);

    for (int i=0; i<ret; i++)
    {
        for(mymask.begin(); !mymask.is_end(); mymask.next())
        {
            assert(mymask.itget(key, sizeof(key), &key_mask));
            printf("%16s : %u\n", key, _GET_LIST_VALUE_(buff, i, key_mask));
        }
        printf("-------------------------------------\n");
    }

    free(buff);

    return 0;
}
catch(...)
{
    fprintf(stderr, "exception caught.\n");
    exit(1);
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
