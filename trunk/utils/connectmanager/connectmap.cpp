#include "connectmap.h"
#include "MyException.h"
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <myutils.h>
#include <string.h>
#include <stdlib.h>
#include <netinet/in.h>

using namespace flexse;

ConnectMap::ConnectMap()
{
    m_health_line = DFT_HEALTH_LINE;
    m_retry_line  = DFT_RETRY_LINE;
    m_connectto_ms = DFT_CONNECT_TIMEOUT;
    m_check_interval = DFT_CHECK_INTERVAL;
    struct timeval now;
    gettimeofday(&now, NULL);
    m_last_check = (uint32_t)(now.tv_sec);
    m_server_deadline = DFT_SERVER_DEADLINE;
    m_err_server_conn = 0;
    m_err_server_full = 0;
    m_punish_mode = true;
    pthread_mutex_init(&m_mutex, NULL);
}

ConnectMap::~ConnectMap()
{
    pthread_mutex_destroy(&m_mutex);
}

uint32_t ConnectMap::ServerSockFullCount()
{
    uint32_t tmp = m_err_server_full;
    m_err_server_full = 0;
    return tmp;
}

uint32_t ConnectMap::ServerErrConnectCount()
{
    uint32_t tmp = m_err_server_conn;
    m_err_server_conn = 0;
    return tmp;
}

void ConnectMap::SetPunishMode(bool mode)
{
    m_punish_mode = mode;
}

void ConnectMap::SetConnectTO(const uint32_t ctimeout)
{
    m_connectto_ms = ctimeout;
}

void ConnectMap::SetRetryLine(const uint32_t rline)
{
    m_retry_line = rline;
}
void ConnectMap::SetHealthLine(const uint32_t hline)
{
    m_health_line = hline;
}
void ConnectMap::SetDeadline  (const uint32_t dline)
{
    m_server_deadline = dline;
}

void ConnectMap::SetCheckInterval(const uint32_t checkinterval)
{
    m_check_interval = checkinterval;
}

int ConnectMap::DelServer  (const char* host, const int port)
{
    if (host == NULL || port <= 0)
    {
        ALARM("Param error. server ip[NULL] or port[%d] <= 0", port);
        return ECONNECT_PARAM;
    }
    int ret = ECONNECT_SUCCESS;
    char key[MODULE_ADDRESS_MAXLEN];
    snprintf (key, sizeof(key), "%s:%d", host, port);

    pthread_mutex_lock(&m_mutex);
    m_icm = m_connectmap.find(string(key));
    if (m_icm == m_connectmap.end())
    {
        ALARM("DelServer fail. server[%s:%d] NOT exist in ConnectMap", host, port);
    }
    else
    {
        ALARM("DelServer[%s:%d] success in ConnectMap, status change to DELETE", host, port);
        connect_info_t& myconnect_info = m_icm->second;
        myconnect_info.status = SERVER_DELETE;
        int has_busy_sock = 0;
        for (uint32_t i=0; i<myconnect_info.module.socknum; i++)
        {
            if (myconnect_info.sockarr[i].status == SOCK_READY)
            {
                close(myconnect_info.sockarr[i].sock);
                myconnect_info.sockarr[i].sock = -1;
                myconnect_info.sockarr[i].status = SOCK_EMPTY;
            }
            else if (myconnect_info.sockarr[i].status == SOCK_BUSY)
            {
                has_busy_sock = 1;
            }
        }
        if (! has_busy_sock)
        {
            m_connectmap.erase(m_icm);
            DEBUG("DelServer[%s:%d] success in ConnectMap, Real delete immediately", host, port);
        }
    }
    pthread_mutex_unlock(&m_mutex);
    return ret;

}

