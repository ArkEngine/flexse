#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include "diskv.h"

int main()
{
    // 正确性
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
    // 大数据量，当前文件写不下的情况
    // 异常接口
    // 句柄泄漏检查
    return 0;
}
