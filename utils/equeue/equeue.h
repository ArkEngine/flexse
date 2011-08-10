#ifndef _EQUEUE_H_
#define _EQUEUE_H_

#include <unistd.h>
#include <pthread.h>
#include <queue>

using namespace std;

/*
 * equeue is my name.
 * (1) 设置监听后，每次收到连接请求时，把这个请求放入socket队列中;
 * (2) 如果服务正常，则继续放入epoll中，等待事件发生;
 * (3) 如果服务不正常，如接收失败/发送失败，则关闭socket，从epoll中删除socket
 * (4) 采用SHOT|ET模式，当从poll里取出后，关于这个sock就不再发提醒了。
 * (5) 建议C/S采用问答的方式，否则后果自负。
 * (6) 当关闭套接口时，工作线程可能得到的是已经关闭的套接口
 */

class equeue
{
    public:
        equeue(const int epollsize, const uint16_t port);
        ~equeue();
        void running();
        int fetch_socket();
        int free_socket(int sock, bool ok);

    private:
        pthread_mutex_t m_qmutex;
        pthread_cond_t  m_qcond;
        std::queue<int> m_qsock;
        int m_listenfd;
        uint16_t m_port;
        int      m_epollsize;
        int      m_epollfd;
        enum {maxBackLog = 64, MAXESOCKQSIZE = 128,};
        equeue();
        equeue(const equeue&);
};

#endif
