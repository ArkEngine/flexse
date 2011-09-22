#ifndef _MYUTILS_H_
#define _MYUTILS_H_
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
using namespace std;

namespace flexse {

    struct term_info_t
    {
        string   strTerm;      ///> 分词后的一个term
        uint32_t id;           ///> 文档的ID
        uint32_t term_desc[4]; ///> 这个term的一些描述信息，最多支持4个uint32_t
    };

    int mylisten(const uint16_t port);

    int setnonblock(int32_t fd);

    bool is_comment(const char* str);

    bool is_valid_ip(const char* str);

    char* strip(char* str);

    int read_file_all(const char* file, char* buff, const uint32_t bufsize);

    int connect_ms(const char* host, const uint16_t port, const uint32_t timeout_ms);
}
#endif
