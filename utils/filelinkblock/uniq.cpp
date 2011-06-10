#include <stdio.h>
#include <string>
#include <stdlib.h>
#include "Log.h"
#include "com_log.h"
#include "uniq.h"
#include "nshead.h"
#include "Configure.h"
#include "ul_sign.h"

uniq_handle::uniq_handle()
{
}

uniq_handle::~uniq_handle()
{
}

int uniq_handle::init(const uint32_t thread_num)
{
	comcfg::Configure comconf;
	int ret = 0;
	if(0 != (ret = comconf.load("./conf/", "strategy-framework.conf", NULL)))
	{
		FATAL ("load ./conf/strategy-framework.conf FAIL. ret[%d]", ret);
		return -1;
	}
	if (0 != ub_init_comlog(comconf["comlog"]))
	{
		FATAL ("INIT LOG FAIL.");
		return -1;
	}

	repeat_count = 0;
	passby_count = 0;
	uint32_t hash_num = 0;
	uint32_t node_num = 0;

	try {
		// 读取去重字段的配置
		comcfg::Configure uniqconf;
		if(0 != (ret = uniqconf.load("./conf/", "uniq.conf", NULL)))
		{
			FATAL ("load ./conf/uniq.conf FAIL. ret[%d]", ret);
			return -1;
		}

		for (uint32_t i=0; i<uniqconf["UNIQ"]["KEY_LIST"].size(); i++)
		{
			vector<string> keypath_list;
			const char* pstr = uniqconf["UNIQ"]["KEY_LIST"][i].to_cstr();
			const char* pbgn = pstr;
			const char* pcur = NULL;
			const char  spliter = '/';
			char  tmpstr[128];
			while (NULL != (pcur = strchr(pbgn, spliter)))
			{
				memcpy(tmpstr, pbgn, pcur-pbgn);
				tmpstr[pcur-pbgn + 1] = 0;
				pbgn = pcur + 1;
				TRACE("path[%s]", tmpstr);
				keypath_list.push_back(tmpstr);
			}
			keypath_list.push_back(pbgn);
			m_cache_key_list.push_back(keypath_list);
		}
		if (m_cache_key_list.size() == 0)
		{
			WARNING("key size == 0. ni bu pei zhi yi ge zi duan, rang wo qu mao chong?!");
			return -1;
		}
		// 读取去重字典的配置
		hash_num = uniqconf["UNIQ"]["HASH_NUM"].to_uint32();
		node_num = uniqconf["UNIQ"]["NODE_NUM"].to_uint32();
		hash_num = (hash_num == 0) ? 10000  : hash_num;
		node_num = (node_num == 0) ? 100000 : node_num;

		snprintf(m_dict_path, sizeof(m_dict_path), "%s", uniqconf["UNIQ"]["PATH"].to_cstr());
		snprintf(m_dict_name, sizeof(m_dict_name), "%s", uniqconf["UNIQ"]["NAME"].to_cstr());
	}
	catch(...)
	{
		WARNING("caught exception. load uniq.conf ERROR.");
		return -1;
	}

	pthread_mutex_init(&m_mfifo_mutex, NULL);
	int load_flag = 1;
	if (NULL == (m_mfdict = mfdict_load(m_dict_path, m_dict_name)))
	{
		load_flag = 0;
		WARNING("mfdict_load FAIL, try create a new one. path[%s] name[%s]", m_dict_path, m_dict_name);
		m_mfdict = mfdict_creat(hash_num, node_num);
		if (m_mfdict == NULL)
		{
			WARNING("mfdict_creat FAIL!");
			return -1;
		}
	}
	NOTICE("plugin[%s] load_flag[%u] path[%s] name[%s] "
			"hash_num[%u] node_num[%u] key_num[%lu] I'm init OK",
			NAME, load_flag, m_dict_path, m_dict_name,
			hash_num, node_num, m_cache_key_list.size());
	return 0;
}

