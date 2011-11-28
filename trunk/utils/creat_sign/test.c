#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "creat_sign.h"

void func(const char* str, uint32_t length)
{
    uint32_t sign[2];
    creat_sign_64(str, length, &sign[0], &sign[1]);
    printf("str[%s] len[%u] sgn[%u:%u]\n", str, length, sign[0], sign[1]);
}

void myfunc(const char* mychar, uint32_t count)
{
    char str[120];
    union uuu{
        uint32_t sign[2];
        uint64_t key;
    }uu;
    int len = snprintf(str, sizeof(str), "%s[%u]", mychar, count);
    creat_sign_64(str, len, &uu.sign[0], &uu.sign[1]);
//    printf("%llu %s\n", key, str);
    printf("key: %s - sign[%lu]\n", str, uu.key);
}


int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test SIZE\n");
        exit(1);
    }
    const uint32_t SIZE = atoi(argv[1]);
    //    printf("hello world.\n");
    //    func("h", 1);
    //    func("h", 2);
    //    func("h0", 2);
    //    func("h0", 3);
    //    func("我们", 6);
    //    func("我们", 7);
    //    func("明天", 6);
    //    func("明天", 7);
    //    func("明天", 6);
    //    func("明天", 7);
    uint32_t i=0;
    for (i=0; i<SIZE; i++)
    {
        myfunc("a", i);
        myfunc("b", i);
        myfunc("c", i);
        myfunc("d", i);
        myfunc("e", i);
        myfunc("f", i);
        myfunc("g", i);
        myfunc("h", i);
        myfunc("i", i);
        myfunc("j", i);
        myfunc("k", i);
        myfunc("l", i);
        myfunc("m", i);
        myfunc("n", i);
        myfunc("o", i);
        myfunc("p", i);
        myfunc("q", i);
        myfunc("r", i);
        myfunc("s", i);
        myfunc("t", i);
    }
    return 0;
}
