#include <stdio.h>
#include <stdint.h>
#include "indexer.h"
#include <vector>
#include <algorithm>
using namespace std;

struct second_index_t
{
    uint32_t  milestone;
    ikey_t    ikey;
    bool operator < (const second_index_t& right)
    {
        return this->ikey.sign64 < right.ikey.sign64;
    }
};

int main()
{
    vector<second_index_t> silist;
    for (uint32_t i=1; i<1000; i+=10)
    {
        second_index_t si;
        si.milestone = i*1000;
        si.ikey.sign64 = i;
        silist.push_back(si);
    }

    vector<second_index_t>::iterator bounds;
    for (uint32_t i=1; i<1001; i+=10)
    {
        second_index_t si;
        si.ikey.sign64 = i;
        bounds = lower_bound (silist.begin(), silist.end(), si);
        printf("key[%u] milestone: [%u] value[%llu] ", i, bounds->milestone, bounds->ikey.sign64);
        printf("bool[%u:%u]\n", bounds == silist.begin(), bounds == silist.end());
    }

    for (uint32_t i=1; i<1001; i+=9)
    {
        second_index_t si;
        si.ikey.sign64 = i;
        bounds = lower_bound (silist.begin(), silist.end(), si);
        printf("key[%u] milestone: [%u] value[%llu] ", i, bounds->milestone, bounds->ikey.sign64);
        printf("bool[%u:%u]\n", bounds == silist.begin(), bounds == silist.end());
    }
    return 0;
}
