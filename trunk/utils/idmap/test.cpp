#include <stdio.h>
#include "idmap.h"

int main()
{
    printf("hello world!\n");
    idmap myidmap("./", 10000, 10000);
    for (uint32_t i=0; i<10000; i++)
    {
        printf("%u\n", myidmap.allocInnerID(i+1));
    }
    return 0;
}