int ConnectMap::__AddServer  (const module_info_t& module_info)
{
    const char* host = module_info.host;
    int port = module_info.port;
    if (strlen(host) == 0 || port <= 0)
    {
        ALARM("Param error. server[%s:%d]", host, port);
        return ECONNECT_PARAM;
    }
    int ret = 0;
    connect_info_t connect_info;
    memset (&connect_info, 0, sizeof(connect_info));
    connect_info.module = module_info;
    uint32_t socknum = connect_info.module.socknum;
    connect_info.module.socknum = socknum > SOCK_MAXNUM_PER_SERVER ? SOCK_MAXNUM_PER_SERVER : socknum;
    connect_info.status = SERVER_BUSY;
    connect_info.fail_count = 0;
    connect_info.service = 0;
    connect_info.timestamp = 0;
    connect_info.freenum = connect_info.module.socknum;
    for (uint32_t i=0; i<SOCK_MAXNUM_PER_SERVER; i++)
    {
        connect_info.sockarr[i].sock = -1;
        connect_info.sockarr[i].status = SOCK_EMPTY;
    }

    char key[MODULE_ADDRESS_MAXLEN];
    snprintf (key, sizeof(key), "%s:%d", host, port);
    m_icm = m_connectmap.find(string(key));
    if (m_icm == m_connectmap.end())
    {
        m_connectmap[string(key)] = connect_info;
        DEBUG("AddServer[%s][%s:%d] success in ConnectMap. A new one",
                module_info.name, host, port);
    }
    else if (0 != m_icm->first.compare(key))
    {
        FATAL("shit happens, insidekey[%s], outsidekey[%s]",
                m_icm->first.c_str(), key);
        MySuicideAssert(0);
    }
    else if (m_icm->second.status != SERVER_DELETE)
    {
        DEBUG("AddServer success. server[%s][%s:%d] exist in ConnectMap",
                module_info.name, host, port);
        __UpdateServerInfo(m_icm->second, module_info);
        ret = ECONNECT_SUCCESS;
    }
    else
    {
        DEBUG("AddServer[%s][%s:%d] success in ConnectMap. status[%u] change to BUSY",
                module_info.name, host, port, m_icm->second.status);
        m_icm->second.status = SERVER_BUSY;
        __UpdateServerInfo(m_icm->second, module_info);
        ret = ECONNECT_SUCCESS;
    }
    return ret;
}



int ConnectMap::AddServer  (const module_info_t& module_info)
{
    pthread_mutex_lock(&m_mutex);
    int ret = __AddServer(module_info);
    pthread_mutex_unlock(&m_mutex);
    return ret;
}

int ConnectMap::FreeSocket(const int sock, bool errclose)
{
    pthread_mutex_lock(&m_mutex);
    int found = 0;
    for (m_icm = m_connectmap.begin(); m_icm != m_connectmap.end(); m_icm++)
    {
        connect_info_t& connect_info = m_icm->second;
        uint32_t idx = 0;
        while (idx < connect_info.module.socknum)
        {
            if (connect_info.sockarr[idx].sock == sock)
            {
                found = 1;
                MySuicideAssert (connect_info.sockarr[idx].status == SOCK_BUSY);
                if (connect_info.status == SERVER_DELETE)
                {
                    close(sock);
                    connect_info.sockarr[idx].status = SOCK_EMPTY;
                    connect_info.sockarr[idx].sock = -1;
                    connect_info.freenum++;
                    if (connect_info.freenum == connect_info.module.socknum)
                    {
                        // ASSERT
                        for (uint32_t i=0; i<connect_info.module.socknum; i++)
                        {
                            MySuicideAssert (connect_info.sockarr[idx].status == SOCK_EMPTY);
                        }
                        ALARM("DelServer[%s:%d] success in ConnectMap, "
                                "Real delete this server", connect_info.module.host, connect_info.module.port);
                        m_connectmap.erase(m_icm); // NOTE: U CAN NOT use m_icm any more!!
                    }
                }
                else
                {
                    connect_info.freenum++;
                    if (errclose || connect_info.module.longconnect == 0)
                    {
                        close(sock);
                        connect_info.sockarr[idx].status = SOCK_EMPTY;
                        connect_info.sockarr[idx].sock = -1;
                        DEBUG("clsoe socket for errclose[%d] longconnect[%d]",
                                errclose, connect_info.module.longconnect);
                        if (errclose && m_punish_mode)
                        {
                            connect_info.fail_count++;
                            DEBUG("punish me~ cau'z i am a bad boy. host[%s] port[%u] fail[%u]",
                                    connect_info.module.host, connect_info.module.port, connect_info.fail_count);
                        }
                    }
                    else
                    {
                        connect_info.sockarr[idx].status = SOCK_READY;
                    }
                }
                break;
            }
            idx++;
        }
        if (found)
        {
            break;
        }
    }
    MySuicideAssert(found);
    pthread_mutex_unlock(&m_mutex);
    return ECONNECT_SUCCESS;
}

