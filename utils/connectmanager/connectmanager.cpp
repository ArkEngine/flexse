#include "connectmanager.h"
#include "mylog.h"
#include "myutils.h"
#include "creat_sign.h"
#include <string.h>

using namespace flexse;

const char* const ConnectManager :: m_StrConnectTimeOut   = "connect_timeout_ms";
const char* const ConnectManager :: m_StrServerHealthLine = "server_health_line";
const char* const ConnectManager :: m_StrRetryLine        = "server_retry_line";
const char* const ConnectManager :: m_StrServerDeadline   = "server_dead_line";
const char* const ConnectManager :: m_StrCheckInterval    = "server_check_interval";
const char* const ConnectManager :: m_StrServerPunishMode = "server_punish_mode";
const char* const ConnectManager :: m_StrUseCarpBalance   = "use_carp_balance";
const char* const ConnectManager :: m_StrServerConfigList = "server_config_list";

ConnectManager::ConnectManager(Json::Value config_json) : 
	m_connectmap()
{
	m_manager_service = 0;
	m_randseed = 0;
	m_manager_fail    = 0;
    m_server_punish_mode = true;

    m_connect_retry_line = m_DefaultRetryLine;
	if (!config_json[m_StrRetryLine].isNull() && config_json[m_StrRetryLine].isInt() )
	{
        m_connect_retry_line = config_json[m_StrRetryLine].asInt();
		if (m_connect_retry_line > ConnectMap :: RECONNECT_BOUNDRY)
		{
			MyToolThrow("Read server_retry_line error, Over ReconnectBoundry");
		}
	}
    ROUTN("[%s] : [%u]", m_StrRetryLine, m_connect_retry_line);

    m_server_health_line = m_DefaultServerHealthLine;
	if (!config_json[m_StrServerHealthLine].isNull() && config_json[m_StrServerHealthLine].isInt())
	{
		m_server_health_line = config_json[m_StrServerHealthLine].asInt();
        if (m_server_health_line <= 0)
        {
            m_server_health_line = m_DefaultServerHealthLine;
        }
	}
    ROUTN("[%s] : [%u]", m_StrServerHealthLine, m_server_health_line);

    m_connect_timeout = m_DefaultConnectTimeOut;
	if (!config_json[m_StrConnectTimeOut].isNull() && config_json[m_StrConnectTimeOut].isInt() )
	{
        m_connect_timeout = config_json[m_StrConnectTimeOut].asInt();
        if (m_connect_timeout < 10 || m_connect_timeout > 1000)
        {
            m_connect_timeout = m_DefaultConnectTimeOut;
        }
	}
    ROUTN("[%s] : [%u]", m_StrConnectTimeOut, m_connect_timeout);

    m_server_deadline = m_DefaultServerDeadline;
	if (!config_json[m_StrServerDeadline].isNull() && config_json[m_StrServerDeadline].isInt() )
	{
		m_server_deadline = config_json[m_StrServerDeadline].asInt();
        if (m_server_deadline < 100)
        {
            m_server_deadline = m_DefaultServerDeadline;
        }
	}
    ROUTN("[%s] : [%u]", m_StrServerDeadline, m_server_deadline);

    m_server_punish_mode = true;
	if (!config_json[m_StrServerPunishMode].isNull() && config_json[m_StrServerPunishMode].isInt() )
	{
		m_server_punish_mode = 0 != config_json[m_StrServerPunishMode].asInt();
	}
    ROUTN("[%s] : [%u]", m_StrServerPunishMode, m_server_punish_mode);

    m_check_interval = m_DefaultCheckInterval;
	if (!config_json[m_StrCheckInterval].isNull() && config_json[m_StrCheckInterval].isInt())
	{
		m_check_interval = config_json[m_StrCheckInterval].asInt();
		if (m_connect_retry_line > m_DefaultCheckInterval)
		{
            m_check_interval = m_DefaultCheckInterval;
		}
	}
    ROUTN("[%s] : [%u]", m_StrCheckInterval, m_check_interval);

    m_use_carp_balance = true;
	if (!config_json[m_StrUseCarpBalance].isNull() && config_json[m_StrUseCarpBalance].isInt())
	{
		m_use_carp_balance = 0 != config_json[m_StrUseCarpBalance].asInt();
	}
	ROUTN("[%s] : [%d]", m_StrUseCarpBalance, m_use_carp_balance);

	m_connectmap.SetConnectTO    (m_connect_timeout);
	m_connectmap.SetRetryLine    (m_connect_retry_line);
	m_connectmap.SetHealthLine   (m_server_health_line);
	m_connectmap.SetDeadline     (m_server_deadline);
	m_connectmap.SetCheckInterval(m_check_interval);
	m_connectmap.SetPunishMode   (m_server_punish_mode != 0);

    Json::Value resourcelist = config_json[m_StrServerConfigList];
    for (uint32_t i=0; i<resourcelist.size(); i++)
    {
        Json::Value resource = resourcelist[i];
        ProcessResource(resource, m_local_resource_info, RESOURCE_MAXNUM);
    }
}

