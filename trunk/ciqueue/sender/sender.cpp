#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <json/json.h>
#include <iostream>
#include <fstream>
#include <signal.h>
#include "mylog.h"
#include "MyException.h"
#include "sender.h"

using namespace std;

void* send_message(void*);

void PrintHelp(void)
{
    printf("\nUsage:\n");
    printf("%s <options>\n", PROJNAME);
    printf("  options:\n");
    printf("    -c:  #conf path\n");
    printf("    -v:  #version\n");
    printf("    -h:  #This page\n");
    printf("\n\n");
}

void PrintVersion(void)
{
    printf("Project    :  %s\n", PROJNAME);
    printf("Version    :  %s\n", VERSION);
    printf("BuildDate  :  %s - %s\n", __DATE__, __TIME__);
}

int main(int argc, char** argv)
{
    // 应该做一个排他性检测，通过文件锁的方式应该比较酷
    signal(SIGPIPE, SIG_IGN);
    const char * strConfigPath = "./conf/"PROJNAME".config.json";
    char configPath[128];
    snprintf(configPath, sizeof(configPath), "%s", strConfigPath);

    char c = '\0';
    while ((c = getopt(argc, argv, "c:vh?")) != -1)
    {
        switch (c)
        {
            case 'c':
                snprintf(configPath, sizeof(configPath), "%s", optarg);
                strConfigPath = configPath;
                break;
            case 'v':
                PrintVersion();
                return 0;
            case 'h':
            case '?':
                PrintHelp();
                return 0;
            default:
                break;
        }
    }

    Json::Value root;
    Json::Reader reader;
    ifstream in(strConfigPath);
    if (! reader.parse(in, root))
    {
        FATAL("file[%s] json format error.", strConfigPath);
        MyToolThrow("json format error.");
    }

    char qpath[128];
    char qfile[128];
    snprintf(qpath, sizeof(qpath), "%s", root["qpath"].asCString());
    snprintf(qfile, sizeof(qfile), "%s", root["qfile"].asCString());

    uint32_t    loglevel = root["LogLevel"].asInt();
    const char* logname = root["LogPath"].asCString();
    SETLOG(loglevel, logname);
    ROUTN( "=====================================================================");

    Json::Value follower_array = root["followers"];
    if (0 >= follower_array.size())
    {
        MyThrowAssert("no followers.");
    }
    sender_config_t* psender_list = new sender_config_t[follower_array.size()];
    Json::Value::const_iterator iter;
    iter = follower_array.begin();

    uint32_t follower_real_count = 0;

    set<string> channel_set;

    for (uint32_t i=0; i<follower_array.size(); i++)
    {
        Json::Value mysender = follower_array[i];
        if (0 == mysender["enable"].asInt())
        {
            ROUTN("channel[%s] skip.", mysender["name"].asCString());
            continue;
        }
        psender_list[follower_real_count].enable = mysender["enable"].asInt();
        psender_list[follower_real_count].sender_id = follower_real_count;
        snprintf (psender_list[follower_real_count].qpath,
                sizeof(psender_list[follower_real_count].qpath), "%s", qpath);
        snprintf (psender_list[follower_real_count].qfile,
                sizeof(psender_list[follower_real_count].qfile), "%s", qfile);
        snprintf (psender_list[follower_real_count].channel,
                sizeof(psender_list[follower_real_count].channel),
                "%s", mysender["name"].asCString());
        if (channel_set.end() == channel_set.find(string(psender_list[follower_real_count].channel)))
        {
            channel_set.insert(string((psender_list[follower_real_count].channel)));
        }
        else
        {
            FATAL("dup channel[%s]", psender_list[follower_real_count].channel);
            MyThrowAssert(0);
        }
        snprintf (psender_list[follower_real_count].host,
                sizeof(psender_list[follower_real_count].host),
                "%s", mysender["host"].asCString());
        psender_list[follower_real_count].port = mysender["port"].asInt();
        psender_list[follower_real_count].long_connect = mysender["long_connect"].asInt();
        psender_list[follower_real_count].send_toms = mysender["send_toms"].asInt();
        psender_list[follower_real_count].recv_toms = mysender["recv_toms"].asInt();
        psender_list[follower_real_count].conn_toms = mysender["conn_toms"].asInt();
        ROUTN("consumer[%s] host[%s] port[%d] long_connect[%d] "
                "send_toms[%u] recv_toms[%u] conn_toms[%u]",
                psender_list[follower_real_count].channel,
                psender_list[follower_real_count].host,
                psender_list[follower_real_count].port,
                psender_list[follower_real_count].long_connect,
                psender_list[follower_real_count].send_toms,
                psender_list[follower_real_count].recv_toms,
                psender_list[follower_real_count].conn_toms);

        Json::Value events_list = mysender["events"];
        if (0 >= events_list.size())
        {
            MyThrowAssert("sender has no events.");
        }

        for (uint32_t k=0; k<events_list.size(); k++)
        {
            Json::Value event = events_list[k];
            char event_string[128];
            snprintf(event_string, sizeof(event_string), FORMAT_QUEUE_OP,
                    events_list[k]["name"].asCString(), events_list[k]["operation"].asCString());
            ROUTN("channel[%s] event[%s]", psender_list[follower_real_count].channel, event_string);
            psender_list[follower_real_count].events_set.insert(string(event_string));
        }
        iter++;
        follower_real_count ++;
    }

    pthread_t* thread_id_list = new pthread_t[follower_real_count];
    // 生成发送线程，使用sender_config_t作为配置
    for (uint32_t i=0; i<follower_real_count; i++)
    {
        MyThrowAssert( 0 == pthread_create(&thread_id_list[i],
                    NULL, send_message, &psender_list[i]));
    }

    for (uint32_t i=0; i<follower_real_count; i++)
    {
        pthread_join(thread_id_list[i], NULL);
    }

    delete [] psender_list;
    delete [] thread_id_list;
}
