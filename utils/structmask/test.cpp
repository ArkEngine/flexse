#include "structmask.h"
#include "json/json.h"
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
#include "mylog.h"
#include "MyException.h"

struct test
{
    uint32_t id0 : 25;
    uint32_t id1 :  7;
    uint32_t id2 : 17;
    uint32_t id3 :  9;
    uint32_t id4 :  5;
    uint32_t id5 :  1;
};

int main(int argc, char** argv) try
{
    if (argc != 2)
    {
        printf ("./test SIZE\n");
        exit(1);
    }

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

    mask_item_t* mask_item = (mask_item_t*)malloc(mymask.get_segment_size()*sizeof(mask_item_t));
    const char* keylist[] = {"id0", "id1", "id2", "id3", "id4", "id5"};
    for (uint32_t i=0; i<mymask.get_segment_size(); i++)
    {
        if (-1 == mymask.get_mask_item(keylist[i], &mask_item[i]))
        {
            printf ("NO KEY[%s] FOUND\n", keylist[i]);
        }
        else
        {
            printf("key[%s] uint_off[%u] item_mask[0x%08x] move_count[%02u] uint32_count[%02u]\n",
                    keylist[i], mask_item[i].uint_offset, 
                    mask_item[i].item_mask, mask_item[i].move_count,
                    mask_item[i].uint32_count);
        }
    }

    const uint32_t SIZE = atoi(argv[1]);
    struct test* ptest = (struct test*)malloc(SIZE*sizeof(struct test));
    for (uint32_t i=0; i<SIZE; i++)
    {
        ptest[i].id0 = i;//& mask_item[0].item_mask;
        ptest[i].id1 = i;//& mask_item[1].item_mask; 
        ptest[i].id2 = i;//& mask_item[2].item_mask; 
        ptest[i].id3 = i;//& mask_item[3].item_mask; 
        ptest[i].id4 = i;//& mask_item[4].item_mask; 
        ptest[i].id5 = i;//& mask_item[5].item_mask; 
    }
    struct timeval btv;
    struct timeval etv;

    gettimeofday(&btv, NULL);
    for (uint32_t i=0; i<SIZE; i++)
    {
        uint32_t k = 0;
        k += ptest[i].id0;
        k += ptest[i].id1;
        k += ptest[i].id2;
        k += ptest[i].id3;
        k += ptest[i].id4;
        k += ptest[i].id5;
        assert (k == 0 || k);
    }
    gettimeofday(&etv, NULL);
    printf ("lcount: %u org-time-consumed: %lu us\n", SIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));



    gettimeofday(&btv, NULL);
    uint32_t* puint = (uint32_t*) ptest;
    const uint32_t uint_count = mymask.get_section_size();
    for (uint32_t i=0; i<SIZE; i++)
    {
        //        assert(ptest[i].id0 == ((puint[mask_item[0].uint_offset] & mask_item[0].item_mask) >> mask_item[0].move_count));
        //        assert(ptest[i].id0 == _GET_VALUE_(puint, mask_item[0]));
        //        assert(ptest[i].id1 == _GET_VALUE_(puint, mask_item[1]));
        //        assert(ptest[i].id2 == _GET_VALUE_(puint, mask_item[2]));
        //        assert(ptest[i].id3 == _GET_VALUE_(puint, mask_item[3]));
        //        assert(ptest[i].id4 == _GET_VALUE_(puint, mask_item[4]));
        //        assert(ptest[i].id5 == _GET_VALUE_(puint, mask_item[5]));
        uint32_t k = 0;
        k += _GET_SOLO_VALUE_(puint, mask_item[0]);
        k += _GET_SOLO_VALUE_(puint, mask_item[1]);
        k += _GET_SOLO_VALUE_(puint, mask_item[2]);
        k += _GET_SOLO_VALUE_(puint, mask_item[3]);
        k += _GET_SOLO_VALUE_(puint, mask_item[4]);
        k += _GET_SOLO_VALUE_(puint, mask_item[5]);
        assert (k == 0 || k);
        puint += uint_count;;
    }
    gettimeofday(&etv, NULL);
    printf ("lcount: %u get-time-consumed: %lu us\n", SIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    gettimeofday(&btv, NULL);
    puint = (uint32_t*) ptest;
    for (uint32_t i=0; i<SIZE; i++)
    {
        _SET_SOLO_VALUE_(puint, mask_item[0], i);
        _SET_SOLO_VALUE_(puint, mask_item[1], i);
        _SET_SOLO_VALUE_(puint, mask_item[2], i);
        _SET_SOLO_VALUE_(puint, mask_item[3], i);
        _SET_SOLO_VALUE_(puint, mask_item[4], i);
        _SET_SOLO_VALUE_(puint, mask_item[5], i);
        //        assert(ptest[i].id0 == _GET_VALUE_(puint, mask_item[0]));
        //        assert(ptest[i].id0 == i);
        puint += uint_count;;
    }
    gettimeofday(&etv, NULL);
    printf ("lcount: %u set-time-consumed: %lu us\n", SIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    gettimeofday(&btv, NULL);
    for (uint32_t i=0; i<SIZE; i++)
    {
        _SET_LIST_VALUE_(ptest, i, mask_item[0], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[1], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[2], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[3], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[4], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[5], i);
    }
    gettimeofday(&etv, NULL);
    printf ("lcount: %u set-time-consumed: %lu us\n", SIZE,
            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    for (uint32_t i=0; i<SIZE; i++)
    {
        _SET_LIST_VALUE_(ptest, i, mask_item[0], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[1], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[2], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[3], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[4], i);
        _SET_LIST_VALUE_(ptest, i, mask_item[5], i);
        assert (ptest[i].id0 == _GET_LIST_VALUE_(ptest, i, mask_item[0]));
    }



    free(ptest);

    return 0;
}
catch(...)
{
    fprintf(stderr, "exception caught.\n");
    exit(1);
}

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
