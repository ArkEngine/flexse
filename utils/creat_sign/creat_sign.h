#ifndef _CREAT_SIGN_H_
#define _CREAT_SIGN_H_
#include <stdint.h>

uint32_t BKDRHash(const char *str, const uint32_t length);
uint32_t APHash(const char *str, const uint32_t length);
void creat_sign_64(const char* str, const uint32_t length, uint32_t* sign1, uint32_t* sign2);
#endif
