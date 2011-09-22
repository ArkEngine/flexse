#ifndef _CONNECTMANAGE_H_
#define _CONNECTMANAGE_H_
#include <pthread.h>
#include <string>
#include <map>
#include <json/json.h>
#include "creat_sign.h"
#include "connectmap.h"
#include "MyException.h"
using namespace std;

#define FILE_PATH_MAXLEN (128)
#define MAX_KEY_MERGE_LEN (128)
#define RESOURCE_MAXNUM (128)
#define RESOURCE_NAME_MAXLEN (128)
#define MODULE_MAXNUM_PER_RESOURCE (32)
#define REGISTERMCPACK_MAXLEN (64*1024)
#define TMPBUFFER_MAXSIZE     (1024*1024)

typedef struct __resource_info_t
{
	char name[RESOURCE_NAME_MAXLEN];
	int  module_num;
	module_info_t modules[MODULE_MAXNUM_PER_RESOURCE];
}resource_info_t;

class ConnectManager;
typedef struct __resource_callback_t
{
	resource_info_t* presource_info;
	u_int maxnum;
	u_int processnum;
	ConnectManager* cmanager;
}resource_callback_t;

class ConnectManager
{
	ConnectMap m_connectmap;
	resource_info_t m_local_resource_info [RESOURCE_MAXNUM];
	char m_register_text[REGISTERMCPACK_MAXLEN];
	// 存储text2mcpack结果
	char m_resource_pack[REGISTERMCPACK_MAXLEN];
	// 存储重新set_port后的mcpack
	char m_register_pack[REGISTERMCPACK_MAXLEN];
	char m_ResourceLoadPath[FILE_PATH_MAXLEN];
	// 存储上次最新的资源信息
	char m_ResourceDumpPath[FILE_PATH_MAXLEN];
	uint32_t m_manager_fail;
	uint32_t m_randseed;
	uint32_t m_manager_success;
	uint32_t m_manager_service;
	char* m_tmpbuff;
	char* m_dumppack;

	static const uint32_t    ConfigMaxNum              = 1024;
	static const uint32_t    m_DefaultConnectTimeOut   = 100;
	static const uint32_t    m_DefaultServerHealthLine = 10;
	static const uint32_t    m_DefaultRetryLine        = 100;
	static const uint32_t    m_DefaultServerDeadline   = 3600;
	static const uint32_t    m_DefaultCheckInterval    = 3600;
	static const char* const m_StrConnectTimeOut;
	static const char* const m_StrServerHealthLine;
	static const char* const m_StrRetryLine;
	static const char* const m_StrServerDeadline;
	static const char* const m_StrCheckInterval;
	static const char* const m_StrUseResourceServer;
	static const char* const m_StrUseCarpBalance;
	static const char* const m_StrResourcePackPath;
	static const char* const m_DefaultResourcePackPath;
	static const char* const m_StrResourceDumpPath;
	static const char* const m_DefaultResourceDumpPath;
	static const char* const m_StrQueryListenPort;
	static const char* const m_StrServerPunishMode;
	int m_connect_timeout;
	int m_server_health_line;
	int m_connect_retry_line;
	int m_server_deadline;
	int m_check_interval;
	int m_query_listen_port;
	int m_server_punish_mode;
	int m_first_run;

	int m_use_resource_server;
	int m_use_carp_balance;

	ConnectManager(const ConnectManager&);
	ConnectManager();

	public:
	ConnectManager(const char* config_path);
	~ConnectManager();

	uint32_t FailCount();
	uint32_t ServiceCount();
	uint32_t ServerSockFullCount();
	uint32_t ServerErrConnectCount();
	const char* ResourceLoadPath();

	/**
	 * @brief   fetch sock by key and balance
	 *
	 * param [in] key : const char* - Comlogic's key[sublogicname] - Sublogic's key[resourcename]
	 * param [in] balance : const int -- carp or random(-1)
	 * @return  int
	 * @retval  retval
	 *          >0 : success
	 *         <=0 : fail
	 **/
	int FetchSocket (const char* key, const char* balance);

	/**
	 * @brief   free sock to ConnectManager
	 *
	 * param [in] sock : const int -- the sock to free
	 * param [in] errclose : bool -- is close for error?
	 * @return  int
	 * @retval  retval
	 *           0 : success
	 *          <0 : fail
	 **/
	int FreeSocket  (const int sock, bool errclose);

	private:

	void ProcessResource(Json::Value resource, resource_info_t* resourcelist, const int listsize);
	/*
	 * @brief this is for local
	 */
	bool InitLocalResource();

	/*
	 * @brief: find if resourcename is in resource_info array
	 * param [in]  resourcename: the key
	 * param [out] presource_info: resource_info array
	 * param [in]  size: size of resource_info
	 * param [out]  idx: index of resource_info
	 * retval bool:
	 * true  find the resourcename in resource_info array
	 * false no such resource. and idx set to -1 if no more space, idx set idx to first empty cell
	 */
	bool find_idx(const char* resourcename, const resource_info_t* presource_info,
			const u_int array_size, int& ridx);

	/**
	 * @brief   Get server list
	 *
	 * param [in]  key : const char* -- sublogicname or resourcename
	 * param [in]  priority :  int   -- priority
	 * param [out] server  :   server_list_t  -- serverlist for return
	 * @return  int
	 * @retval  u_int
	 *       >=0 : servernum
	 *       <0  : error happen
	 **/
	uint32_t  GetServerList(const char* key, module_info_t* server, const uint32_t listsize, int* prinum);

	/**
	 * @brief   Calculate the CARP value
	 *
	 * param [in]  server    : server_list_t* -- server array
	 * param [in]  servernum : const int   -- servernum
	 * param [in]  balance   : const int   -- balanceKey
	 * param [out] carpidx   : int*        -- server index array sort by carp-value
	 * @return  void
	 **/
	void CarpCalculate(module_info_t* server, const u_int servernum, const char* balance, int* prinum);
};

#endif
