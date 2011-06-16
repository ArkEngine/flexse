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
	m_headlist_size = 0x1000000;
	m_headlist = (term_head_t*)malloc(m_headlist_size*sizeof(term_head_t));
	m_headlist_used = 0;

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
	const uint32_t bucket_no = (uint32_t)(key & m_bucket_mask);
	uint32_t head_list_offset = m_bucket[bucket_no];
	bool found = false;
	while(head_list_offset != END_OF_LIST)
	{
		term_head_t* phead = &m_headlist[head_list_offset];
		if (phead->sign64 == key)
		{
			found = true;
			// 看看当前的memblock是否能够放下
			uint32_t left_size = phead->mem_link->self_size - phead->mem_link->used_size;
			if (left_size <= m_postinglist_cell_size)
			{
				char* dest = &(((char*)&phead->mem_link[1])[phead->mem_link->used_size]);
				memcpy(dest, buff, m_postinglist_cell_size);
				phead->mem_link->used_size += m_postinglist_cell_size;
				phead->list_num++;
				break;
			}
			else
			{
				// 判断一下是否需要merge内存
				mem_link_t* new_memlink = (mem_link_t*)m_memblocks->AllocMem(m_memsize[0]);
				MyThrowAssert(new_memlink != NULL);
				new_memlink->used_size = m_memsize[0] - sizeof(mem_link_t);
				new_memlink->self_size = m_memsize[0];
				new_memlink->next_size = phead->mem_link->self_size;
				new_memlink->next      = phead->mem_link;
				char* dest = &(((char*)&new_memlink[1])[new_memlink->used_size]);
				memcpy(dest, buff, m_postinglist_cell_size);
				new_memlink->used_size += m_postinglist_cell_size;
				phead->list_num++;
				phead->list_buffer_size += m_memsize[0];
				break;
			}
		}
		else
		{
			head_list_offset = phead->next;
		}
	}
	if (! found)
	{
		// 分配一个 headlist cell
		m_headlist++;
		if (m_headlist_used == m_headlist_size)
		{
			// 冰冻住set操作，已经没有空余空间了。
			// 以后可以考虑使用一个无穷hash的方式，就是需要排序时麻烦一点
			// return sth.
		}
		else
		{
			m_headlist[m_headlist_used].sign64 = key;
			m_headlist[m_headlist_used].list_num = 1;
			m_headlist[m_headlist_used].next = m_bucket[bucket_no];
			m_headlist[m_headlist_used].list_buffer_size = m_memsize[0];

			mem_link_t* new_memlink = (mem_link_t*)m_memblocks->AllocMem(m_memsize[0]);
			MyThrowAssert(new_memlink != NULL);
			m_headlist[m_headlist_used].mem_link = new_memlink;
			m_headlist[m_headlist_used].mem_link->used_size = m_memsize[0] - sizeof(mem_link_t);
			m_headlist[m_headlist_used].mem_link->self_size = m_memsize[0];
			m_headlist[m_headlist_used].mem_link->next_size = 0;
			char* dest = &(((char*)&new_memlink[1])[new_memlink->used_size]);
			memcpy(dest, buff, m_postinglist_cell_size);
			m_headlist[m_headlist_used].mem_link->used_size += m_postinglist_cell_size;
		}
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