int ConnectMap::__FetchSocket(const char* host, const int port)
{
    if (host == NULL || port <= 0)
    {
        ALARM("Param error. server ip[NULL] or port[%d] < 0 @ [%s:%d]",
                port, __FILE__, __LINE__);
        return ECONNECT_PARAM;
    }
    int sock = ECONNECT_GENERAL;
    char key[MODULE_ADDRESS_MAXLEN];
    snprintf (key, sizeof(key), "%s:%d", host, port);

    m_icm = m_connectmap.find(string(key));
    if (m_icm == m_connectmap.end())
    {
        ALARM("Can't find server[%s:%d]", host, port);
        sock = ECONNECT_SRVNOTEXIST;
        m_err_server_miss++;
        return sock;
    }
    connect_info_t& connect_info = m_icm->second;
    connect_info.service++;

    timeval now;
    gettimeofday(&now, NULL);
    connect_info.timestamp = now.tv_sec;
    if (connect_info.freenum <= 0)
    {
        ALARM("Socket Full server[%s:%d] sockmaxnum[%d]",
                host, port, connect_info.module.socknum);
        m_err_server_full++;
        sock = ECONNECT_SRVSOCKFULL;
        return sock;
    }
    else
    {
        uint32_t idx=0;
        while (idx<connect_info.module.socknum)
        {
            if (connect_info.sockarr[idx].status == SOCK_BUSY)
            {
                idx++;
            }
            else if (connect_info.sockarr[idx].status == SOCK_READY)
            {
                sock = connect_info.sockarr[idx].sock;
                if (0 == __CheckConnection(sock))
                {
                    connect_info.sockarr[idx].status = SOCK_BUSY;
                    connect_info.freenum--;
                }
                else
                {
                    close(sock);
                    if (0 != __ConnectServer(connect_info, idx))
                    {
                        ALARM("Connect to server[%s:%d] Fail.", host, port);
                        connect_info.sockarr[idx].status = SOCK_EMPTY;
                        connect_info.sockarr[idx].sock = -1;
                        connect_info.fail_count++;
                        sock = ECONNECT_SRVCONCT;
                    }
                    else
                    {
                        sock = connect_info.sockarr[idx].sock;
                        connect_info.sockarr[idx].status = SOCK_BUSY;
                        connect_info.freenum--;
                        connect_info.fail_count = 0; // reset it
                    }
                }
                break;
            }
            else if (connect_info.sockarr[idx].status == SOCK_EMPTY)
            {
                break;
            }
            else
            {
                MySuicideAssert(0);
            }
        }
        MySuicideAssert (idx<connect_info.module.socknum);
        if (sock < 0) // cause by find a empty cell or a ready-sock checked fail
        {
            if (0 != __ConnectServer(connect_info, idx))
            {
                connect_info.sockarr[idx].status = SOCK_EMPTY;
                connect_info.sockarr[idx].sock = -1;
                connect_info.fail_count++;
                sock = ECONNECT_SRVCONCT;
            }
            else
            {
                sock = connect_info.sockarr[idx].sock;
                connect_info.sockarr[idx].status = SOCK_BUSY;
                connect_info.freenum--;
                connect_info.fail_count = 0; // reset it
            }
        }
    }

    DEBUG("server[%s:%d] socknum[%d] freenum [%d] service[%u]",
            host, port, connect_info.module.socknum, connect_info.freenum, connect_info.service);
    __CheckDeadServer();
    return sock;
}


int ConnectMap::FetchSocket(const module_info_t& module_info, bool healthcheck)
{
    const char* host  = module_info.host;
    const int   port  = module_info.port;
    if (host == NULL || port <= 0)
    {
        ALARM("Param error. server ip[NULL] or port[%d] <= 0", port);
        return ECONNECT_PARAM;
    }
    int sock = ECONNECT_GENERAL;
    char key[MODULE_ADDRESS_MAXLEN];
    snprintf (key, sizeof(key), "%s:%d", host, port);

    pthread_mutex_lock(&m_mutex);
    m_icm = m_connectmap.find(string(key));
    if (m_icm == m_connectmap.end())
    {
        __AddServer(module_info);
    }

    // find again
    if (m_icm == m_connectmap.end())
    {
        m_icm = m_connectmap.find(string(key));
        if (m_icm == m_connectmap.end())
        {
            ALARM("Can't find server[%s:%d]", host, port);
            sock = ECONNECT_SRVNOTEXIST;
            pthread_mutex_unlock(&m_mutex);
            return sock;
        }
    }

    connect_info_t& connect_info = m_icm->second;
    if (0 != m_icm->first.compare(key))
    {
        FATAL("shit happens, insidekey[%s], outsidekey[%s]",
                m_icm->first.c_str(), key);
    }
    else
    {
        __UpdateServerInfo(connect_info, module_info);
    }

    if (connect_info.status == SERVER_DELETE)
    {
        ALARM("Can't connect server[%s:%d] status[DELETE] ", host, port);
        sock = ECONNECT_SRVSTATUS;
        goto FAILEXIT;
    }
    if (healthcheck
            && connect_info.fail_count > m_health_line
            && ((connect_info.fail_count*MAGIC_NUM)&RECONNECT_BOUNDRY) < m_retry_line)
    {
        DEBUG("Too much fail on Server[%s:%u] failcount[%u], healthline[%u] retryline[%u]",
                host, port, connect_info.fail_count, m_health_line, m_retry_line);
        connect_info.fail_count++;
        sock = ECONNECT_SRVDOWN;
        goto FAILEXIT;
    }

    sock = __FetchSocket(host, port);
FAILEXIT:
    pthread_mutex_unlock(&m_mutex);
    return sock;
}

