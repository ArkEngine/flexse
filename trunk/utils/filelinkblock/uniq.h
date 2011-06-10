#ifndef _FLEXSE_H
#define _FLEXSE_H

#include <virtualStrategy.h>
#include <threadContext.h>
#include <pthread.h>
#include <vector>
#include <string>
#include "mfifo_dict.h"

using namespace std;
class uniq_handle:public DEFAULT_STRATEGY_HANDLE
{
	private:
		// 用于存放要求去重的字段，从配置中读取
		vector< vector<string> >  m_cache_key_list;
		// 用于实现 mfdict_build_t 的线程安全
		pthread_mutex_t m_mfifo_mutex;
		// 去重 cache 的实现
		mfdict_build_t* m_mfdict;

		char m_dict_path[128];
		char m_dict_name[128];

		// 去重的结果
		uint32_t repeat_count;
		// 非去重的结果
		uint32_t passby_count;
	public:

		uniq_handle();
		virtual ~uniq_handle();
		/**
		 * @brief 初始化
		 *
		 * @return  int 
		 * @retval    
		 **/
		int init(const uint32_t thread_num);


		/**
		 * @brief 处理数据报文
		 *
		 * @return  int 
		 * @retval   
		 **/
		int process(thread_context* pcontext);

		/**
		 * @brief 退出时调用
		 *
		 * @return  int 
		 * @retval   
		 **/
		int finish();

		/**
		 * @brief 定时调用
		 *
		 * @return  int 
		 * @retval   
		 **/
		int ontime();

		/**
		 * @brief 版本信息
		 *
		 * @return  const char*
		 * @retval   
		 **/
		const char* version();

};

#endif
