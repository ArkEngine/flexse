#include "memblocks.h"
#include <assert.h>

using namespace sndass;

memblocks::memblocks()
{
	m_catnum = 0;
	m_memsize = 0;
	m_blocknum = 0;
	m_mem = NULL;
	for (unsigned int i=0; i<m_MemCatMaxnum; i++)
	{
		m_mem_cat_info[i].memlisthead = NULL;
		m_mem_cat_info[i].size = 0;
		m_mem_cat_info[i].count = 0;
		m_mem_cat_info[i].freecount = 0;
		m_mem_cat_info[i].bmem = NULL;
		m_mem_cat_info[i].emem = NULL;
	}
	m_mem_extra.memlisthead = NULL;
	m_mem_extra.bmem = NULL;
	m_mem_extra.emem = NULL;
	m_mem_extra.size = 0;
	m_mem_extra.count = 0;
	m_mem_extra.freecount = 0;

	m_mem_extra_tatol = 0;
	pthread_mutex_init(&m_mutex, NULL);
}

memblocks::~memblocks()
{
	free(m_mem);
}

memblocks::memblocks(const int* blocksize, const int* blocknum, const int catnum)
{
	if (blocksize == NULL || blocknum == NULL || catnum <= 0 || catnum > (int)m_MemCatMaxnum)
	{
		WARNING("param error @ blocksize[%p] blocknum[%p] catnum[%d] < m_MemCatMaxnum[%d]",
				blocksize, blocknum, catnum, m_MemCatMaxnum);
		throw MyException("Param Error Failed in Mempool Construct",
				__FILE__, __LINE__, __func__);
	}
	m_catnum = catnum;
	for (int i=0; i<m_catnum; i++)
	{
		if (blocksize[i] <= 0 || blocknum[i] <= 0)
		{
			WARNING("param error @ blocksize[%d]: %d blocknum[%d]: %d",
					i, blocksize[i], i, blocknum[i]);
			throw MyException("Param Error Failed in Mempool Construct",
					__FILE__, __LINE__, __func__);
		}
		if (i>0 && blocksize[i] < blocksize[i-1])
		{
			WARNING("blocksize order error @ blocksize[%d]: %d < blocknum[%d]: %d",
					i, blocksize[i], i-1, blocksize[i-1]);
			throw MyException("Param Error Failed in Mempool Construct",
					__FILE__, __LINE__, __func__);
		}
		m_mem_cat_info[i].size = blocksize[i];
		m_mem_cat_info[i].count = blocknum[i];
		m_mem_cat_info[i].freecount = blocknum[i];
		m_memsize += blocksize[i] * blocknum[i];
		m_blocknum += blocknum[i];
	}
	m_mem = (char*)malloc(m_memsize);
	if (m_mem == NULL)
	{
		WARNING("malloc failed. memsize[%d]", m_memsize);
		throw MyException("MemAllocBad in Mempool Construct",
				__FILE__, __LINE__, __func__);
	}
	int memoffset = 0;
	for (int i=0; i<m_catnum; i++)
	{
		m_mem_cat_info[i].bmem = &m_mem[memoffset];
		m_mem_cat_info[i].emem = &m_mem[memoffset + m_mem_cat_info[i].count * m_mem_cat_info[i].size];
		for(int j=0; j<m_mem_cat_info[i].count; j++)
		{
			mem_link_t* tmpmlist = (mem_link_t*)&m_mem[memoffset];
			tmpmlist->mem  = &m_mem[memoffset];
			tmpmlist->next = m_mem_cat_info[i].memlisthead;
			m_mem_cat_info[i].memlisthead = tmpmlist;
			memoffset += m_mem_cat_info[i].size;
		}
		NOTICE("catidx[%d] blocksize[%d] blocknum[%d]",
				i, m_mem_cat_info[i].size, m_mem_cat_info[i].count);
	}
}

void* memblocks::AllocMem(const int memsize)
{
	if (m_mem == NULL)
	{
		FATAL("memblocks maybe Not be Inited");
		return NULL;
	}
	if (memsize < 0)
	{
		WARNING("Param error memsize[%d]", memsize);
		return NULL;
	}
	pthread_mutex_lock(&m_mutex);

	void* tmpmem = NULL;
	int memblocks_crupt = 0;
	for (int i=0; i<m_catnum; i++)
	{
		if (m_mem_cat_info[i].size >= memsize && m_mem_cat_info[i].freecount > 0)
		{
			if (m_mem_cat_info[i].memlisthead == NULL)
			{
				FATAL("memblocks maybe crupt in catmem");
				memblocks_crupt = 1;
				break;
			}
			tmpmem = m_mem_cat_info[i].memlisthead->mem;
			assert (m_mem_cat_info[i].memlisthead == m_mem_cat_info[i].memlisthead->mem);
			m_mem_cat_info[i].memlisthead = m_mem_cat_info[i].memlisthead->next;
			m_mem_cat_info[i].freecount--;
			break;
		}
	}
	if (!memblocks_crupt && tmpmem == NULL)
	{
		tmpmem = malloc(memsize);
		if (tmpmem == NULL)
		{
			FATAL("malloc for extra mem failed.");
		}
		else
		{
			m_mem_extra_tatol++;
			m_mem_extra.count++;
			WARNING("allocmem[%p] in extramem memsize[%d] blocknum[%d]",
					tmpmem, memsize, m_mem_extra.count);
		}
	}
	pthread_mutex_unlock(&m_mutex);
	return tmpmem;
}

int memblocks::FreeMem(void* mem)
{
	int ret = 0;
	if (mem == NULL)
	{
		return 0;
	}
	if (m_mem == NULL)
	{
		FATAL("memblocks maybe Not be Inited");
		return -1;
	}
	pthread_mutex_lock(&m_mutex);
	int i;
	for (i=0; i<m_catnum; i++)
	{
		if (mem >= m_mem_cat_info[i].bmem && mem < m_mem_cat_info[i].emem)
		{
			break;
		}
	}
	if (i < m_catnum)
	{
		mem_link_t* pmlink = (mem_link_t*)mem;
		pmlink->mem = mem;
		pmlink->next = m_mem_cat_info[i].memlisthead;
		m_mem_cat_info[i].memlisthead = pmlink;
		m_mem_cat_info[i].freecount++;
	}
	else if (i==m_catnum)
	{
		assert(m_mem_extra.count > 0);
		free(mem);
		m_mem_extra.count--;
	}

	pthread_mutex_unlock(&m_mutex);
	return ret;
}

int memblocks::ExtraMemCount()
{
	return m_mem_extra.count;
}

void memblocks::MemStatus()
{
	for (int i=0; i<m_catnum; i++)
	{
		TRACE("memcatidx[%d] blocksize[%8d] blocknum[%3d] free[%3d]",
				i, m_mem_cat_info[i].size, m_mem_cat_info[i].count, m_mem_cat_info[i].freecount);
	}
	TRACE("extramem total[%d] blocknum[%d]", m_mem_extra_tatol, m_mem_extra.count);
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
