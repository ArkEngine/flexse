#ifndef  _MEMBLOCKS_H_
#define  _MEM_H_
#include <stdlib.h>
#include <pthread.h>
#include "Log.h"
#include "MyException.h"

class memblocks
{
	public:
		~memblocks();
		void MemStatus();

		/*
		 * @brief: show the status of memblocks
		 * retval: int
		 * the count of malloc memblock
		 */
		int ExtraMemCount();

		/*
		 * @brief: Init the memblocks
		 * @param[in] blocksize : const int* -- blocksize array
		 * @param[in] blocknum  : const int* -- blocknum  array
		 * @param[in] catnum    : const int  -- cat num (array num)
		 * retval: excpetion
		 */
		memblocks(const int* blocksize, const int* blocknum, const int catnum);

		/*
		 * @brief: Alloc memory by size;
		 * @param[in] memsize : const int -- memory size
		 * first try alloc memory in memblocks, if the size is too large or memblocks-use-out, try glibc-malloc.
		 * retval: void*
		 * NULL  fail
		 * !NULL success
		 */
		void* AllocMem(const int memsize);

		/* 
		 * @brief: free mem
		 * @param[in] mem : void* -- mem point
		 * first try free memory in memblocks, if NOT found in memblocks, try glibc-free.
		 * NOTE: no warranty to check the mem-point valid
		 * retval: int
		 * 0  success
		 * <0 fail
		 */
		int FreeMem (void* mem);
	private:
		static const unsigned int m_MemCatMaxnum = 4;
		memblocks();
		memblocks(const memblocks&);
		struct mem_link_t
		{
			void* mem;
			mem_link_t* next;
		};
		struct mem_category_info_t
		{
			mem_link_t* memlisthead;
			int size;
			int count;
			int freecount;
			char* bmem;
			char* emem;
		};
		mem_category_info_t m_mem_cat_info[m_MemCatMaxnum];
		mem_category_info_t m_mem_extra;
		int m_catnum;
		int m_memsize;
		int m_blocknum;
		char* m_mem;
		u_int m_mem_extra_tatol;
		pthread_mutex_t m_mutex;
};

#endif  //_MEMPOOL_H_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
