#ifndef _UTILS_H_
#define _UTILS_H_
#include <stdint.h>

namespace flexse {

    int mylisten(const uint16_t port);

    int setnonblock(int32_t fd);
}
#endif
