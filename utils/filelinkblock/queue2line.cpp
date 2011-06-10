#include <stdio.h>
#include <string>
#include <vector>
#include "FileOutputer.h"
#include "Log.h"
#include "FileLinkBlock.h"

using namespace std;

// 用于存放输出字段配置的数组
vector< vector<string> >  g_key_list;

void print_help(void)
{               
	printf("\nUsage:\n");
	printf("%s <options>\n", PROJNAME);
	printf("  options:\n");
	printf("    -p:  #queue path\n");
	printf("    -n:  #queue name\n");
	printf("    -b:  #queue block_id\n");
	printf("    -f:  #queue file no\n");
	printf("    -c:  #read block count\n");
	printf("    -v:  #Print version information\n");
	printf("    -h:  #This page\n");
	printf("\n\n");
}               

void print_version(void)
{
	printf("Project    :  %s\n", PROJNAME);
	printf("Version    :  %s\n", VERSION);
	printf("Cvstag     :  %s\n", CVSTAG);
	printf("Cvspath    :  %s\n", CVSPATH);
	printf("BuildDate  :  %s\n", __DATE__);
}

void queue2line(FileLinkBlock& flb, FileOutputer& fop, char* readbuff, char* tempbuff, uint32_t SIZE)
{
	uint32_t log_id   = 0;
	uint32_t block_id = 0;
	bsl::ResourcePool rp;
	uint32_t read_size = flb.read_message(log_id, block_id, readbuff, SIZE);
	assert (0 < read_size);
	//deserialize mcpack to IVar
	bsl::var::McPackDeserializer mpd(rp);
	bsl::var::IVar& myVar = mpd.deserialize(readbuff, read_size);
	fprintf(stdout, "========== log_id : %u block_id : %u ==========\n", log_id, block_id);
	bsl::var::print(myVar, 10);
	fflush(stdout);

	uint32_t       slen = 0;
	for (uint32_t ki=0; ki<g_key_list.size(); ki++)
	{
		bsl::var::IVar* myDict = &myVar;
		const uint32_t path_len = g_key_list[ki].size();
		for (uint32_t ip=0; ip<(path_len-1); ip++)
		{
			if ((*myDict)[g_key_list[ki][ip].c_str()].is_null())
			{
				WARNING("log_id[%u] item[%u] can't find the path key [%s]", log_id, ip, g_key_list[ki][ip].c_str());
				break;
			}
			myDict = &((*myDict)[g_key_list[ki][ip].c_str()]);
			if (! (*myDict).is_dict())
			{
				WARNING("log_id[%u] item[%u] [%s] is NOT dict", log_id, ip, g_key_list[ki][ip].c_str());
				break;
			}
		}
		char key[128];
		snprintf ( key, sizeof(key), "%s", g_key_list[ki][path_len - 1].c_str());

		if ((*myDict)[key].is_null())
		{
			slen += snprintf(&tempbuff[slen], SIZE-slen, "%s:[%s]\t", key, "");
		}
		else if ((*myDict)[key].is_string())
		{
			slen += snprintf(&tempbuff[slen], SIZE-slen, "%s:[%s]\t",   key, (*myDict)[key].c_str());
		}
		else if ((*myDict)[key].is_double())
		{
			slen += snprintf(&tempbuff[slen], SIZE-slen, "%s:[%f]\t",   key, (*myDict)[key].to_double());
		}
		else if ((*myDict)[key].is_number())
		{
			slen += snprintf(&tempbuff[slen], SIZE-slen, "%s:[%llu]\t", key, (*myDict)[key].to_uint64());
		}
		else
		{
			slen += snprintf(&tempbuff[slen], SIZE-slen, "%s:[%s]\t", key, "");
			WARNING("NOT SUPPORTED TYPE. key[%s]", key);
		}
	}
	tempbuff[slen - 1] = '\n';
	tempbuff[slen] = '\0';
	printf("########%s#########", tempbuff);
	fop.WriteNByte(tempbuff, slen);
}

int main()
{
	comcfg::Configure conf;
	try {
		// 日志配置
		int ret = 0;
		// 读取配置
		if(0 != (ret = conf.load("./conf/", "queue2line.conf", NULL)))
		{
			FATAL ("load ./conf/queue2line.conf FAIL. ret[%d]", ret);
			return -1;
		}
		if (0 != ub_init_comlog(conf["LOG"]))
		{
			FATAL ("INIT LOG FAIL.");
			return -1;
		}

		for (uint32_t i=0; i<conf["QUEUE2LINE"]["KEY_LIST"].size(); i++)
		{
			vector<string> keypath_list;
			const char* pstr = conf["QUEUE2LINE"]["KEY_LIST"][i].to_cstr();
			const char* pbgn = pstr;
			const char* pcur = NULL;
			const char  spliter = '/';
			char  tmpstr[128];
			memset (tmpstr, 0, sizeof(tmpstr));
			while (NULL != (pcur = strchr(pbgn, spliter)))
			{
				memcpy(tmpstr, pbgn, pcur-pbgn);
				tmpstr[pcur-pbgn + 1] = 0;
				pbgn = pcur + 1;
				TRACE("path[%s] orig[%s]", tmpstr, pstr);
				keypath_list.push_back(tmpstr);
			}
			keypath_list.push_back(pbgn);
			g_key_list.push_back(keypath_list);
		}
		if (g_key_list.size() == 0)
		{
			WARNING("key size == 0. ni bu pei zhi yi ge zi duan, rang wo shu chu mao?!");
			return -1;
		}
	}
	catch(...)
	{
		WARNING("caught exception. load uniq.conf ERROR.");
		return -1;
	}

	// 设置输出的文件
	FileOutputer  fop(conf["QUEUE2LINE"]["OUTFILE"].to_cstr());
	FileLinkBlock flb(conf["QUEUE2LINE"]["PATH"].to_cstr(), conf["QUEUE2LINE"]["NAME"].to_cstr());
	flb.set_channel(conf["QUEUE2LINE"]["OFFSET"].to_cstr());
	uint32_t fno = 0;
	uint32_t off = 0;
	uint32_t bid = 0;
	flb.load_offset(fno, off, bid);
	flb.seek_message(fno, bid);
	const uint32_t SIZE = 4*1024*1024;
	char*      readbuff = (char*)malloc(SIZE);
	char*      tempbuff = (char*)malloc(SIZE);
	// tail -f 模式
	while( 1 )
	{
		queue2line(flb, fop, readbuff, tempbuff, SIZE);
//		flb.log_offset();
	}
	free(readbuff);
	free(tempbuff);

	return 0;
}
