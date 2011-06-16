#include <assert.h>
#include <string.h>
#include "postinglist.h"


postinglist :: postinglist(const uint32_t posting_cell_size /*json config*/)
{
    // 以sizeof(u_char)为单位
    m_postinglist_cell_size = posting_cell_size;
    // 初始化bucket
    m_bucket_size = 0x100000;
    m_bucket_mask = m_bucket_size - 1;
    m_bucket = (uint32_t*)calloc(m_bucket_size, sizeof(uint32_t));
    // 初始化headlist
    m_headlist_size = 0x1000000;
    m_headlist = (term_head_t*)malloc(m_headlist_size*sizeof(term_head_t));
    m_headlist_used = 0;

    uint32_t msiz = 256;
    uint32_t mblocknum[8];
    for (uint32_t i=0; i<sizeof(m_memsize)/sizeof(m_memsize[0]); i++)
    {
        m_memsize[i] = msiz;
        msiz = msiz * 2;
        mblocknum[i] = 16;
    }
    
    m_memblocks = new memblocks(m_memsize, mblocknum, 8);
    memset(m_bucket, 0xFF, m_bucket_size*sizeof(uint32_t));
}

postinglist :: ~postinglist()
{
    delete m_memblocks;
}

int32_t postinglist :: get (const uint64_t& key, char* buff, const uint32_t length)
{
    int32_t  result_num = 0;
    uint32_t left_size = length;
    const uint32_t bucket_no = (uint32_t)(key & m_bucket_mask);
    uint32_t head_list_offset = m_bucket[bucket_no];
    while(head_list_offset != END_OF_LIST)
    {
        term_head_t* phead = &m_headlist[head_list_offset];
        if (phead->sign64 == key)
        {
            // 拷贝内存数据
            const mem_link_t* mem_link = phead->mem_link;
            while(NULL != mem_link)
            {
                if (left_size == 0)
                {
                    WARNING("key[%llu] buffer length[%u] is NOT enough.",
                            key, length);
                    break;
                }
                uint32_t coffset = mem_link->self_size - mem_link->used_size;
                char* src = &(((char*)&mem_link[1])[coffset]);
                uint32_t copy_length =
                    (left_size >= mem_link->used_size) ? mem_link->used_size: left_size;
                memmove(&buff[length - copy_length], src, copy_length);
                result_num += copy_length / m_postinglist_cell_size;
                left_size  -= copy_length;
                DEBUG("mem_link[%p] next[%p]", mem_link, mem_link->next);
                mem_link = mem_link->next;
            }
            break;
        }
    }

    return result_num;
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
            // 继续去掉mem_link_t头部占用的大小
            left_size -= sizeof(mem_link_t);

            if (left_size >= m_postinglist_cell_size)
            {
                uint32_t woffset = phead->mem_link->self_size - phead->mem_link->used_size - m_postinglist_cell_size;
                char* tmp = (char*)(&(phead->mem_link[1]));
                char* dst = &tmp[woffset];
                memmove(dst, buff, m_postinglist_cell_size);
                phead->mem_link->used_size += m_postinglist_cell_size;
                DEBUG("-next[%p]", phead->mem_link->next);
                phead->list_num++;
//                DEBUG("key[%llu] left_size[%u] cell_size[%u] mem_link_used[%u] list_num[%u] id[%u].",
//                        key, left_size, m_postinglist_cell_size, phead->mem_link->used_size,
//                        phead->list_num, *((uint32_t*)buff));
                break;
            }
            else
            {
                // 判断一下是否需要merge内存
                mem_link_t* new_memlink = (mem_link_t*)m_memblocks->AllocMem(m_memsize[0]);
                MyThrowAssert(new_memlink != NULL);
                new_memlink->used_size = 0;
                new_memlink->self_size = m_memsize[0];
                new_memlink->next_size = phead->mem_link->self_size;
                new_memlink->next      = phead->mem_link;
                uint32_t woffset = new_memlink->self_size - new_memlink->used_size - m_postinglist_cell_size;
                char* tmp = (char*)(&new_memlink[1]);
                char* dst = &tmp[woffset];
                DEBUG("new mem _1_next[%p : %p] id[%u]", phead->mem_link->next, new_memlink->next, *((uint32_t*)buff));
                memmove(dst, buff, m_postinglist_cell_size);
                DEBUG("new mem _2_next[%p : %p] id[%u]", phead->mem_link->next, new_memlink->next, *((uint32_t*)buff));
                phead->mem_link->next = NULL;
                new_memlink->used_size += m_postinglist_cell_size;
                phead->list_num++;
                phead->list_buffer_size += m_memsize[0];
                phead->mem_link = new_memlink;
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
        m_headlist_used++;
        if (m_headlist_used == m_headlist_size)
        {
            // 冰冻住set操作，已经没有空余空间了。
            // 以后可以考虑使用一个无穷hash的方式，就是需要排序时麻烦一点
            // return sth.
        }
        else
        {
            // 设置 head
            m_headlist[m_headlist_used].sign64 = key;
            m_headlist[m_headlist_used].list_num = 1;
            m_headlist[m_headlist_used].next = m_bucket[bucket_no];
            m_headlist[m_headlist_used].list_buffer_size = m_memsize[0];

            mem_link_t* new_memlink = (mem_link_t*)m_memblocks->AllocMem(m_memsize[0]);
            MyThrowAssert(new_memlink != NULL);
            new_memlink->used_size = 0;
            new_memlink->self_size = m_memsize[0];
            new_memlink->next_size = 0;
            new_memlink->next      = NULL;
            m_headlist[m_headlist_used].mem_link = new_memlink;

            // 内存是倒着写的，先写尾部
            uint32_t woffset = new_memlink->self_size - new_memlink->used_size - m_postinglist_cell_size;
            char* dst = &(((char*)&new_memlink[1])[woffset]);
            memcpy(dst, buff, m_postinglist_cell_size);
            m_headlist[m_headlist_used].mem_link->used_size += m_postinglist_cell_size;
            m_bucket[bucket_no] = m_headlist_used;
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
