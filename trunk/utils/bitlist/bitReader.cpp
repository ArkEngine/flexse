#include "structmask.h"
#include "bitlist.h"
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
    if (argc != 5)
    {
        printf ("./bitReader dir filename beginid loopcount\n");
        exit(1);
    }

    const char* pdir = argv[1];
    const char* pfile = argv[2];
    uint32_t beginid = atoi(argv[3]);
    uint32_t lpcount = atoi(argv[4]);

    // read json config
    Json::Value root;
    Json::Reader reader;
    ifstream in("./conf/test.conf");
    if (! reader.parse(in, root))
    {
        FATAL("json format error.");
        MyToolThrow("json format error.");
    }

    Json::Value field_array = root["document_attribute"];
    structmask mymask(field_array);
    printf ("uint32_t size[%u]\n", mymask.get_section_size());
    printf ("segment  size[%u]\n", mymask.get_segment_size());

    char key[128];

    // getfilesize
    struct stat fs;
    char path[128];
    snprintf(path, sizeof(path), "%s/%s", pdir, pfile);
    MyThrowAssert( 0 == stat( path, &fs ) );
    MyThrowAssert( 0 == (uint32_t)fs.st_size % mymask.get_section_size());
    bitlist mybitlist(pdir, pfile, mymask.get_segment_size(), (uint32_t)(fs.st_size / mymask.get_section_size()));


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
    for (uint32_t i=0; i<lpcount; i++)
    {
        for(mymask.begin(); !mymask.is_end(); mymask.next())
        {
            assert(mymask.itget(key, sizeof(key), &key_mask));
            printf("%16s : %u\n", key, _GET_LIST_VALUE_(mybitlist.puint, (beginid+i), key_mask));
        }
        printf("-------------------------------------\n");
    }

    return 0;
}
catch(...)
{
    fprintf(stderr, "exception caught.\n");
    exit(1);
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
