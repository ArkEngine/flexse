#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "xHead.h"
#include <sys/types.h>
#include <sys/socket.h>

int xrecv(int sock, xhead_t* xhead, const uint32_t buffsize,
		const uint32_t timeo_ms)
{
	if (buffsize < sizeof(xhead_t) || sock < 0)
	{
		return -1;
	}
	struct timeval timeout = {0, 0};
	timeout.tv_sec  = timeo_ms/1000;
	timeout.tv_usec = 1000*(timeo_ms%1000);
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeval));
	int ret = recv(sock, (char*)xhead, sizeof(xhead_t), MSG_WAITALL);
	if (ret != sizeof(xhead_t))
	{
		return -2;
	}
	if (buffsize-sizeof(xhead_t) < xhead->detail_len)
	{
        fprintf(stderr, "buffsize[%d] detail[%d]\n", buffsize, xhead->detail_len);
		return -3;
	}
	ret = recv(sock, (char*)(xhead+1), xhead->detail_len, MSG_WAITALL);
	if(ret != (int)xhead->detail_len)
	{
		return -4;
	}
	return 0;
}

int SendNByte(const int fd, const char* buff, const uint32_t size, uint32_t flag)
{
	int32_t  left   = size;
	uint32_t offset = size;
	while (left > 0)
	{
		int len = send(fd, buff+offset-left, left, flag);
		if ((len == -1 || len == 0 ) && errno != EAGAIN)
		{
			return -1;
		}
		left -= len;
	}
	return size;
}

int xsend(int sock, const xhead_t* xhead, const uint32_t timeo_ms)
{
	if (xhead == NULL || sock < 0)
	{
		return -1;
	}
	int size = sizeof(xhead_t) + xhead->detail_len;
	struct timeval timeout = {0, 0};
	timeout.tv_sec  = timeo_ms/1000;
	timeout.tv_usec = 1000*(timeo_ms%1000);
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeval));
	int ret = SendNByte(sock, (char*)xhead, size, 0);
	if (ret != size)
	{
        fprintf(stderr, "size[%d] detail[%d] errno[%d]\n", size, xhead->detail_len, errno);
		return -2;
	}
	return 0;
}