int ConnectMap::__CheckConnection(const int sock)
{
    char buf[1];
    ssize_t ret = recv(sock, buf, sizeof(buf), MSG_DONTWAIT);
    if(ret>0)
    {
        ALARM("check_connection: some data pending, error accur! sock[%d]", sock);
        return -1;
    }
    else if (ret==0)
    {
        DEBUG("check_connection: connection close by peer! sock[%d]", sock);
        return -1;
    }
    else
    {
        if(errno == EWOULDBLOCK)
        {
            return 0;//connection ok
        }
        else
        {
            ALARM("check_connection: read error[%d] on sock[%d] ERR[%m]", errno, sock);
            return -1;
        }
    }
    return -1;
}

int ConnectMap::__ConnectServer(connect_info_t& connect_info, const int idx)
{
    int ret = -1;
    int sock = connect_ms(connect_info.module.host, connect_info.module.port, m_connectto_ms);

    if (sock<0)
    {
        ALARM("[%s] ul_tcpconnecto_ms failed addr(%s) port(%d) timeout(%d) ERR[%m]",
                connect_info.module.name, connect_info.module.host, connect_info.module.port, m_connectto_ms);
        m_err_server_conn++;
        ret = -1;
    }
    else
    {
        connect_info.sockarr[idx].sock = sock;
        connect_info.sockarr[idx].status = SOCK_READY;
        int on =1;
        if (setsockopt(connect_info.sockarr[idx].sock,IPPROTO_TCP,TCP_NODELAY,&on, sizeof(on)) < 0)
        {
            ALARM("(setsockopt(fd,IPPROTO_TCP,TCP_NODELAY,&on, sizeof(on)) ERR(%m)");
        }
        ret = 0;
    }

    return ret;
}

void ConnectMap::__UpdateServerInfo(connect_info_t& connect_info, const module_info_t& src_module)
{
    const char* host  = connect_info.module.host;
    const int   port  = connect_info.module.port;

    if (connect_info.module.socknum < src_module.socknum)
    {
        uint32_t tsocknum = src_module.socknum;
        if (src_module.socknum > SOCK_MAXNUM_PER_SERVER || src_module.socknum <= 0)
        {
            tsocknum = SOCK_MAXNUM_PER_SERVER;
        }
        DEBUG("server[%s:%d] socknum[%d] change to [%d]",
                host, port, connect_info.module.socknum, tsocknum);
        connect_info.freenum += tsocknum - connect_info.module.socknum;
        connect_info.module.socknum = tsocknum;
    }
    if (connect_info.module.longconnect != src_module.longconnect)
    {
        DEBUG("server[%s:%d] longconnect[%d] change to [%d]",
                host, port, connect_info.module.longconnect, src_module.longconnect);
        connect_info.module.longconnect = (src_module.longconnect == 0? 0 : 1);
    }
}

void ConnectMap::__CheckDeadServer()
{
    struct timeval now;
    if (0 != gettimeofday(&now, NULL))
    {
        return;
    }
    if ((now.tv_sec - m_last_check) < m_check_interval)
    {
        return;
    }
    m_last_check = (uint32_t)(now.tv_sec);
    for (m_icm = m_connectmap.begin(); m_icm != m_connectmap.end(); m_icm++)
    {
        if ((now.tv_sec - m_icm->second.timestamp) > (int)m_server_deadline)
        {
            int has_busy_sock = 0;
            connect_info_t connect_info = m_icm->second;
            for (uint32_t i=0; i< connect_info.module.socknum; i++)
            {
                if (connect_info.sockarr[i].status == SOCK_READY)
                {
                    close(connect_info.sockarr[i].sock);
                    connect_info.sockarr[i].sock = -1;
                    connect_info.sockarr[i].status = SOCK_EMPTY;
                }
                else if (connect_info.sockarr[i].status == SOCK_BUSY)
                {
                    has_busy_sock = 1;
                }
            }
            if (! has_busy_sock)
            {
                ROUTN("DelServer[%s:%d] success in ConnectMap, Real delete immediately",
                        connect_info.module.host, connect_info.module.port);
                for (uint32_t i=connect_info.module.socknum; i<SOCK_MAXNUM_PER_SERVER; i++)
                {
                    MySuicideAssert(connect_info.sockarr[i].status == SOCK_EMPTY);
                }
                MySuicideAssert(connect_info.freenum == connect_info.module.socknum);
                connect_info.status = SERVER_DELETE;
                m_connectmap.erase(m_icm); // can't use this m_icm any more
                break;
            }
            else
            {
                ALARM("This server[%s:%d] dead long long ago, but still have busy sock.",
                        connect_info.module.host, connect_info.module.port);
            }
        }
    }
    return;
}
