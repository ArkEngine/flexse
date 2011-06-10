#include <stdint.h>

uint32_t BKDRHash(const char *str, const uint32_t length);

uint32_t APHash(const char *str, const uint32_t length);

void creat_sign_BKDR_AP(const char* str, const uint32_t length, uint32_t* sign1, uint32_t* sign2);
