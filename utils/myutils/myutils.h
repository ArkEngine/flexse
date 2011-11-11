#ifndef _MYUTILS_H_
#define _MYUTILS_H_
#include <stdint.h>
#include <string>
#include <vector>
#include <set>
#include "algo.h"
using namespace std;

namespace flexse {

    struct term_info_t
    {
        string   strTerm;      ///> 分词后的一个term
        uint32_t id;           ///> 文档的ID
        uint32_t term_desc[5]; ///> 这个term的一些描述信息，最多支持5个uint32_t
    };

    struct query_param_t{
        char* jsonstr;                       ///< 原始的查询json-string
        uint32_t offset;                     ///< 起始位置 
        uint32_t size;                       ///< 从起始位置开始，到第几个结束
        uint32_t all_num;                    ///< 这里能查询到的总数
        string   orderby;
        vector<list_info_t>     term_vector; ///< term的列表
        vector<filter_logic_t>  filt_vector; ///< 过滤列表
        vector<ranking_logic_t> rank_vector; ///< 调权列表
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
