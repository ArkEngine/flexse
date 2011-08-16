#ifndef _MYUTILS_H_
#define _MYUTILS_H_
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
using namespace std;

namespace flexse {

    int mylisten(const uint16_t port);

    int setnonblock(int32_t fd);

    bool is_comment(const char* str);

    bool is_valid_ip(const char* str);

    char* strip(char* str);
}
#endif
