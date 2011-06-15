#include <assert.h>
#include <string.h>
#include "postinglist.h"


postinglist :: postinglist(const uint32_t posting_cell_size /*json config*/)
{
	// 初始化bucket
    m_postinglist_cell_size = posting_cell_size;
	m_bucket_size = 0x100000;
	m_bucket_mask = m_bucket_size - 1;
	m_bucket = (uint32_t*)calloc(m_bucket_size, sizeof(uint32_t));
	// 初始化headlist
	const uint32_t headlist_size = 0x1000000;
	m_headlist = (term_head_t*)malloc(headlist_size*sizeof(term_head_t));
	m_headlist_use = 0;

    uint32_t msiz = 256;
    for (uint32_t i=0; i<sizeof(m_memsize)/sizeof(m_memsize[0]); i++)
    {
        m_memsize[i] = msiz;
        msiz = msiz * 2;
    }
}

int32_t postinglist :: get (const uint64_t& key, char* buff, const uint32_t length)
{
    assert(key && buff && length);
    return 0;
}

int32_t postinglist :: set (const uint64_t& key, char* buff)
{
	uint32_t bucket_no = (uint32_t)(key & m_bucket_mask);
	uint32_t head_list_offset = m_bucket[bucket_no];
	bool found = false;
	while(head_list_offset != END_OF_LIST)
	{
		term_head_t* phead = &m_headlist[head_list_offset];
		if (phead->sign64 == key)
		{
			found = true;
			// 看看当前的memblock是否能够放下
			if (phead->mem_link->left_size <= m_postinglist_cell_size)
			{
				char* dest = &(((char*)&phead->mem_link[1])[phead->mem_link->self_size - phead->mem_link->left_size]);
				memcpy(dest, buff, m_postinglist_cell_size);
				phead->mem_link->left_size -= m_postinglist_cell_size;
			}
			else
			{
				term_head_t* newhead = (term_head_t*)m_memblocks->AllocMem(m_memsize[0]);
				MyToolThrow("memblocks allocmem fail.");
				newhead->mem_link->left_size = m_memsize[0] - sizeof(term_head_t);
				newhead->mem_link->self_size = m_memsize[0];
				newhead->mem_link->next_size = phead->mem_link->self_size;
			}
		}
	}
	if (! found)
	{
	}
    return 0;
}
int32_t postinglist :: sort()
{
    return 0;
}
int32_t postinglist :: dump()
{
    return 0;
}
int32_t postinglist :: merge()
{
    return 0;
}
int32_t postinglist :: begin()
{
    return 0;
}
int32_t postinglist :: next(const uint64_t& key, char* buff, const uint32_t length)
{
    assert(key && buff && length);
    return 0;
}
bool postinglist :: isend()
{
    return 0;
}
