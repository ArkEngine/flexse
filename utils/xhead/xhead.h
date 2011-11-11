#ifndef  __XHEAD_H_
#define  __XHEAD_H_
#include <stdint.h>

/*
 * Name : xHead
 * Feature:
 */
typedef struct _xhead_t
{
    uint32_t log_id;       
    char     srvname[16];  
    union{
        uint32_t version;      // 接口版本
        uint32_t file_no;      // 消息队列进度中的当前文件号
    };
    union{
        uint32_t reserved;     // 保留位
        uint32_t block_id;     // 消息队列进度中的块号
    };
    union{
        uint32_t status;       // 状态位
        uint32_t all_num;      // 总数
    };
    uint32_t detail_len;   // xhead_t后面的变长数据
}xhead_t;

// [xhead_t]+[buffer]
int xsend(int sock, const xhead_t* xhead, const uint32_t timeo);
// [xhead_t]+[buffer]
int xrecv(int sock, xhead_t* xhead, const uint32_t size, const uint32_t timeo);

#endif  //__XHEAD_H_
