#include "connectmanager.h"
#include "mylog.h"
#include "myutils.h"
#include "creat_sign.h"
#include <string.h>

using namespace flexse;

const char* const ConnectManager :: m_StrConnectTimeOut       = "connect_timeout_ms";
const char* const ConnectManager :: m_StrServerHealthLine     = "server_health_line";
const char* const ConnectManager :: m_StrRetryLine            = "server_retry_line";
const char* const ConnectManager :: m_StrServerDeadline       = "server_dead_line";
const char* const ConnectManager :: m_StrServerPunishMode     = "server_punish_mode";
const char* const ConnectManager :: m_StrCheckInterval        = "server_check_interval";
const char* const ConnectManager :: m_StrUseResourceServer    = "use_resource_server";
const char* const ConnectManager :: m_StrUseCarpBalance       = "use_carp_balance";
const char* const ConnectManager :: m_StrResourcePackPath     = "resource_pack_path";
const char* const ConnectManager :: m_DefaultResourcePackPath = "./conf/resourcepack";
const char* const ConnectManager :: m_StrResourceDumpPath     = "resource_dump_path";
const char* const ConnectManager :: m_DefaultResourceDumpPath = "./data/resourcedump";

bool ConnectManager::find_idx(const char* resourcename, const resource_info_t* presource_info,
		const u_int array_size, int& ridx)
{
	ridx = -1;
	for (u_int i = 0; i < RESOURCE_MAXNUM && i < array_size; i++)
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

ConnectManager::ConnectManager(const char* config_path) : 
	m_connectmap()
{
	m_first_run = 1;
	m_manager_service = 0;
	m_randseed = 0;
	m_manager_fail    = 0;
	m_server_punish_mode = 0;
	memset (m_local_resource_info,  0, sizeof(m_local_resource_info));
	memset (m_register_text, 0, sizeof(m_register_text));
	memset (m_register_pack, 0, sizeof(m_register_pack));
	memset (m_resource_pack, 0, sizeof(m_resource_pack));
    MySuicideAssert(NULL != config_path);

	m_tmpbuff  =  new char[TMPBUFFER_MAXSIZE];
	m_dumppack =  new char[TMPBUFFER_MAXSIZE];
	if (m_tmpbuff == NULL || m_dumppack == NULL)
	{
		MyToolThrow("mem alloc fail");
	}

//	if ( NULL == (m_pconfdata = ul_initconf( ConfigMaxNum )) )
//	{
//		throw SpaceException("ul_initconf fail");
//	}
//
//	if ( ul_readconf( configdir, configfile, m_pconfdata) < 0 )
//	{
//		ul_freeconf( m_pconfdata );
//		throw SpaceException("can't open config file", configfile);
//	}
//
//	if ( ul_getconfint( m_pconfdata, m_StrUseResourceServer, &m_use_resource_server) <=0 )
//	{
//		m_use_resource_server = 1; // default
//	}
//	else
//	{
//		m_use_resource_server = (m_use_resource_server != 0);
//	}
//
//	if (listen_port <=0 || listen_port > 65535)
//	{
//		throw SpaceException ("query_port error", "boundary < 0 or > 65535");
//	}
//	m_query_listen_port = listen_port;
//
//	if (( ul_getconfint( m_pconfdata, m_StrRetryLine, &m_connect_retry_line) <=0 )
//			|| (m_connect_retry_line <= 0))
//	{
//		ROUTN("read [%s] err. use default [%u]", m_StrRetryLine, m_DefaultRetryLine);
//		m_connect_retry_line = m_DefaultRetryLine;
//	}
//	else
//	{
//		if (m_connect_retry_line > ConnectMap :: RECONNECT_BOUNDRY)
//		{
//			throw SpaceException ("Read server_retry_line error", "Over ReconnectBoundry");
//		}
//		ROUTN("[%s] : [%u]", m_StrRetryLine, m_connect_retry_line);
//	}
//
//	if (( ul_getconfint( m_pconfdata, m_StrServerHealthLine, &m_server_health_line) <=0 )
//			|| (m_server_health_line <= 0))
//	{
//		m_server_health_line = m_DefaultServerHealthLine;
//		ROUTN("read [%s] err. use default [%u]", m_StrServerHealthLine, m_DefaultServerHealthLine);
//	}
//	else
//	{
//		ROUTN("[%s] : [%u]", m_StrServerHealthLine, m_server_health_line);
//	}
//
//	if (( ul_getconfint( m_pconfdata, m_StrConnectTimeOut, &m_connect_timeout) <=0 )
//			|| (m_connect_timeout < 10))
//	{
//		m_connect_timeout = m_DefaultConnectTimeOut;
//		ROUTN("read [%s] err. use default [%u]", m_StrConnectTimeOut, m_DefaultConnectTimeOut);
//	}
//	else
//	{
//		ROUTN("[%s] : [%u]", m_StrConnectTimeOut, m_connect_timeout);
//	}
//
//	if (( ul_getconfint( m_pconfdata, m_StrServerDeadline, &m_server_deadline) <=0 )
//			|| (m_server_deadline < 100))
//	{
//		m_server_deadline = m_DefaultServerDeadline;
//		ROUTN("read [%s] err. use default [%u]", m_StrServerDeadline, m_DefaultServerDeadline);
//	}
//	else
//	{
//		ROUTN("[%s] : [%u]", m_StrServerDeadline, m_server_deadline);
//	}
//
//	if ( ul_getconfint( m_pconfdata, m_StrServerPunishMode, &m_server_punish_mode) <=0 )
//	{
//		m_server_punish_mode = true;
//		ROUTN("read [%s] err. use default [false]", m_StrServerPunishMode);
//	}
//	else
//	{
//		ROUTN("[%s] : [%u]", m_StrServerPunishMode, m_server_punish_mode);
//	}
//
//
//	if (( ul_getconfint( m_pconfdata, m_StrCheckInterval, &m_check_interval) <=0 )
//			|| (m_check_interval < 100))
//	{
//		m_check_interval = m_DefaultCheckInterval;
//		ROUTN("read [%s] err. use default [%u]", m_StrCheckInterval, m_DefaultCheckInterval);
//	}
//	else
//	{
//		ROUTN("[%s] : [%u]", m_StrCheckInterval, m_check_interval);
//	}
//
//	if ( ul_getconfstr( m_pconfdata, m_StrResourcePackPath, m_ResourceLoadPath) <=0 )
//	{
//		snprintf(m_ResourceLoadPath, sizeof(m_ResourceLoadPath), "%s", m_DefaultResourcePackPath);
//		ROUTN("read [%s] err. use default [%s]", m_StrResourcePackPath, m_DefaultResourcePackPath);
//	}
//	else
//	{
//		ROUTN("[%s] : [%s]", m_StrResourcePackPath, m_ResourceLoadPath);
//	}
//
//	if ( ul_getconfstr( m_pconfdata, m_StrResourceDumpPath, m_ResourceDumpPath) <=0 )
//	{
//		snprintf(m_ResourceDumpPath, sizeof(m_ResourceDumpPath), "%s", m_DefaultResourceDumpPath);
//		ROUTN("read [%s] err. use default [%s]", m_StrResourceDumpPath, m_DefaultResourceDumpPath);
//	}
//	else
//	{
//		ROUTN("[%s] : [%s]", m_StrResourceDumpPath, m_ResourceDumpPath);
//	}
//
//	if ( ul_getconfint( m_pconfdata, m_StrUseCarpBalance, &m_use_carp_balance) <=0 )
//	{
//		m_use_carp_balance = 1; // default
//	}
//	else
//	{
//		m_use_carp_balance = (m_use_carp_balance != 0);
//	}
//	ROUTN("[%s] : [%d]", m_StrUseCarpBalance, m_use_carp_balance);
//
//	m_connectmap.SetConnectTO    (m_connect_timeout);
//	m_connectmap.SetRetryLine    (m_connect_retry_line);
//	m_connectmap.SetHealthLine   (m_server_health_line);
//	m_connectmap.SetDeadline     (m_server_deadline);
//	m_connectmap.SetCheckInterval(m_check_interval);
//	m_connectmap.SetPunishMode   (m_server_punish_mode != 0);
//
//	if (false == InitLocalResource())
//	{
//		if (! m_use_resource_server)
//		{
//			throw SpaceException("Load local resource failed", "FATAL");
//		}
//		else
//		{
//			ALARM("some err happened when loading local resource, i will continue");
//		}
//	}
//	else
//	{
//		ROUTN("loading local resource OK");
//	}
//
//	if (m_use_resource_server)
//	{
//		if (!ResourceClientInit(configdir, configfile))
//		{
//			FATAL("Init ResourceClient failed");
//			throw SpaceException("Init ResourceClient failed");
//		}
//		if (! RegisterByFile())
//		{
//			FATAL("Register to ResourceServer failed");
//			throw SpaceException("Register to ResourceServer failed");
//		}
//	}
//	MyThrowAssert(1 == ul_freeconf( m_pconfdata ));
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

const char* ConnectManager::ResourceLoadPath()
{
	return m_ResourceLoadPath;
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
		DEBUG("new server, new cell. path[%s] name[%s]", resource["path"].asCString(), resourcename);
		resourcenum++;
	}
	else if (false == bret && cur_ridx == -1)
	{
		ALARM("Resource_info array full. process %d resource, following resource will be droped", cur_ridx);
		return;
	}
	else
	{
		DEBUG("new server, old cell. path[%s] name[%s]", resource["path"].asCString(), resourcename);
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
            DEBUG("repeat resource host[%s] port[%u] path[%s] name[%s]",
                    resource["ip"].asCString(), resource["port"].asInt(),
                    resource["path"].asCString(), resource["name"].asCString());
            break;
        }
        if (resourcelist[cur_ridx].modules[midx].port != 0)
        {
            // this cell is NOT empty
            continue;
        }
        resourcelist[cur_ridx].modules[midx].name[0] = 0;
        const char* modulename = resource["path"].asCString();
        if (0 == modulename[0])
        {
            ALARM("Can't get modulename in resourcepack @ resourcearray[%d]", cur_ridx);
            return;
        }
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

        resourcelist[cur_ridx].modules[midx].priority = 444;
        u_int priority = resource["priority"].asInt();
        priority = priority > PRIORITY_MAXLEVEL-1 ? PRIORITY_MAXLEVEL-1 : priority;
        resourcelist[cur_ridx].modules[midx].priority = priority;

        resourcelist[cur_ridx].modules[midx].longconnect = 444;
        u_int longconnect = resource["longconnect"].asInt();
        longconnect = longconnect > 1 ? 1 : longconnect;
        resourcelist[cur_ridx].modules[midx].longconnect = longconnect;

        resourcelist[cur_ridx].modules[midx].socknum = 31;
        u_int socknum = resource["sockperserver"].asInt();
        socknum = socknum > SOCK_MAXNUM_PER_SERVER ? SOCK_MAXNUM_PER_SERVER : socknum;
        resourcelist[cur_ridx].modules[midx].socknum = socknum;
        DEBUG("Process resource[%s] @ subsystem[%s] OK. "
                "address[%s:%d] priority[%d] longconnect[%d] socknum[%d] cur_ridx[%d] resourcenum[%u]",
                resourcelist[cur_ridx].name,
                resourcelist[cur_ridx].modules[midx].name,
                resourcelist[cur_ridx].modules[midx].host,
                resourcelist[cur_ridx].modules[midx].port,
                resourcelist[cur_ridx].modules[midx].priority,
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

bool ConnectManager::InitLocalResource()
{
    // read the resource information
    memset(m_local_resource_info, 0, sizeof(m_local_resource_info));
    if (0 >= read_file_all(m_ResourceDumpPath, m_tmpbuff, TMPBUFFER_MAXSIZE))
    {
        FATAL("read dump file[%s] fail", m_ResourceDumpPath);
        return false;
    }
    Json::Value resourcelist;
    // TODO

    for (uint32_t i=0; i<resourcelist.size(); i++)
    {
        Json::Value resource = resourcelist[i];
        ProcessResource(resource, m_local_resource_info, RESOURCE_MAXNUM);
    }

    return true;
}

ConnectManager::~ConnectManager()
{
    delete [] m_dumppack;
    delete [] m_tmpbuff;
}

uint32_t ConnectManager::GetServerList(const char* key, module_info_t* server, const uint32_t listsize, 
        int* prinum)
{
    MySuicideAssert (key && server && listsize > 0);
    uint32_t servernum = 0;

    char resourcename[RESOURCE_NAME_MAXLEN];
    memset (resourcename, 0, sizeof(resourcename));
    const char* pslash = strrchr(key, '/');
    if (pslash == NULL)
    {
        ALARM("invalid key[%s]", key);
        return 0;
    }
    strncpy(resourcename, pslash+1, sizeof(resourcename));

    uint32_t localresourcesize = sizeof(m_local_resource_info)/sizeof(m_local_resource_info[0]);
    for (int pri=0; pri < PRIORITY_MAXLEVEL; pri++)
    {
        prinum[pri] = 0;
        for (uint32_t i=0; i<localresourcesize; i++)
        {
            if (0 == m_local_resource_info[i].name[0])
            {
                break;
            }
            if (0 == strcmp(resourcename, m_local_resource_info[i].name))
            {
                for (int j=0; j<m_local_resource_info[i].module_num; j++)
                {
                    if (m_local_resource_info[i].modules[j].priority == pri && servernum < listsize)
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
                        server[servernum].priority    = m_local_resource_info[i].modules[j].priority;
                        snprintf(server[servernum].name, sizeof(server[0].name), "%s", m_local_resource_info[i].modules[j].name);
                        servernum++;
                        prinum[pri] ++;
                    }
                }
            }
        }
    }
    DEBUG("get %u servers. key[%s] pri0[%d] pri1[%d] pri2[%d]",
            servernum, key, prinum[0], prinum[1], prinum[2]);
    return servernum;
}

int ConnectManager::FreeSocket(const int sock, bool errclose)
{
    return m_connectmap.FreeSocket(sock, errclose);
}

int ConnectManager::FetchSocket(const char* key, const char* strbalance)
{
    int sock = 0;
    m_manager_service++;
    u_int servernum;
    module_info_t server[MODULE_MAXNUM_PER_RESOURCE];
    memset (server, 0, sizeof(server));
    int prinum[PRIORITY_MAXLEVEL] = {0};
    servernum = GetServerList(key, server, MODULE_MAXNUM_PER_RESOURCE, prinum);
    if (servernum <= 0)
    {
        ALARM("GetServerList fail servernum[%d]. key[%s]", servernum, key);
        return -1;
    }
    CarpCalculate(server, servernum, strbalance, prinum);
    for (u_int sid=0; sid<servernum; sid++)
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
    for (u_int lucksid=0; lucksid<servernum; lucksid++)
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

int Compare_Carp(const void *a, const void *b)
{
    const module_info_t* pa = (module_info_t*)a;
    const module_info_t* pb = (module_info_t*)b;
    return (pa->carpvalue < pb->carpvalue);
}

void ConnectManager::CarpCalculate(module_info_t* server,
        const u_int servernum, const char* strbalance, int* prinum)
{
    char tmp[MAX_KEY_MERGE_LEN];
    memset(tmp, 0, sizeof(tmp));
    int prioffset[PRIORITY_MAXLEVEL] = {0};
    prioffset[0] = 0;
    prioffset[1] = prioffset[0];
    prioffset[2] = prioffset[0] + prioffset[1];
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
    for (int pri = 0; pri < PRIORITY_MAXLEVEL; pri++)
    {
        if (prinum[pri] == 0)
        {
            continue;
        }
        int myoffset = prioffset[pri];
        qsort(&server[myoffset], prinum[pri], sizeof(module_info_t), Compare_Carp);
    }
    return;
}
