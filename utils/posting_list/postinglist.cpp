#include "postinglist.h"


postinglist :: postinglist(/*Json::Value config*/)
{
	// 初始化bucket
	m_bucket_size = 0x100000;
	m_bucket_mask = bucket_size - 1;
	m_bucket = (uint32_t*)calloc(bucket_size, sizeof(uint32_t));
	// 初始化headlist
	const uint32_t headlist_size = 0x1000000;
	m_headlist = (term_head_t*)malloc(headlist_size*sizeof(term_head_t));
	m_headlist_use = 0;
}
int32_t postinglist :: get (const uint64_t& key, char* buff, const uint32_t length)
{
}
int32_t postinglist :: set (const uint64_t& key, char* buff, const uint32_t length)
{
	uint32_t bucket_no = key & (uint64_t)m_bucket_mask;
	uint32_t head_list_offset = m_bucket[bucket_no];
	bool found = false;
	while(head_list_offset != END_OF_LIST)
	{
		term_head_t* phead = &m_headlist[head_list_offset];
		if (phead->sign64 == key)
		{
			found = true;
			// 看看当前的memblock是否能够放下
			if (phead->left_size <= length)
			{
				char* dest = &(((char*)list_buffer)[phead->self_size-phead->left_size]);
				memcpy(dest, buff, length);
				phead->left_size -= length;
			}
			else
			{
				term_head_t* newhead = (term_head_t*)m_memblocks->AllocMem(m_memsize[0]);
				MyToolThrow("memblocks allocmem fail.");
				newhead->left_size = m_memsize[0] - sizeof(term_head_t);
				newhead->self_size = m_memsize[0];
				newhead->next_size = phead->self_size;
			}
		}
	}
	if (! found)
	{
	}
}
int32_t postinglist :: sort();
int32_t postinglist :: dump();
int32_t postinglist :: merge();
int32_t postinglist :: begin();
int32_t postinglist :: next(key_t& key, char* buff, const uint32_t length);
bool    postinglist :: isend();
