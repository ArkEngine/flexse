#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <string.h>
#include <stdint.h>

#include "mylog.h"
#include "myutils.h"


namespace flexse
{
    int mylisten(const uint16_t port)
    {
        struct sockaddr_in adr_srv;
        int len_adr;
        const char* listeningip = "0.0.0.0";
        int32_t listenfd = -1;

        len_adr = sizeof adr_srv;
        memset (&adr_srv, 0, len_adr);
        adr_srv.sin_family = AF_INET;
        adr_srv.sin_port = htons (port);
        adr_srv.sin_addr.s_addr = inet_addr (listeningip);
        if (INADDR_NONE == adr_srv.sin_addr.s_addr)
        {
            FATAL( "bad address. ip[%s] port[%u]", listeningip, port);
            return -1;
        }

        if ((listenfd = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
            FATAL("socket() fail. msg[%m]");
            return -1;
        }

        int reuse_on = 1;
        setsockopt( listenfd, SOL_SOCKET, SO_REUSEADDR, &reuse_on, sizeof(reuse_on) );

        if (-1 == bind (listenfd, (struct sockaddr *) &adr_srv, len_adr))
        {
            FATAL( "bind() fail. port[%u] msg[%m]", port);
            return -1;
        }

        const uint32_t maxBackLog = 10;
        if ( -1 == listen (listenfd, maxBackLog))
        {
            FATAL( "listen(listenfd[%u], backlog[%u]) fail. msg[%m]", listenfd, maxBackLog);
            return -1;
        }
        return listenfd;
    }

    int setnonblock(int fd)
    {
        int flags = fcntl(fd, F_GETFL);
        if (flags < 0)
        {
            return flags;
        }
        flags |= O_NONBLOCK;
        if(fcntl(fd, F_SETFL, flags)<0)
        {
            return -1;
        }
        return 0;
    }

    bool is_comment(const char* str)
    {
        while (*str == '\t' || *str == ' ')
        {
            str++;
        }
        return (*str == '#') ? true : false;
    }

    char* strip(char* str)
    {
        char* b = str;
        char* e = NULL;
        while (*b == '\t' || *b == ' ' || *b == '\n') {
            b++;
        }
        memmove(str, b, strlen(b));
        if (NULL != (e = strchr(str, ' '))) {
            *e = 0;
        }
        if (NULL != (e = strchr(str, '\t'))) {
            *e = 0;
        }
        if (NULL != (e = strchr(str, '\n'))) {
            *e = 0;
        }
        return str;
    }

    bool is_valid_ip(const char* str_ip)
    {
        struct sockaddr_in adr_srv;
        adr_srv.sin_addr.s_addr = inet_addr (str_ip);
        return (INADDR_NONE != adr_srv.sin_addr.s_addr);
    }

}
