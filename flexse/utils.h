#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
using namespace std;

namespace flexse {

    int mylisten(const uint16_t port);

    int setnonblock(int32_t fd);

    void strspliter(char* str, vector<string>& vstr);
}
#endif