ConnectManager::~ConnectManager()
{
    // TODO
}

int ConnectManager::FreeSocket(const int sock, bool errclose)
{
    return m_connectmap.FreeSocket(sock, errclose);
}

int ConnectManager::FetchSocket(const char* key, const char* strbalance)
{
    int sock = 0;
    m_manager_service++;
    uint32_t servernum = 0;
    module_info_t server[MODULE_MAXNUM_PER_RESOURCE];
    memset (server, 0, sizeof(server));
    servernum = GetServerList(key, server, MODULE_MAXNUM_PER_RESOURCE);
    if (servernum <= 0)
    {
        ALARM("GetServerList fail servernum[%d]. key[%s]", servernum, key);
        return -1;
    }
    CarpCalculate(server, servernum, strbalance);
    for (uint32_t sid=0; sid<servernum; sid++)
    {
        sock = m_connectmap.FetchSocket(server[sid]);
        if (sock > 0)
        {
            DEBUG(
                    "Key[%s] FetchSocket[%d] success. carpidx[%d] snum[%d]"
                    " host[%s] port[%d] longconnect[%d] balance[%s]",
                    key, sock, sid, servernum,
                    server[sid].host, server[sid].port, server[sid].longconnect, strbalance);
            return sock;
        }
        else
        {
            ALARM("FetchSocket from connectmap fail[%d]. querykey[%s] host[%s] port[%d]",
                    sock, key, server[sid].host, server[sid].port);
        }
    }
    // damn, all servers was blocked by health check
    bool healthcheck = false;
    for (uint32_t lucksid=0; lucksid<servernum; lucksid++)
    {
        uint32_t sid = (lucksid + m_randseed ++ ) % servernum;
        sock = m_connectmap.FetchSocket(server[sid], healthcheck);
        if (sock > 0)
        {
            DEBUG("Lucky~ FetchSocket[%d] success. carpidx[%d] snum[%d]host[%s] port[%d] longconnect[%d] balance[%s]",
                    sock, sid, servernum, server[sid].host, server[sid].port, server[sid].longconnect, strbalance);
            return sock;
        }
    }

    m_manager_fail++;
    return -1;
}
bool ConnectManager::find_idx(const char* resourcename, const resource_info_t* presource_info,
		const uint32_t array_size, int& ridx)
{
	ridx = -1;
	for (uint32_t i = 0; i < RESOURCE_MAXNUM && i < array_size; i++)
	{
		if (presource_info[i].name[0] == '\0')
		{
			// got a new cell
			ridx = i;
			return false;
		}
		if (0 == strcmp(presource_info[i].name, resourcename))
		{
			// get a old cell
			ridx = i;
			return true;
		}
	}
	// cells all full
	return false;
}

