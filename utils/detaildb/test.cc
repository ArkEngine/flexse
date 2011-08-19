#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "detaildb.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        exit(1);
    }
    const uint32_t loopcount = atoi(argv[1]);
    detaildb dtdb("./data/", "test");
    const uint32_t SIZE = 100;
    char* str = (char*)malloc(SIZE);
    memset(str, 'A', SIZE);
    for (uint32_t i=0; i<loopcount; i++)
    {
        assert (0 == dtdb.set(i, str, SIZE));
    }
    return 0;
}