int uniq_handle::process(thread_context* pcontext)
{
	nshead_t* nshead = (nshead_t*) pcontext->get_buff();
	const char* STR_RESULT_ARRAY = "result_array";
	bsl::var::MagicDict& mydict = *(pcontext->get_magic_dict());
	if (mydict[STR_RESULT_ARRAY].is_null() || (! mydict[STR_RESULT_ARRAY].is_array()))
	{
		WARNING("log_id[%u] can't find array field '%s' OR wrong type", nshead->log_id, STR_RESULT_ARRAY);
		return 0;
	}
	bsl::var::IVar& result_array = mydict[STR_RESULT_ARRAY];
	printf("###logid[%u] localDict begin###\n", nshead->log_id);
	bsl::var::print(result_array, 10);
	printf("###logid[%u] localDict   end###\n", nshead->log_id);
	// 强制不去重
	// 某些情况，比如去重的配置错误，我们就不去重了。
	for (uint32_t i=0; i<result_array.size(); i++)
	{
		//		printf("##################index[%u]########################\n", i);
		bool isForceSkip = false;
		bsl::var::IVar& localDict = result_array[i];
		if (! localDict.is_dict())
		{
			// 不去重
			WARNING("log_id[%u] item[%u] is NOT dict", nshead->log_id, i);
			continue;
		}
		const uint32_t SIZE = 1000000;
		char           temp_buffer[SIZE];
		uint32_t       slen = 0;
		for (uint32_t ki=0; ki<m_cache_key_list.size(); ki++)
		{
			bsl::var::IVar* myDict = &localDict;
			//			printf("---logid[%u] localDict begin---\n", nshead->log_id);
			//			bsl::var::print(*myDict, 10);
			//			printf("---logid[%u] localDict   end---\n", nshead->log_id);
			const uint32_t path_len = m_cache_key_list[ki].size();
			for (uint32_t ip=0; ip<(path_len-1); ip++)
			{
				if ((*myDict)[m_cache_key_list[ki][ip].c_str()].is_null())
				{
					//					printf("---ERROR logid[%u] path[%s] ip[%u] localDict begin---\n", nshead->log_id, m_cache_key_list[ki][ip].c_str(), ip);
					//					bsl::var::print(localDict, 10);
					//					printf("---ERROR logid[%u] path[%s] ip[%u] localDict   end---\n", nshead->log_id, m_cache_key_list[ki][ip].c_str(), ip);
					WARNING("log_id[%u] item[%u] can't find the path key [%s]", nshead->log_id, i, m_cache_key_list[ki][ip].c_str());
					isForceSkip = true;
					break;
				}
				myDict = &((*myDict)[m_cache_key_list[ki][ip].c_str()]);
				if (! (*myDict).is_dict())
				{
					WARNING("log_id[%u] item[%u] [%s] is NOT dict", nshead->log_id, i, m_cache_key_list[ki][ip].c_str());
					isForceSkip = true;
					break;
				}
			}
			if (isForceSkip)
			{
				continue;
			}
			char key[128];
			snprintf ( key, sizeof(key), "%s", m_cache_key_list[ki][path_len - 1].c_str());
			//			printf("******************key[%s]************************\n", key);
			//			printf("---logid[%u] myDict begin---\n", nshead->log_id);
			//			bsl::var::print(*myDict, 10);
			//			printf("---logid[%u] myDict   end---\n", nshead->log_id);
			if ((*myDict)[key].is_null())
			{
				slen += snprintf(&temp_buffer[slen], SIZE-slen, "%s:0 ", key);
				TRACE ("NOT FOUND. key[%s] using default[0]", key);
				//				printf ("NOT FOUND. key[%s] using default[0]\n", key);
			}
			else if ((*myDict)[key].is_string())
			{
				slen += snprintf(&temp_buffer[slen], SIZE-slen, "%s:%s ",   key, (*myDict)[key].c_str());
			}
			else if ((*myDict)[key].is_double())
			{
				slen += snprintf(&temp_buffer[slen], SIZE-slen, "%s:%f ",   key, (*myDict)[key].to_double());
			}
			else if ((*myDict)[key].is_number())
			{
				slen += snprintf(&temp_buffer[slen], SIZE-slen, "%s:%llu ", key, (*myDict)[key].to_uint64());
			}
			else
			{
				WARNING("NOT SUPPORTED TYPE. key[%s]", key);
			}
		}
		mfdict_snode_t snode;
		creat_sign_fs64(temp_buffer, slen, &(snode.sign1), &(snode.sign2));

		pthread_mutex_lock(&m_mfifo_mutex);
		int sret = mfdict_seek(m_mfdict, &snode);
		if(sret != 0)
		{
			int aret = mfdict_add(m_mfdict, &snode);
			passby_count ++;
			if(aret != 0)
			{
				WARNING("mfdict_add failed ret[%d]", aret);
			}
		}
		else
		{
			// 找到了，就从数组中删除
			result_array.del(i);
			repeat_count ++;
		}
		pthread_mutex_unlock(&m_mfifo_mutex);
		TRACE("repeat[%d] repeat_count[%u] passby_count[%u] index[%3u]"
				" slen[%u] SIZE[%u] k-v string[%s]",
				(int)(sret == 0), repeat_count, passby_count, i,
				slen, SIZE, temp_buffer);
	}
	printf("---logid[%u] localDict begin---\n", nshead->log_id);
	bsl::var::print(result_array, 10);
	printf("---logid[%u] localDict   end---\n", nshead->log_id);
	return 0;
}

int uniq_handle::finish()
{
	int ret = mfdict_save(m_mfdict, m_dict_path, m_dict_name);
	if(ret!=0)
	{
		WARNING("saver dict failed. ret[%d]", ret);
		return -1;
	}
	mfdict_destroy(m_mfdict);
	NOTICE("name[%s] finish DONE. OK", NAME);
	return 0;
}

int uniq_handle::ontime()
{
	NOTICE ("name[%s] I'm ontiming... clock it", NAME);
	return 0;
}
const char* uniq_handle::version()
{
	return VERSION;
}

strategy_handle_t strategy_uniq_handle = 
{
	new uniq_handle(),
};

#ifdef __cplusplus
extern "C" {
#endif
#ifndef LD_SO_PATH
#ifndef __i386
#define LD_SO_PATH "/lib64/ld-linux-x86-64.so.2"
#else
#define LD_SO_PATH "/lib/ld-linux.so.2"
#endif
#endif

	const char interp[] __attribute__((section(".interp"))) = LD_SO_PATH;
	void so_main()
	{
		printf("version %s\n", VERSION);
		exit(0);
	}
#ifdef __cplusplus
}
#endif

/* vim: set ts=4 sw=4 sts=4 tw=100 */
