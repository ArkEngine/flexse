#include <stdint.h>
#include <set>
#include <string>
using namespace std;

#define FORMAT_QUEUE_OP "%s # %s"

enum
{
    OK = 0,
    ROLL_BACK = 1,
};

struct sender_config_t
{
	uint32_t sender_id;
	char     channel[32];    ///> 消息接受者的名字，它的进度文件为 $channel.offset
	char     host[32];       ///> 消息接受者ip
	uint32_t port;           ///> 消息接受者port
	uint32_t long_connect;   ///> 是否长链接
	uint32_t send_toms;      ///> socket的写入超时 单位: ms
	uint32_t recv_toms;      ///> socket的读取超时 单位: ms
	uint32_t conn_toms;      ///> socket的连接超时 单位: ms
	uint32_t enable;         ///> 是否开启

	set<string> events_set;  ///> 监听的事件列表
	char     qpath[128];     ///> 消息队列的路径 ./data/$qpath/
	char     qfile[128];     ///> 消息队列的文件名, $file.n, n为整数
};
