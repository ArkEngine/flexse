#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <json/json.h>
#include "sender.h"
#include "filelinkblock.h"
#include "MyException.h"
#include "xhead.h"
#include "mylog.h"

int myconnect(const sender_config_t* psender_config)
{
	struct sockaddr_in adr_srvr;  /* AF_INET */
	int len_inet = sizeof (adr_srvr);
	memset (&adr_srvr, 0, len_inet);
	adr_srvr.sin_family = AF_INET;
	adr_srvr.sin_port = htons (psender_config->port);
	adr_srvr.sin_addr.s_addr = inet_addr (psender_config->host);

	if (adr_srvr.sin_addr.s_addr == INADDR_NONE)
	{
		ALARM("inet_addr[%s:%u] channel[%s] error [%m]",
				psender_config->host, psender_config->port,
				psender_config->channel);
		return -1;
	}

	int sockfd = socket (PF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		ALARM("socket()[%s:%s] channel[%s] error [%m]",
				psender_config->host, psender_config->port,
				psender_config->channel);
		return -1;
	}

	struct timeval  timeout = {0, 0};
	timeout.tv_sec  = psender_config->conn_toms/1000;
	timeout.tv_usec = 1000*(psender_config->conn_toms%1000);
	setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeval));

	int err = connect (sockfd, (struct sockaddr *) &adr_srvr, len_inet);
	if (err == -1)
	{
		ALARM("connent() [%s:%u] channel[%s] error [%m]",
				psender_config->host, psender_config->port,
				psender_config->channel);
        close(sockfd);
		return -1;
	}
	return sockfd;
}

void* send_message(void* arg)
{
	sender_config_t* psender_config = (sender_config_t*) arg;
	filelinkblock myflb(psender_config->qpath, psender_config->qfile, true);
	char channel[128];
	snprintf(channel, sizeof(channel), "%s.offset", psender_config->channel);
	myflb.set_channel(channel);
//	myflb.seek_message();
	const uint32_t READ_BUFF_SIZE = 10*1024*1024;
	xhead_t* sxhead = (xhead_t*) malloc(READ_BUFF_SIZE + sizeof(xhead_t));
	char*    msgbuf = (char*)    malloc(READ_BUFF_SIZE);
	MyThrowAssert(sxhead != NULL && msgbuf != NULL);
	snprintf(sxhead->srvname, sizeof(sxhead->srvname), "%s", PROJNAME);
	xhead_t  rxhead;

	char all_event_string[128];
	snprintf(all_event_string, sizeof(all_event_string), FORMAT_QUEUE_OP,
			"all", "all");

	int sock = -1;

	while(1)
	{
		// read disk message
		uint32_t file_no  = 0;
		uint32_t block_id = 0;
		uint32_t log_id   = 0;
		uint32_t message_len = myflb.read_message(log_id, file_no, block_id,
				msgbuf, READ_BUFF_SIZE);
		msgbuf[message_len] = '\0';

		// 检查是否在监听的事件中
		Json::Value root;
		Json::Reader reader;

		if (! reader.parse(msgbuf, root))
		{
			ALARM("json[%s] format error.", msgbuf);
			continue;
		}

		char event_string[128];
		snprintf(event_string, sizeof(event_string), FORMAT_QUEUE_OP,
				root["__QUEUE_NAME__"].asCString(), root["__OPERATION__"].asCString());
		if ((psender_config->events_set.end() ==
					psender_config->events_set.find(string(event_string)))
			&& (psender_config->events_set.end() ==
					psender_config->events_set.find(string(all_event_string))))
		{
			// 忽略既不是指定的消息也不是all all的消息
			DEBUG("channel[%s]: event[%s] is NOT in mylist",
					psender_config->channel, event_string);
			myflb.save_offset();
			continue;
		}

        Json::FastWriter fwriter;
        string msgbody = fwriter.write(root["__OPERATION_BODY__"]);
        char* sendbody = (char*)(&sxhead[1]);
        uint32_t bodysize = msgbody.size();
        memmove(sendbody, msgbody.c_str(), bodysize);
        if (bodysize <= 0)
        {
			continue;
        }

        sxhead->log_id = log_id;
        sxhead->version = file_no;
        sxhead->reserved = block_id;
        sxhead->detail_len = bodysize;

        while(1)
        {
            if (sock == -1)
            {
                sock = myconnect(psender_config);
                if (sock == -1)
                {
                    sleep(1);
                    continue;
                }
            }

            if (0 != xsend(sock, sxhead, psender_config->send_toms))
            {
                ALARM("xsend error log_id[%u] file_no[%u] block_id[%u] "
                        "channel[%s] event[%s] msglen[%u] e[%m]",
                        log_id, file_no, block_id,
                        psender_config->channel, event_string, message_len);
                close(sock);
                sock = -1;
                // 防止日志打太多
                sleep(1);
                continue;
            }
            if (0 != xrecv(sock, &rxhead, sizeof(xhead_t), psender_config->recv_toms))
            {
                // 对方没返回，应该重试
                ALARM("xrecv error log_id[%u] file_no[%u] block_id[%u] "
                        "channel[%s] event[%s] msglen[%u] e[%m]",
                        log_id, file_no, block_id,
                        psender_config->channel, event_string, message_len);
                close(sock);
                sock = -1;
                // 防止日志打太多
                sleep(1);
                continue;
            }
            else
            {
                if (rxhead.status == OK)
                {
                    // 成功了，保存进度
                    myflb.save_offset();
                    ROUTN("logid[%u] file_no[%u] block_id[%u] channel[%s] event[%s] msglen[%u]",
                            log_id, file_no, block_id, psender_config->channel,
                            event_string, message_len);
                    if (! psender_config->long_connect)
                    {
                        close(sock);
                        sock = -1;
                    }
                    break;
                }
                else if (rxhead.status == ROLL_BACK)
                {
                    // 对方要求回滚
                    myflb.seek_message(rxhead.file_no, rxhead.block_id);
                    ROUTN("channel[%s] roll back to file_no[%u] block_id[%u]",
                            psender_config->channel, rxhead.file_no, rxhead.block_id);
                    break;
                }
                else
                {
                    ALARM("remote server error log_id[%u] status[%u] errno[%u] file_no[%u] block_id[%u] "
                            "channel[%s] event[%s] msglen[%u] e[%m]",
                            log_id, rxhead.status, rxhead.reserved, file_no, block_id,
                            psender_config->channel, event_string, message_len);
                    // 防止日志打太多
                    sleep(1);
                    continue;
                }
            }
        }
    }
}
