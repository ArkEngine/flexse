#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include "fileblock.h"

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        exit(1);
    }
    const uint32_t SIZE = atoi(argv[1]);
    fileblock myfl("./data/", "myfl", sizeof(uint32_t));
    myfl.begin();
    for (uint32_t i=0; i<SIZE; i++)
    {
        myfl.write_next((char*)&i);
    }
    myfl.begin();
    for (uint32_t i=0; i<SIZE; i++)
    {
        uint32_t v = 0;
        int x = myfl.read_next((char*)&v, sizeof(uint32_t));
        assert(x == sizeof(uint32_t) && v == i);
//        printf("x[%d] v[%u] msg[%m]\n", x, v);
    }
    return 0;
}
