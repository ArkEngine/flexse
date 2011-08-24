#include "mylog.h"
#include <stdio.h>

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
    printf("ooxx\n");
    return 0;
}
