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
		// ���ڴ��Ҫ��ȥ�ص��ֶΣ��������ж�ȡ
		vector< vector<string> >  m_cache_key_list;
		// ����ʵ�� mfdict_build_t ���̰߳�ȫ
		pthread_mutex_t m_mfifo_mutex;
		// ȥ�� cache ��ʵ��
		mfdict_build_t* m_mfdict;

		char m_dict_path[128];
		char m_dict_name[128];

		// ȥ�صĽ��
		uint32_t repeat_count;
		// ��ȥ�صĽ��
		uint32_t passby_count;
	public:

		uniq_handle();
		virtual ~uniq_handle();
		/**
		 * @brief ��ʼ��
		 *
		 * @return  int 
		 * @retval    
		 **/
		int init(const uint32_t thread_num);


		/**
		 * @brief �������ݱ���
		 *
		 * @return  int 
		 * @retval   
		 **/
		int process(thread_context* pcontext);

		/**
		 * @brief �˳�ʱ����
		 *
		 * @return  int 
		 * @retval   
		 **/
		int finish();

		/**
		 * @brief ��ʱ����
		 *
		 * @return  int 
		 * @retval   
		 **/
		int ontime();

		/**
		 * @brief �汾��Ϣ
		 *
		 * @return  const char*
		 * @retval   
		 **/
		const char* version();

};

#endif
