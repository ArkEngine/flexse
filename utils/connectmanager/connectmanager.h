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
	uint32_t m_manager_fail;
	uint32_t m_randseed;
	uint32_t m_manager_success;
	uint32_t m_manager_service;

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
	static const char* const m_StrUseCarpBalance;
	static const char* const m_StrServerPunishMode;
    static const char* const m_StrServerConfigList;
	uint32_t m_connect_timeout;
	uint32_t m_server_health_line;
	uint32_t m_connect_retry_line;
	uint32_t m_server_deadline;
	uint32_t m_check_interval;
	uint32_t m_server_punish_mode;
	uint32_t m_use_carp_balance;

	ConnectManager(const ConnectManager&);
	ConnectManager();

	public:
	ConnectManager(Json::Value config_json);
	~ConnectManager();

	uint32_t FailCount();
	uint32_t ServiceCount();
	uint32_t ServerSockFullCount();
	uint32_t ServerErrConnectCount();

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
	 * param [out] server  :   server_list_t  -- serverlist for return
	 * @return  int
	 * @retval  u_int
	 *       >=0 : servernum
	 *       <0  : error happen
	 **/
	uint32_t  GetServerList(const char* key, module_info_t* server, const uint32_t listsize);

	/**
	 * @brief   Calculate the CARP value
	 *
	 * param [in]  server    : server_list_t* -- server array
	 * param [in]  servernum : const int   -- servernum
	 * param [in]  balance   : const int   -- balanceKey
	 * param [out] carpidx   : int*        -- server index array sort by carp-value
	 * @return  void
	 **/
	void CarpCalculate(module_info_t* server, const uint32_t servernum, const char* balance);
};

#endif
