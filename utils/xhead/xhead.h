#ifndef  __XHEAD_H_
#define  __XHEAD_H_
#include <stdint.h>

/*
 * Name : xHead
 * Feature:
 * (1) ¿¿¿¿¿¿¿
 * (2) ¿¿¿¿¿¿¿/¿¿¿log_id/¿¿buffer
 */
typedef struct _xhead_t
{
	uint32_t log_id;       // ¿¿¿¿¿ log_id
	char     srvname[16];  // ¿¿¿¿¿¿
	uint32_t version;      // ¿¿¿¿
	uint32_t reserved;     // ¿¿¿¿
	uint32_t detail_len;   // xhead_t ¿¿¿buffer¿¿
}xhead_t;

// [xhead_t]+[buffer]
int xsend(int sock, const xhead_t* xhead, const uint32_t timeo);
// [xhead_t]+[buffer]
int xrecv(int sock, xhead_t* xhead, const uint32_t size, const uint32_t timeo);

#endif  //__XHEAD_H_
