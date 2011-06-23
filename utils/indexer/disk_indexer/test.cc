#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "base_indexer.h"
#include "disk_indexer.h"
#include <vector>
#include <assert.h>
#include <algorithm>
using namespace std;

int main()
{
    const uint32_t SIZE = 10000;
    uint32_t* read_buff = (uint32_t*)malloc(SIZE * sizeof(uint32_t));
    assert(read_buff != NULL);

    // WRITE AND READ
    disk_indexer di("./data/", "test");
    printf("NO_SUCH_TERM ret[%d]\n", di.get_posting_list("NO_SUCH_TERM", read_buff, SIZE * sizeof(uint32_t)));
    di.clear();

    for (uint32_t i=0; i<SIZE; i++)
    {
        read_buff[i] = i;
    }
    
    for (uint32_t i=0; i<SIZE; i++)
    {
        ikey_t ikey;
        ikey.sign64 = i;
        di.set_posting_list(i, ikey, read_buff, i*sizeof(uint32_t));
    }
    memset(read_buff, 0, SIZE * sizeof(uint32_t));
    di.begin();
    uint64_t it = 0;
    while(!di.is_end())
    {
        int retnum = di.itget(it, read_buff, sizeof(uint32_t)*SIZE);
        assert((uint64_t)retnum == sizeof(uint32_t)*(it));
        for (uint32_t k=0; k<it; k++)
        {
            assert(read_buff[k] == k);
        }
        di.next();
        it++;
    }
    di.set_finish();

    // fileblock跨文件
    di.clear();

    const uint32_t NUMBER = 10;
    const uint32_t MFILE_COUNT = 140000000;
    for (uint32_t i=0; i<SIZE; i++)
    {
        read_buff[i] = i;
    }
    
    for (uint32_t i=0; i<MFILE_COUNT; i++)
    {
        ikey_t ikey;
        ikey.sign64 = i;
        di.set_posting_list(i, ikey, read_buff, NUMBER*sizeof(uint32_t));
    }

    di.begin();
    it = 0;
    while(!di.is_end())
    {
        memset(read_buff, 0, NUMBER * sizeof(uint32_t));
        int retnum = di.itget(it, read_buff, sizeof(uint32_t)*SIZE);
        assert((uint64_t)retnum == sizeof(uint32_t)*(NUMBER));
        for (uint32_t k=0; k<NUMBER; k++)
        {
            assert(read_buff[k] == k);
        }
        di.next();
        it++;
    }
    di.set_finish();
    di.clear();

    free(read_buff);
    return 0;
}

//struct second_index_t
//{
//    uint32_t  milestone;
//    ikey_t    ikey;
//    bool operator < (const second_index_t& right)
//    {
//        return this->ikey.sign64 < right.ikey.sign64;
//    }
//};

//int main()
//{
//    vector<second_index_t> silist;
//    for (uint32_t i=1; i<1000; i+=10)
//    {
//        second_index_t si;
//        si.milestone = i*1000;
//        si.ikey.sign64 = i;
//        silist.push_back(si);
//    }
//
//    vector<second_index_t>::iterator bounds;
//    for (uint32_t i=1; i<1001; i+=10)
//    {
//        second_index_t si;
//        si.ikey.sign64 = i;
//        bounds = lower_bound (silist.begin(), silist.end(), si);
//        printf("key[%u] milestone: [%u] value[%llu] ", i, bounds->milestone, bounds->ikey.sign64);
//        printf("bool[%u:%u]\n", bounds == silist.begin(), bounds == silist.end());
//    }
//
//    for (uint32_t i=1; i<1001; i+=9)
//    {
//        second_index_t si;
//        si.ikey.sign64 = i;
//        bounds = lower_bound (silist.begin(), silist.end(), si);
//        printf("key[%u] milestone: [%u] value[%llu] ", i, bounds->milestone, bounds->ikey.sign64);
//        printf("bool[%u:%u]\n", bounds == silist.begin(), bounds == silist.end());
//    }
//    return 0;
//////////////////////////////}

