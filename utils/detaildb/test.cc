#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "detaildb.h"

int main()
{
    detaildb dtdb("./data/", "test");
    const uint32_t SIZE = 100;
    char* str = (char*)malloc(SIZE);
    memset(str, 'A', SIZE);
    for (uint32_t i=0; i<100; i++)
    {
        dtdb.set(i, str, SIZE);
    }
    printf("hello world.");
    return 0;
}