uint32_t ConnectManager::FailCount()
{
	uint32_t tmp = m_manager_fail;
	m_manager_fail = 0;
	return tmp;
}

uint32_t ConnectManager::ServiceCount()
{
	uint32_t tmp = m_manager_service;
	m_manager_service = 0;
	return tmp;
}

uint32_t ConnectManager::ServerSockFullCount()
{
	return m_connectmap.ServerSockFullCount();
}

uint32_t ConnectManager::ServerErrConnectCount()
{
	return m_connectmap.ServerErrConnectCount();
}

void ConnectManager::ProcessResource(Json::Value resource,
		resource_info_t* resourcelist, const int listsize)
{
	MySuicideAssert (NULL != resourcelist);
	int resourcenum = 0;

	const char* resourcename = resource["name"].asCString();

	int cur_ridx = -1;
	bool bret = find_idx(resourcename, resourcelist, listsize, cur_ridx);
	if (false == bret && cur_ridx >= 0 )
	{
		DEBUG("new server, new cell. name[%s]", resourcename);
		resourcenum++;
	}
	else if (false == bret && cur_ridx == -1)
	{
		ALARM("Resource_info array full. process %d resource, following resource will be droped", cur_ridx);
		return;
	}
	else
	{
		DEBUG("new server, old cell. name[%s]", resourcename);
	}
	if ( 0 == snprintf(resourcelist[cur_ridx].name,
				sizeof(resourcelist[0].name), "%s", resourcename))
	{
		ALARM("r u kiding me?. resourcename[%s]", resourcename);
	}
	int midx;
	for (midx=0; midx<MODULE_MAXNUM_PER_RESOURCE; midx++)
    {
        if (resourcelist[cur_ridx].modules[midx].port == (int)resource["port"].asInt()
                && 0 == strcmp(resourcelist[cur_ridx].modules[midx].host, resource["ip"].asCString()))
        {
            // repeat
            DEBUG("repeat resource host[%s] port[%u] name[%s]",
                    resource["ip"].asCString(), resource["port"].asInt(), resource["name"].asCString());
            break;
        }
        if (resourcelist[cur_ridx].modules[midx].port != 0)
        {
            // this cell is NOT empty
            continue;
        }
        resourcelist[cur_ridx].modules[midx].name[0] = 0;
        snprintf(resourcelist[cur_ridx].modules[midx].name,
                sizeof(resourcelist[0].modules[0].name), "%s", resourcename);

        resourcelist[cur_ridx].modules[midx].host[0] = 0;
        const char* ip = resource["ip"].asCString();
        if (0 == ip[0])
        {
            ALARM("Can't get ip in resourcepack @ resourcearray[%d]", cur_ridx);
            return;
        }
        snprintf(resourcelist[cur_ridx].modules[midx].host,
                sizeof(resourcelist[0].modules[0].host), "%s", ip);

        uint32_t port = resource["port"].asInt();
        if (port <= 0)
        {
            ALARM("Can't get port in resourcepack @ resourcearray[%d]", cur_ridx);
            return;
        }
        resourcelist[cur_ridx].modules[midx].port = (uint16_t)port;

        resourcelist[cur_ridx].modules[midx].longconnect = 444;
        uint32_t longconnect = resource["longconnect"].asInt();
        longconnect = longconnect > 1 ? 1 : longconnect;
        resourcelist[cur_ridx].modules[midx].longconnect = longconnect;

        resourcelist[cur_ridx].modules[midx].socknum = 31;
        uint32_t socknum = resource["sockperserver"].asInt();
        socknum = socknum > SOCK_MAXNUM_PER_SERVER ? SOCK_MAXNUM_PER_SERVER : socknum;
        resourcelist[cur_ridx].modules[midx].socknum = socknum;
        DEBUG("Process resource[%s] @ subsystem[%s] OK. "
                "address[%s:%d] longconnect[%d] socknum[%d] cur_ridx[%d] resourcenum[%u]",
                resourcelist[cur_ridx].name,
                resourcelist[cur_ridx].modules[midx].name,
                resourcelist[cur_ridx].modules[midx].host,
                resourcelist[cur_ridx].modules[midx].port,
                resourcelist[cur_ridx].modules[midx].longconnect,
                resourcelist[cur_ridx].modules[midx].socknum,
                cur_ridx,
                resourcenum
             );

        resourcelist[cur_ridx].module_num++;
        break;
    }
    if (midx == MODULE_MAXNUM_PER_RESOURCE)
    {
        ALARM("Modulenum > MODULE_MAXNUM_PER_RESOURCE[%d]. ignore this resource[%s].",
                MODULE_MAXNUM_PER_RESOURCE, resourcename);
    }
    return;
}

