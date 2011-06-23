#include "memblocks.h"
#include <assert.h>

using namespace flexse;

memblocks::~memblocks()
{
	free(m_mem);
}

memblocks::memblocks(const uint32_t* blocksize, const uint32_t* blocknum, const uint32_t catnum)
{
	if (blocksize == NULL || blocknum == NULL || catnum <= 0 || catnum > m_MemCatMaxnum)
	{
		ALARM("param error @ blocksize[%p] blocknum[%p] catnum[%d] < m_MemCatMaxnum[%d]",
				blocksize, blocknum, catnum, m_MemCatMaxnum);
		MyToolThrow("Param Error Failed in Mempool Construct");
	}
	m_catnum = catnum;
	for (uint32_t i=0; i<m_catnum; i++)
	{
		if (blocksize[i] <= 0 || blocknum[i] <= 0)
		{
			ALARM("param error @ blocksize[%d]: %d blocknum[%d]: %d",
					i, blocksize[i], i, blocknum[i]);
			MyToolThrow("Param Error Failed in Mempool Construct");
		}
		if (i>0 && blocksize[i] < blocksize[i-1])
		{
			ALARM("blocksize order error @ blocksize[%d]: %d < blocknum[%d]: %d",
					i, blocksize[i], i-1, blocksize[i-1]);
			MyToolThrow("Param Error Failed in Mempool Construct");
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
		ALARM("malloc failed. memsize[%u]", m_memsize);
		MyToolThrow("MemAllocBad in Mempool Construct");
	}
	int memoffset = 0;
	for (uint32_t i=0; i<m_catnum; i++)
	{
		m_mem_cat_info[i].bmem = &m_mem[memoffset];
		m_mem_cat_info[i].emem = &m_mem[memoffset + m_mem_cat_info[i].count * m_mem_cat_info[i].size];
		for(uint32_t j=0; j<m_mem_cat_info[i].count; j++)
		{
			mem_link_t* tmpmlist = (mem_link_t*)&m_mem[memoffset];
			tmpmlist->mem  = &m_mem[memoffset];
			tmpmlist->next = m_mem_cat_info[i].memlisthead;
			m_mem_cat_info[i].memlisthead = tmpmlist;
			memoffset += m_mem_cat_info[i].size;
		}
		ROUTN("catidx[%u] blocksize[%u] blocknum[%u]",
				i, m_mem_cat_info[i].size, m_mem_cat_info[i].count);
	}

	m_mem_extra.memlisthead = NULL;
	m_mem_extra.bmem = NULL;
	m_mem_extra.emem = NULL;
	m_mem_extra.size = 0;
	m_mem_extra.count = 0;
	m_mem_extra.freecount = 0;

	m_mem_extra_tatol = 0;

    m_log_killer = 0;
	pthread_mutex_init(&m_mutex, NULL);
}

void* memblocks::AllocMem(const uint32_t memsize)
{
	if (m_mem == NULL)
	{
		FATAL("memblocks maybe Not be Inited");
		return NULL;
	}
	if (memsize <= 0)
	{
		ALARM("Param error memsize[%u]", memsize);
		return NULL;
	}
	pthread_mutex_lock(&m_mutex);

	void* tmpmem = NULL;
	int memblocks_crupt = 0;
	for (uint32_t i=0; i<m_catnum; i++)
	{
        // 完全相等才分配，避免内存浪费
		if (m_mem_cat_info[i].size == memsize && m_mem_cat_info[i].freecount > 0)
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
            if ((++m_log_killer) > 1000)
            {
                m_log_killer = 0;
                ALARM("allocmem[%p] in extramem memsize[%d] blocknum[%d]",
                        tmpmem, memsize, m_mem_extra.count);
            }
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
    uint32_t i;
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
    for (uint32_t i=0; i<m_catnum; i++)
    {
        DEBUG("memcatidx[%d] blocksize[%8d] blocknum[%3d] free[%3d]",
                i, m_mem_cat_info[i].size, m_mem_cat_info[i].count, m_mem_cat_info[i].freecount);
    }
    DEBUG("extramem total[%d] blocknum[%d]", m_mem_extra_tatol, m_mem_extra.count);
}

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
