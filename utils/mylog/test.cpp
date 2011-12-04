#include "mylog.h"
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main()
{
    SETLOG(0, "test");
    FATAL("this is message");
    ALARM("this is message");
    ROUTN("this is message");
    DEBUG("this is message");
    uint32_t i = 1;
    uint64_t lol = 10;
    ROUTN("this is int[%u] str[%s] ptr[%p] lol[%llu]", i, "hello", &i, 3*lol);
    ROUTN("this is message");
    // BUG HERE TODO
    ROUTN("this is int[%u] str[%s] ptr[%p] lol[%llu]", i, "hello");
    char str[2000];
    memset(str, 'A', sizeof(str));
    str[1999] = 0;
    printf("%s\n", str);
    ROUTN("this is [%s]", str);
    ROUTN("this is message");
//    for (uint32_t x=0; x<1000000; x++)
//    {
//        ROUTN("this is test.");
//        usleep(10000);
//    }
    return 0;
}
