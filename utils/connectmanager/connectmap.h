#ifndef _CONNECTMAP_H_
#define _CONNECTMAP_H_

#include <string>
#include <map>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include "mylog.h"
#define SOCK_MAXNUM_PER_SERVER (64)
#define MODULE_NAME_MAXLEN (128)
#define MODULE_ADDRESS_MAXLEN (128)
using namespace std;

typedef struct __module_info_t
{
	char     name[MODULE_NAME_MAXLEN];
	char     host[MODULE_ADDRESS_MAXLEN];
	uint16_t port;
	uint32_t longconnect;
	uint32_t socknum;
	uint32_t carpvalue;
}module_info_t;
enum error_code
{
	ECONNECT_SUCCESS = 0,
	ECONNECT_SRVSOCKFULL = -1,
	ECONNECT_SRVDOWN = -2,
	ECONNECT_SRVCONCT = -3,
	ECONNECT_PARAM = -4,
	ECONNECT_SRVNOTEXIST = -5,
	ECONNECT_SRVSTATUS = -6,
	ECONNECT_GENERAL = -7,
};

class ConnectMap
{
	private:
		uint32_t m_health_line;
		uint32_t m_retry_line;
		uint32_t m_check_interval;
		uint32_t m_last_check;
		uint32_t m_server_deadline;
		uint32_t m_connectto_ms;
		bool m_punish_mode;
		uint32_t m_err_server_miss;
		uint32_t m_err_server_conn;
		uint32_t m_err_server_full;
		static const uint32_t DFT_CONNECT_TIMEOUT = 100;
		static const uint32_t DFT_HEALTH_LINE = 10;
		static const uint32_t DFT_RETRY_LINE = 100;
		static const uint32_t DFT_CHECK_INTERVAL = 3600;
		static const uint32_t DFT_SERVER_DEADLINE = 3600;
		static const uint32_t MAGIC_NUM = 0x4A3B2C1D;

		pthread_mutex_t m_mutex;

		enum sock_status
		{
			SOCK_EMPTY,
			SOCK_BUSY,
			SOCK_READY,
		};
		enum server_status
		{
			SERVER_DELETE,
			SERVER_BUSY,
		};

		struct sock_info_t {
			int sock;
			sock_status status;
		};

		struct connect_info_t
		{
			server_status status;
			time_t timestamp;
			module_info_t module;
			uint32_t  fail_count;
			uint32_t  freenum; // READY和EMPTY状态的cell数目
			uint32_t  service; // 服务次数
			sock_info_t  sockarr[SOCK_MAXNUM_PER_SERVER];
		};
		map <string, connect_info_t> m_connectmap;
		map <string, connect_info_t>::iterator  m_icm;

		ConnectMap(const ConnectMap&);

		/**
		 * @brief   check if the sock is ok
		 *
		 * param [in] sock : const int   -- sock to check
		 * @return  int
		 * @retval  retval
		 *  =0 : success
		 *  <0 : fail
		 **/
		int __CheckConnection(const int sock);

		/**
		 * @brief   connect server to get a sock
		 *
		 * param [in && out] connect_info_t : connect_info_t& -- connect_info
		 * param [in]        idx            : const int       -- index of sockarr inside
		 * @return  int
		 * @retval  retval
		 *  =0 : success
		 *  <0 : fail
		 **/
		int __ConnectServer(connect_info_t& connect_info, const int idx);

		/**
		 * @brief   fetch sock by hostip and port -- no mutex
		 *
		 * param [in] host    : const char* -- server's IP  
		 * param [in] port    : const int   -- server's port
		 * @return  int
		 * @retval  retval
		 *  >=0 : success
		 *   <0 : fail
		 **/
		int __FetchSocket(const char* host, const int port);

		/**
		 * @brief   update server info
		 *
		 * param [in] host    : connect_info_t&       -- server's info
		 * param [in] port    : const module_info_t&  -- src module
		 * @return  void
		 * @retval  void
		 **/
		void __UpdateServerInfo(connect_info_t& connect_info, const module_info_t& src_module);

		/*
		 * @brief check the dead server and close the sock;
		 *
		 * if the server idle too long, close all ready sock and delete it
		 * note: if one server is check&delete, return, never try next one.
		 * in a short, 一次检查，最多关闭一个 server
		 * @return  void
		 * @retval  void
		 */
		void __CheckDeadServer();

	public:

		static const uint32_t RECONNECT_BOUNDRY = 0x0000007F;

		ConnectMap();
		~ConnectMap();

		uint32_t ServerSockFullCount();
		uint32_t ServerErrConnectCount();

		/**
		 * @brief   set the timeout of connectmap
		 *
		 * param [in] ctimeout : const int -- ctimeout, ms
		 * @return  void
		 * @retval  void
		 **/
		void SetConnectTO    (const uint32_t ctimeout);
		void SetRetryLine    (const uint32_t rline);
		void SetHealthLine   (const uint32_t hline);
		void SetDeadline     (const uint32_t dline);
		void SetCheckInterval(const uint32_t checkinterval);
		void SetPunishMode(bool mode); // 当因为错误而 FreeSocket 时，惩罚这个server

		/**
		 * @brief   fetch sock by module_info_t
		 *
		 * param [in] moduinfo: const module_info_t   -- module_info
		 * if module_info change, update it
		 * if no module exist, return, NOT add
		 * @return  int
		 * @retval  retval
		 *  >=0 : success
		 *   <0 : fail
		 **/
		int FetchSocket(const module_info_t& module_info, bool healthcheck = true);

		/**
		 * @brief   free sock
		 *
		 * param [in] sock     : const int -- sock to free
		 * param [in] errclose : bool      -- is close by error?
		 * @return  int
		 * @retval  retval
		 *   =0 : success
		 *   <0 : fail
		 **/
		int FreeSocket (const int sock, bool errclose);

		/**
		 * @brief   add server in connect-map no mutex
		 *
		 * param [in] module : const module_info_t -- module to add
		 * @return  int
		 * @retval  retval
		 *   =0 : success
		 *   <0 : fail
		 **/
		int __AddServer  (const module_info_t& module);

		/**
		 * @brief   add server in connect-map
		 *
		 * param [in] module : const module_info_t -- module to add
		 * @return  int
		 * @retval  retval
		 *   =0 : success
		 *   <0 : fail
		 **/
		int AddServer  (const module_info_t& module);

		/**
		 * @brief   del server in connect-map
		 *
		 * param [in] host : const char* -- server's IP  
		 * param [in] port : const int   -- server's port
		 * @return  int
		 * @retval  retval
		 *   =0 : success
		 *   <0 : fail
		 **/
		int DelServer  (const char* host, const int port);
};
#endif