uint32_t ConnectManager::GetServerList(const char* key, module_info_t* server, const uint32_t listsize)
{
    MySuicideAssert (key && server && listsize > 0);
    uint32_t servernum = 0;

    uint32_t localresourcesize = sizeof(m_local_resource_info)/sizeof(m_local_resource_info[0]);
    for (uint32_t i=0; i<localresourcesize; i++)
    {
        if (0 == m_local_resource_info[i].name[0])
        {
            break;
        }
        if (0 == strcmp(key, m_local_resource_info[i].name))
        {
            for (int j=0; j<m_local_resource_info[i].module_num; j++)
            {
                if (servernum < listsize)
                {
                    if (0 == snprintf(server[servernum].host, sizeof(server[0].host),
                                "%s", m_local_resource_info[i].modules[j].host))
                    {
                        break;
                    }
                    if (0 == m_local_resource_info[i].modules[j].port)
                    {
                        break;
                    }
                    server[servernum].port        = m_local_resource_info[i].modules[j].port;
                    server[servernum].longconnect = m_local_resource_info[i].modules[j].longconnect;
                    server[servernum].socknum     = m_local_resource_info[i].modules[j].socknum;
                    snprintf(server[servernum].name, sizeof(server[0].name), "%s", m_local_resource_info[i].modules[j].name);
                    servernum++;
                }
            }
        }
    }
    DEBUG("get %u servers. key[%s]", servernum, key);
    return servernum;
}

int Compare_Carp(const void *a, const void *b)
{
    const module_info_t* pa = (module_info_t*)a;
    const module_info_t* pb = (module_info_t*)b;
    return (pa->carpvalue < pb->carpvalue);
}

void ConnectManager::CarpCalculate(module_info_t* server,
        const uint32_t servernum, const char* strbalance)
{
    char tmp[MAX_KEY_MERGE_LEN];
    memset(tmp, 0, sizeof(tmp));
    // 计算每个server的carp 值
    for (uint32_t i=0; i<servernum; i++)
    {
        if ((strbalance[0] == 0) || (! m_use_carp_balance))
        {
            server[i].carpvalue = rand();
            continue;
        }
        int len = snprintf(tmp, MAX_KEY_MERGE_LEN, "%d:%s:%s",
                server[i].port, strbalance, server[i].host);
        if (len >= MAX_KEY_MERGE_LEN-1)
        {
            ALARM("MAX_KEY_MERGE_LEN[%d] too short for [%s:%s:%d] so i cut something",
                    MAX_KEY_MERGE_LEN, strbalance, server[i].host, server[i].port);
        }
        uint32_t sign1, sign2;
        creat_sign_64(tmp, len, &sign1, &sign2);
        server[i].carpvalue = sign1 * sign2;
        DEBUG("host[%s] port[%d] balance[%s] carpvalue[%u] sign1[%u] sign2[%u]",
                server[i].host, server[i].port, strbalance, server[i].carpvalue, sign1, sign2);
    }
    // 按照优先级pri和carp值进行排序, 优先级高的放在前面，然后同一优先级的carp值大的放在前面
    qsort(server, servernum, sizeof(module_info_t), Compare_Carp);
    return;
}
