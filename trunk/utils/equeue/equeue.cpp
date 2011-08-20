#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <queue>
#include <errno.h>
#include "equeue.h"
#include "mylog.h"
#include "myutils.h"

using namespace std;
using namespace flexse;

/*
 * equeue is my name.
 * (1) 设置监听后，每次收到连接请求时，把这个请求放入socket队列中;
 * (2) 如果服务正常，则继续放入epoll中，等待事件发生;
 * (3) 如果服务不正常，如接收失败/发送失败，则关闭socket，从epoll中删除socket
 * (4) 采用EPOLLONESHOT|EPOLLET模式，当从poll里取出后，关于这个sock就不再发提醒了。
 * (5) 建议C/S采用问答的方式，否则后果自负。因为无法保证一个套接口只被一个线程服务。
 * (6) 能检测到客户端关闭的情况，对内核有版本要求。2.6.17以上支持
 * (7) EPOLLONESHOT 方式能避免(5), 避免在多线程的情况下，多个线程使用同一个fd
 */

equeue::equeue(const int epollsize, const uint16_t port)
{
    pthread_mutex_init(&m_qmutex, NULL);
    pthread_cond_init(&m_qcond, NULL);
    m_epollsize = epollsize;
    m_port = port;

    m_epollfd = epoll_create(m_epollsize);
    if (m_epollfd < 0)
    {
        FATAL( "create epoll fail. size[%u]", m_epollsize);
        while(0!=raise(SIGKILL)) {}
    }
    else
    {
        ROUTN( "create epoll[%u] on size[%u]", m_epollfd, m_epollsize);
    }

    if ((m_listenfd = flexse::mylisten(m_port)) < 0)
    {
        while(0!=raise(SIGKILL)) {}
    }

    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = m_listenfd;
    if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, m_listenfd, &ev) < 0)
    {
        ALARM( "epoll set insertion error: fd[%d]", m_epollfd);
        while(0!=raise(SIGKILL)) {}
    }
}

void equeue::running()
{
    struct epoll_event ev;
    struct epoll_event events[MAXESOCKQSIZE];
    for(;;) {
        int nfds = epoll_wait(m_epollfd, events, MAXESOCKQSIZE, -1);
        for(int n = 0; n < nfds; ++n) {
            if(events[n].data.fd == m_listenfd)
            {
                // this is a new one
                sockaddr_in cltaddr;
                socklen_t   caddr_len=0;
                int clientfd = accept(m_listenfd, (struct sockaddr *) &cltaddr, &caddr_len);
                if(clientfd < 0)
                {
                    ALARM( "accept client fail, don't fuck me. msg[%m]");
                    continue;
                }
//                flexse :: setnonblock(clientfd);
                int tcp_nodelay = 1;
                setsockopt(clientfd, IPPROTO_TCP, TCP_NODELAY, &tcp_nodelay, sizeof(int));
                ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
//              ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
                ev.data.fd = clientfd;
                if (epoll_ctl(m_epollfd, EPOLL_CTL_ADD, clientfd, &ev) < 0)
                {
                    ALARM( "epoll set insertion error: fd[%d]", clientfd);
                    close(clientfd);
                }
//                else
//                {
//                    DEBUG( "epoll set insertion ok: fd[%d] total[%u] ok[%d] events[%d]",
//                            clientfd, nfds, events[n].events & EPOLLRDHUP, events[n].events);
//                }
// 如果是RT的方式，必须重新设置一下，那个ONESHOT呢?
//             ev.events = EPOLLIN | EPOLLET;
//             ev.data.fd = m_listenfd;
//             epoll_ctl(m_epollfd, EPOLL_CTL_MOD, m_listenfd, &ev);
            }
            else
            {
//                DEBUG( "socket react: fd[%d] total[%u] ok[%d] events[%d]",
//                        events[n].data.fd, nfds, events[n].events & EPOLLRDHUP, events[n].events);
                // client 端关闭了连接。
                if (events[n].events & EPOLLRDHUP)
                {
                    close(events[n].data.fd);
                    continue;
                }
                pthread_mutex_lock(&m_qmutex);
//                if (m_qsock.size() > 0 && events[n].data.fd == m_qsock.front())
//                {
//                    DEBUG("======: qsize[%d] fd[%d] front[%d]",
//                            m_qsock.size(), events[n].data.fd,  m_qsock.front());
//                }
                m_qsock.push(events[n].data.fd);
                pthread_cond_signal(&m_qcond);
                pthread_mutex_unlock(&m_qmutex);
            }
        }
    }
}

equeue::~equeue()
{
    if (m_listenfd != -1)
    {
        close(m_listenfd);
        m_listenfd = -1;
    }
}

int equeue::fetch_socket()
{
    pthread_mutex_lock(&m_qmutex);
    while (0 == m_qsock.size())
    {
        pthread_cond_wait(&m_qcond, &m_qmutex);
    }
    int cursock = m_qsock.front();
    m_qsock.pop();
    DEBUG( "epoll queue size[%d]", m_qsock.size());
    pthread_mutex_unlock(&m_qmutex);
    return cursock;
}

int equeue::free_socket(int sock, bool ok)
{
    if (sock < 0)
    {
        return -1;
    }
    struct epoll_event ev;
    if (!ok)
    {
//        DEBUG( "close the socket[%d]", sock);
        close(sock);
        return 0;
    }
    else
    {
        pthread_mutex_lock(&m_qmutex);
        ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT | EPOLLRDHUP;
//      ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = sock;
        // 必须重新设置，否则这个套接口不会被再次激活了。
        // 也就是说，如果是长连接的话，服务完第一次，就没有第二次了。
        int ret = epoll_ctl(m_epollfd, EPOLL_CTL_MOD, sock, &ev);
        pthread_mutex_unlock(&m_qmutex);
        if (( ret != 0) && errno != EEXIST)
        {
            ALARM( "epoll set insertion error: fd[%d] errno[%d] ret[%d] EE[%d] msg[%m]",
                    sock, errno, ret, EEXIST);
            close(sock);
            return -1;
        }
//        else
//        {
//            DEBUG( "epoll set insertion ok: fd[%d]", sock);
//        }
    }
    return 0;
}
