#include "algo.h"
#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <iostream>
#include <fstream>
using namespace std;

struct posting
{
    uint32_t doc_id   : 28;
    uint32_t intitle  :  1;
    uint32_t reserved :  3;
    uint32_t idf      :  8;
    uint32_t tf       :  8;
    uint32_t unused   : 16;
};

struct attribute
{
    uint32_t doc_id      : 28;
    uint32_t qingxidu    :  3;
    uint32_t delete_flag :  1;
    uint32_t publish_time;
    uint32_t update_time;
    uint32_t siteno      : 20;
    uint32_t type        :  6;
    uint32_t is_movie    :  3;
    uint32_t movie_sub   :  3;
    uint32_t is_tv_serial:  2;
    uint32_t serial_sub  :  8;
    uint32_t duration    : 10;
    uint32_t weight      : 12;
};
int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("%s ALL|BIGGER|SMALLER|ZONE|SET|EQUAL\n", argv[0]);
        exit(1);
    }
    struct timeval btv;
    struct timeval etv;
    struct timeval bbtv;
    struct timeval eetv;
    gettimeofday(&btv, NULL); 

    const uint32_t SIZE = 1000000;
    Json::Value root;
    Json::Reader reader;
    ifstream in("./conf/config.json");
    assert(reader.parse(in, root));

    structmask post_mask_map(root["POST"]);
    structmask attr_mask_map(root["ATTR"]);
    mask_item_t doc_id_mask;
    assert (0 == post_mask_map.get_mask_item("doc_id", &doc_id_mask));

    const uint32_t post_uint_count = post_mask_map.get_section_size();
    const uint32_t attr_uint_count = attr_mask_map.get_section_size();

    uint32_t* post_list_org = (uint32_t*)malloc(SIZE * post_uint_count * sizeof(uint32_t));
    uint32_t* attr_list_org = (uint32_t*)malloc(SIZE * attr_uint_count * sizeof(uint32_t));

    int32_t rst_num = 0;

    // set logic
    filter_logic_t mlogic;
    bool test_flag = false;
    uint32_t all_time_count = 0;

    // BIGGER
    if (0 == strcmp(argv[1], "BIGGER")||0 == strcmp(argv[1], "ALL"))
    {
        printf(".");
        test_flag = true;
        memset (post_list_org, 0, SIZE * post_uint_count * sizeof(uint32_t));
        memset (attr_list_org, 0, SIZE * attr_uint_count * sizeof(uint32_t));
        uint32_t* post_list = post_list_org;
        uint32_t* attr_list = attr_list_org;
        const char* strKey = "qingxidu";

        uint32_t ivalue = 2;
        mlogic.type   = BIGGER;
        mlogic.value  = ivalue;
        assert (0 == attr_mask_map.get_mask_item(strKey, &(mlogic.key_mask)));
        uint32_t mask = 0x03;
        // set the lists
        for (uint32_t i=0; i<SIZE; i++)
        {
            _SET_SOLO_VALUE_(post_list, doc_id_mask, i);
            _SET_SOLO_VALUE_(attr_list, mlogic.key_mask, i & mask);

            //            printf("%u : %u : %u\n", pattr[i].qingxidu, i & mask, ivalue);
            post_list += post_uint_count;
            attr_list += attr_uint_count;
        }

        gettimeofday(&bbtv, NULL); 
        rst_num = filter( post_list_org, SIZE, doc_id_mask,
                attr_list_org, &mlogic, 1 );
        gettimeofday(&eetv, NULL); 
        all_time_count += (eetv.tv_sec - bbtv.tv_sec)*1000000+(eetv.tv_usec - bbtv.tv_usec);
        //        printf("rst_num: %d\n", rst_num);
        post_list = post_list_org;
        for (int32_t i=0; i<rst_num; i++)
        {
            uint32_t id = _GET_SOLO_VALUE_(post_list, doc_id_mask);
            uint32_t vv = _GET_LIST_VALUE_(attr_list_org, id, mlogic.key_mask);
            assert (vv >= ivalue);
            post_list += post_uint_count;
        }
    }

    // SMALLER
    if (0 == strcmp(argv[1], "SMALLER")||0 == strcmp(argv[1], "ALL"))
    {
        printf(".");
        test_flag = true;
        memset (post_list_org, 0, SIZE * post_uint_count * sizeof(uint32_t));
        memset (attr_list_org, 0, SIZE * attr_uint_count * sizeof(uint32_t));
        uint32_t* post_list = post_list_org;
        uint32_t* attr_list = attr_list_org;
        const char* strKey = "qingxidu";

        uint32_t ivalue = 1;
        mlogic.type   = SMALLER;
        mlogic.value  = ivalue;

        assert (0 == attr_mask_map.get_mask_item(strKey, &(mlogic.key_mask)));

        uint32_t mask = 0x03;
        // set the lists
        for (uint32_t i=0; i<SIZE; i++)
        {
            _SET_SOLO_VALUE_(post_list, doc_id_mask, i);
            _SET_SOLO_VALUE_(attr_list, mlogic.key_mask, i & mask);
            //            printf("o[%u] : s[%u]\n", i & mask, ivalue);
            post_list += post_uint_count;
            attr_list += attr_uint_count;
        }

        gettimeofday(&bbtv, NULL); 
        rst_num = filter( post_list_org, SIZE, doc_id_mask,
                attr_list_org, &mlogic, 1 );
        gettimeofday(&eetv, NULL); 
        all_time_count += (eetv.tv_sec - bbtv.tv_sec)*1000000+(eetv.tv_usec - bbtv.tv_usec);
        //        printf("rst_num: %d\n", rst_num);
        post_list = post_list_org;
        for (int32_t i=0; i<rst_num; i++)
        {
            uint32_t id = _GET_SOLO_VALUE_(post_list, doc_id_mask);
            uint32_t vv = _GET_LIST_VALUE_(attr_list_org, id, mlogic.key_mask);
            assert (vv <= ivalue);
            post_list += post_uint_count;
        }
    }

    // ZONE
    if (0 == strcmp(argv[1], "ZONE")||0 == strcmp(argv[1], "ALL"))
    {
        printf(".");
        test_flag = true;
        memset (post_list_org, 0, SIZE * post_uint_count * sizeof(uint32_t));
        memset (attr_list_org, 0, SIZE * attr_uint_count * sizeof(uint32_t));
        uint32_t* post_list = post_list_org;
        uint32_t* attr_list = attr_list_org;
        const char* strKey = "duration";

        uint32_t min_value = 3;
        uint32_t max_value = 6;
        mlogic.type   = ZONE;
        mlogic.min_value  = min_value;
        mlogic.max_value  = max_value;

        assert (0 == attr_mask_map.get_mask_item(strKey, &(mlogic.key_mask)));

        uint32_t mask = 0x07;
        // set the lists
        for (uint32_t i=0; i<SIZE; i++)
        {
            _SET_SOLO_VALUE_(post_list, doc_id_mask, i);
            _SET_SOLO_VALUE_(attr_list, mlogic.key_mask, i & mask);
            //            printf("o[%u] : m[%u] M[%u]\n", i & mask, min_value, max_value);
            post_list += post_uint_count;
            attr_list += attr_uint_count;
        }

        gettimeofday(&bbtv, NULL); 
        rst_num = filter( post_list_org, SIZE, doc_id_mask,
                attr_list_org, &mlogic, 1 );
        gettimeofday(&eetv, NULL); 
        all_time_count += (eetv.tv_sec - bbtv.tv_sec)*1000000+(eetv.tv_usec - bbtv.tv_usec);
        //        printf("rst_num: %d\n", rst_num);
        post_list = post_list_org;
        for (int32_t i=0; i<rst_num; i++)
        {
            uint32_t id = _GET_SOLO_VALUE_(post_list, doc_id_mask);
            uint32_t vv = _GET_LIST_VALUE_(attr_list_org, id, mlogic.key_mask);
            //            printf("v[%u] : m[%u] M[%u]\n", vv, min_value, max_value);
            assert (min_value <= vv  && vv <= max_value);
            post_list += post_uint_count;
        }
    }

    // EQUAL
    if (0 == strcmp(argv[1], "EQUAL")||0 == strcmp(argv[1], "ALL"))
    {
        printf(".");
        test_flag = true;
        memset (post_list_org, 0, SIZE * post_uint_count * sizeof(uint32_t));
        memset (attr_list_org, 0, SIZE * attr_uint_count * sizeof(uint32_t));
        uint32_t* post_list = post_list_org;
        uint32_t* attr_list = attr_list_org;
        const char* strKey = "qingxidu";

        uint32_t ivalue = 2;
        mlogic.type   = EQUAL;
        mlogic.value  = ivalue;

        assert (0 == attr_mask_map.get_mask_item(strKey, &(mlogic.key_mask)));

        uint32_t mask = 0x03;
        // set the lists
        for (uint32_t i=0; i<SIZE; i++)
        {
            _SET_SOLO_VALUE_(post_list, doc_id_mask, i);
            _SET_SOLO_VALUE_(attr_list, mlogic.key_mask, i & mask);
            //            printf("o[%u] : s[%u]\n", i & mask, ivalue);
            post_list += post_uint_count;
            attr_list += attr_uint_count;
        }

        gettimeofday(&bbtv, NULL); 
        rst_num = filter( post_list_org, SIZE, doc_id_mask,
                attr_list_org, &mlogic, 1 );
        gettimeofday(&eetv, NULL); 
        all_time_count += (eetv.tv_sec - bbtv.tv_sec)*1000000+(eetv.tv_usec - bbtv.tv_usec);
        //        printf("rst_num: %d\n", rst_num);
        post_list = post_list_org;
        for (int32_t i=0; i<rst_num; i++)
        {
            uint32_t id = _GET_SOLO_VALUE_(post_list, doc_id_mask);
            uint32_t vv = _GET_LIST_VALUE_(attr_list_org, id, mlogic.key_mask);
            assert (vv == ivalue);
            post_list += post_uint_count;
        }
    }

    // SET
    if (0 == strcmp(argv[1], "SET")||0 == strcmp(argv[1], "ALL"))
    {
        printf(".");
        test_flag = true;
        memset (post_list_org, 0, SIZE * post_uint_count * sizeof(uint32_t));
        memset (attr_list_org, 0, SIZE * attr_uint_count * sizeof(uint32_t));
        uint32_t* post_list = post_list_org;
        uint32_t* attr_list = attr_list_org;
        const char* strKey = "type";

        mlogic.type       = SET;
        uint32_t set_count  = 4;
        for (uint32_t i=0; i<set_count; i++)
        {
            mlogic.vset.insert(i);
        }

        assert (0 == attr_mask_map.get_mask_item(strKey, &(mlogic.key_mask)));

        uint32_t mask = 0x07;
        // set the lists
        for (uint32_t i=0; i<SIZE; i++)
        {
            _SET_SOLO_VALUE_(post_list, doc_id_mask, i);
            _SET_SOLO_VALUE_(attr_list, mlogic.key_mask, i & mask);
            //            printf("o[%u] m[%u]\n", i & mask, mask/2);
            post_list += post_uint_count;
            attr_list += attr_uint_count;
        }

        gettimeofday(&bbtv, NULL); 
        rst_num = filter( post_list_org, SIZE, doc_id_mask,
                attr_list_org, &mlogic, 1 );
        gettimeofday(&eetv, NULL); 
        all_time_count += (eetv.tv_sec - bbtv.tv_sec)*1000000+(eetv.tv_usec - bbtv.tv_usec);
        //        printf("rst_num: %d\n", rst_num);
        post_list = post_list_org;
        for (int32_t i=0; i<rst_num; i++)
        {
            uint32_t id = _GET_SOLO_VALUE_(post_list, doc_id_mask);
            uint32_t vv = _GET_LIST_VALUE_(attr_list_org, id, mlogic.key_mask);
            //            printf("o[%u] m[%u] --\n", vv, mask/2);
            assert (vv <= mask/2);
            post_list += post_uint_count;
        }
    }

    // MIX
    if (0 == strcmp(argv[1], "MIX"))
    {
        printf(".");
        //        test_flag = true;
        memset (post_list_org, 0, SIZE * post_uint_count * sizeof(uint32_t));
        memset (attr_list_org, 0, SIZE * attr_uint_count * sizeof(uint32_t));
        uint32_t* post_list = post_list_org;
        uint32_t* attr_list = attr_list_org;
        const char* strBiggerKey_0  = "qingxidu";
        const char* strSmallerKey_1 = "is_movie";
        const char* strZoneKey_2    = "duration";
        const char* strEqualKey_3   = "delete_flag";
        const char* strSetKey_4     = "movie_sub";

        filter_logic_t logic_list[5];
        // SET BIGGER
        uint32_t big_ivalue = 3;
        uint32_t big_mask = 0x03;
        logic_list[0].type   = BIGGER;
        logic_list[0].value  = big_ivalue;
        assert (0 == attr_mask_map.get_mask_item(strBiggerKey_0, &(logic_list[0].key_mask)));

        // SET SMALLER
        uint32_t small_ivalue = 3;
        uint32_t small_mask = 0x03;
        logic_list[1].type   = SMALLER;
        logic_list[1].value  = small_ivalue;
        assert (0 == attr_mask_map.get_mask_item(strSmallerKey_1, &(logic_list[1].key_mask)));

        // SET ZONE
        uint32_t min_value = 3;
        uint32_t max_value = 6;
        uint32_t zone_mask = 0x07;
        logic_list[2].type   = ZONE;
        logic_list[2].min_value  = min_value;
        logic_list[2].max_value  = max_value;
        assert (0 == attr_mask_map.get_mask_item(strZoneKey_2, &(logic_list[2].key_mask)));

        // SET EQUAL
        uint32_t equal_ivalue = 1;
        uint32_t equal_mask   = 0x01;
        logic_list[3].type    = EQUAL;
        logic_list[3].value   = equal_ivalue;
        assert (0 == attr_mask_map.get_mask_item(strEqualKey_3, &(logic_list[3].key_mask)));

        // SET SET
        logic_list[4].type = SET;
        uint32_t set_count = 4;
        uint32_t set_mask  = 0x07;
        for (uint32_t i=0; i<set_count; i++)
        {
            logic_list[4].vset.insert(i);
        }
        assert (0 == attr_mask_map.get_mask_item(strSetKey_4, &(logic_list[4].key_mask)));

        attribute* pattr = (attribute*)attr_list_org;
        // set the lists
        for (uint32_t i=0; i<SIZE; i++)
        {
            _SET_SOLO_VALUE_(post_list, doc_id_mask, i);

            _SET_SOLO_VALUE_(attr_list, logic_list[0].key_mask, i & big_mask);
            _SET_SOLO_VALUE_(attr_list, logic_list[1].key_mask, i & small_mask);
            _SET_SOLO_VALUE_(attr_list, logic_list[2].key_mask, i & zone_mask);
            _SET_SOLO_VALUE_(attr_list, logic_list[3].key_mask, i & equal_mask);
            _SET_SOLO_VALUE_(attr_list, logic_list[4].key_mask, i & set_mask);
            //            printf("[%u : %u] [%u : %u] [%u : %u] [%u : %u] [%u : %u]\n",
            //                    pattr[i].qingxidu,    i & big_mask,
            //                    pattr[i].is_movie,    i & small_mask,
            //                    pattr[i].duration,    i & zone_mask,
            //                    pattr[i].delete_flag, i & equal_mask,
            //                    pattr[i].movie_sub,   i & set_mask
            //                    );
            post_list += post_uint_count;
            attr_list += attr_uint_count;
        }
        uint32_t mcount = 0;
        for (uint32_t i=0; i<SIZE; i++)
        {
            if (pattr[i].qingxidu < big_ivalue)
            {
                continue;
            }
            if (pattr[i].is_movie > small_ivalue)
            {
                continue;
            }
            if (pattr[i].duration < min_value || pattr[i].duration > max_value)
            {
                continue;
            }
            if (pattr[i].delete_flag != equal_ivalue )
            {
                continue;
            }
            if (logic_list[4].vset.end() == logic_list[4].vset.find(pattr[i].movie_sub))
            {
                continue;
            }
            mcount++;
        }

        //        printf("mcount : %d\n", mcount);

        gettimeofday(&bbtv, NULL); 
        rst_num = filter( post_list_org, SIZE, doc_id_mask,
                attr_list_org, logic_list, 5 );
        gettimeofday(&eetv, NULL); 
        //        printf("rst_num: %d\n", rst_num);

        printf("  OK!\n");
        printf ("list-size: %u flt-time-consumed: %lu us\n", SIZE,
                (eetv.tv_sec - bbtv.tv_sec)*1000000+(eetv.tv_usec - bbtv.tv_usec));
        post_list = post_list_org;
        for (int32_t i=0; i<rst_num; i++)
        {
            uint32_t id = _GET_SOLO_VALUE_(post_list, doc_id_mask);
            // uint32_t vv = _GET_LIST_VALUE_(attr_list_org, id, attr_uint_count,
            // mlogic.key_mask.uint_offset, mlogic.key_mask.item_mask, mlogic.key_mask.move_count);
            // printf("o[%u] m[%u] --\n", vv, mask/2);
            // assert (vv <= mask/2);
            post_list += post_uint_count;
            assert (!(pattr[id].qingxidu < big_ivalue));
            assert (!(pattr[id].is_movie > small_ivalue));
            assert (!(pattr[id].duration < min_value || pattr[id].duration > max_value));
            assert (!(pattr[id].delete_flag != equal_ivalue ));
            assert (!(logic_list[4].vset.end() == logic_list[4].vset.find(pattr[id].movie_sub)));
        }
    }

    gettimeofday(&etv, NULL); 
    if (test_flag)
    {
        printf("  OK!\n");
        printf ("list-size: %u all-time-consumed: %u us\n", SIZE, all_time_count);
    }
    //    printf ("list-size: %u time-consumed: %lu us\n", SIZE,
    //            (etv.tv_sec - btv.tv_sec)*1000000+(etv.tv_usec - btv.tv_usec));

    free(post_list_org);
    free(attr_list_org);

    structmask merge_mask_map(root["MERGE"]);
    const uint32_t post_uint_count_new = merge_mask_map.get_section_size();

    uint32_t* post_list_org_0 = (uint32_t*)malloc(SIZE * post_uint_count_new * sizeof(uint32_t));
    uint32_t* post_list_org_1 = (uint32_t*)malloc(SIZE * post_uint_count_new * sizeof(uint32_t));
    result_pair_t* result_pair_lst = (result_pair_t*)malloc(SIZE * sizeof(result_pair_t));
    uint32_t* post_list_0 = post_list_org_0;
    uint32_t* post_list_1 = post_list_org_1;
    mask_item_t weight_mask;

    assert (0 == merge_mask_map.get_mask_item("doc_id", &doc_id_mask));
    assert (0 == merge_mask_map.get_mask_item("weight", &weight_mask));

    for (uint32_t i=0; i<SIZE; i+=2)
    {
        _SET_SOLO_VALUE_(post_list_0, doc_id_mask, SIZE-i);
        _SET_SOLO_VALUE_(post_list_0, weight_mask, 1);
        post_list_0 += post_uint_count_new;
    }

    for (uint32_t i=0; i<SIZE; i+=3)
    {
        _SET_SOLO_VALUE_(post_list_1, doc_id_mask, SIZE-i);
        _SET_SOLO_VALUE_(post_list_1, weight_mask, 1);
        post_list_1 += post_uint_count_new;
    }

    terminfo_t termlist[2];
    termlist[0].posting_list = post_list_org_0;
    termlist[1].posting_list = post_list_org_1;
    termlist[0].list_size = SIZE/2;
    termlist[1].list_size = SIZE/3;
    termlist[0].weight = 1;
    termlist[1].weight = 1;

    int32_t merge_rst_num = weight_merge(termlist, 2, doc_id_mask, weight_mask,
            result_pair_lst, SIZE);
    printf("merge_rst_num: %d\n", merge_rst_num);
    //    for (uint32_t i=0; i<merge_rst_num; i++)
    //    {
    //        printf("%8u %u\n", result_pair_lst[i].id, result_pair_lst[i].weight);
    //    }

    // MERGE TEST
    free(post_list_org_0);
    free(post_list_org_1);
    free(result_pair_lst);

    return 0;
}
