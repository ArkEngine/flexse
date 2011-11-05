#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "diskv.h"

int main()
{
    // 大数据量
    // 正确性
    // 异常接口
    printf("hello world\n");
    diskv dv("./data/", "test");
    diskv :: diskv_idx_t idx;
    const uint32_t SIZE = 16*1024*1024;
    char* str = (char*)malloc(SIZE);
    memset (str, 'A', SIZE);
    for (uint32_t i=0; i<100; i++)
    {
        dv.set(idx, str, SIZE);
    }
    printf("file_no[%u] offset[%u] data_len[%u]\n",
            idx.file_no, idx.offset, idx.data_len);
    dv.get(idx, str, SIZE);
    for (uint32_t i=0; i<SIZE; i++)
    {
        assert('A' == str[i]);
    }
    free(str);
    return 0;
}
