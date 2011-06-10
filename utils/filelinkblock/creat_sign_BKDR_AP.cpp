#include <stdint.h>

uint32_t BKDRHash(const char *str, const uint32_t length)
{
    uint32_t seed = 131; // 31 131 1313 13131 131313 etc..
    uint32_t hash = 0;

    for (uint32_t i=0; i<length; i++)
    {
        hash = hash * seed + (*str++);
    }

    return (hash & 0x7FFFFFFF);
}

uint32_t APHash(const char *str, const uint32_t length)
{
    uint32_t hash = 0;
    uint32_t i;

    for (i=0; i<length; i++)
    {
        if ((i & 1) == 0)
        {
            hash ^= ((hash << 7) ^ (*str++) ^ (hash >> 3));
        }
        else
        {
            hash ^= (~((hash << 11) ^ (*str++) ^ (hash >> 5)));
        }
    }

    return (hash & 0x7FFFFFFF);
}

void creat_sign_BKDR_AP(const char* str, const uint32_t length, uint32_t* sign1, uint32_t* sign2)
{
    *sign1 = BKDRHash(str, length);
    *sign2 = APHash(str, length);
    return;
}
