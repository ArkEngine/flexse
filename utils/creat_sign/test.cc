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

int main()
{
    printf("hello world.\n");
    func("h", 1);
    func("h", 2);
    func("h0", 2);
    func("h0", 3);
    func("我们", 6);
    func("我们", 7);
    func("明天", 6);
    func("明天", 7);
    func("明天", 6);
    func("明天", 7);
    return 0;
}

