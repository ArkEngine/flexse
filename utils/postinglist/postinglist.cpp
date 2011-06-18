#include <assert.h>
#include <string.h>
#include "postinglist.h"


postinglist :: postinglist(
        const uint32_t  posting_cell_size,
        const uint32_t  bucket_size,
        const uint32_t  headlist_size,
        const uint32_t* mblklist,
        const uint32_t  mblklist_size
        )
{
    // 以sizeof(u_char)为单位
    m_postinglist_cell_size = posting_cell_size;
    // merge内存块的前提，避免连续多次merge
    MyThrowAssert(m_postinglist_cell_size <= sizeof(mem_link_t));
    // 初始化bucket
    m_bucket_size = 1;
    MyThrowAssert(bucket_size < 32);
    m_bucket_size <<= (bucket_size < 20 ) ? 20 : m_bucket_size;
    m_bucket_mask = m_bucket_size - 1;
    m_bucket = (uint32_t*)calloc(m_bucket_size, sizeof(uint32_t));
    // 初始化headlist
    m_headlist_size = headlist_size; // TODO
    m_headlist = (term_head_t*)malloc(m_headlist_size*sizeof(term_head_t));
    m_headlist_used = 0;

    uint32_t mem_base_min = m_postinglist_cell_size * 4 + sizeof(mem_link_t);
    for (uint32_t i=0; i<32; i++)
    {
        if ((1 << i) >= mem_base_min)
        {
            m_mem_base_size = 1 << i;
            break;
        }
    }
    uint32_t memsize[8];
    for (uint32_t i=0; i<mblklist_size; i++)
    {
        memsize[i] = (i == 0) ? m_mem_base_size : memsize[i-1]*2;
    }
    
    m_memblocks = new memblocks(memsize, mblklist, mblklist_size);
    // because of END_OF_LIST == 0xFFFFFFFF
    memset(m_bucket, 0xFF, m_bucket_size*sizeof(uint32_t));
}

postinglist :: ~postinglist()
{
    delete m_memblocks;
}

void postinglist :: memlinkcopy(mem_link_t* mem_link, const void* buff, const uint32_t length)
{
    // 倒着使用内存，从尾部向头部写
    int32_t woffset = mem_link->self_size - mem_link->used_size - (length+sizeof(mem_link_t));
    assert(woffset >= 0);
    char* tmp = ((char*)&mem_link[1]);
    char* dst = &tmp[woffset];
    memmove(dst, buff, length);
    mem_link->used_size += length;
}

postinglist::mem_link_t* postinglist :: memlinknew(const uint32_t memsiz, mem_link_t* next_memlink)
{
    mem_link_t* memlink = (mem_link_t*)m_memblocks->AllocMem(memsiz);
    MyThrowAssert(memlink != NULL);
    memlink->used_size = 0;
    memlink->self_size = memsiz;
    memlink->next      = next_memlink;
    memlink->next_size = (next_memlink == NULL) ? 0 : next_memlink->self_size;
    return memlink;
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
                    ALARM("key[%llu] buffer length[%u] is NOT enough.",
                            key, length);
                    break;
                }
                uint32_t coffset = mem_link->self_size - mem_link->used_size;
                coffset -= sizeof(mem_link_t);
                char* src = &(((char*)&mem_link[1])[coffset]);
                uint32_t copy_length =
                    (left_size > mem_link->used_size) ? mem_link->used_size: left_size;
                memmove(&buff[length - left_size], src, copy_length);
                result_num += copy_length / m_postinglist_cell_size;
                left_size  -= copy_length;
                //                DEBUG("mem_link[%p] next[%p] bid[%u]", mem_link, mem_link->next, *(uint32_t*)src);
                mem_link = mem_link->next;
            }
            break;
        }
    }

    return result_num;
}

int32_t postinglist :: set (const uint64_t& key, const char* buff)
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
                memlinkcopy(phead->mem_link, buff, m_postinglist_cell_size);
                phead->list_num++;
            }
            else
            {
                // 判断一下是否需要merge内存
                // 执行merge之后，就不需要继续申请内存了，因为肯定空出来一个mem_link_t
                // 新的数据就放在这个mem_link_t里好了。
                if ( (NULL != phead->mem_link->next)
                        && (phead->mem_link->self_size == phead->mem_link->next->self_size))
                {
                    mem_link_t* merge_memlink = memlinknew(phead->mem_link->self_size*2, phead->mem_link->next->next);
                    DEBUG("merge the memblocks seg1[%u] seg2[%u]\n",
                            phead->mem_link->used_size, phead->mem_link->next->used_size);

                    uint32_t coffset = 0;
                    char*    src = NULL;
                    coffset = phead->mem_link->next->self_size - phead->mem_link->next->used_size;
                    coffset -= sizeof(mem_link_t);
                    src = &(((char*)&phead->mem_link->next[1])[coffset]);
                    memlinkcopy(merge_memlink, src, phead->mem_link->next->used_size);

                    coffset = phead->mem_link->self_size - phead->mem_link->used_size;
                    coffset -= sizeof(mem_link_t);
                    src = &(((char*)&phead->mem_link[1])[coffset]);
                    memlinkcopy(merge_memlink, src, phead->mem_link->used_size);

                    // set 数据
                    memlinkcopy(merge_memlink, buff, m_postinglist_cell_size);

                    phead->list_num++;
                    // 把新的mem_link接入headlist
                    mem_link_t* toBeFree = phead->mem_link;
                    phead->mem_link = merge_memlink;

                    // 释放被合并的内存块
                    m_memblocks->FreeMem(toBeFree->next);
                    m_memblocks->FreeMem(toBeFree);
                }
                else
                {
                    // 申请 mem_link_t
                    mem_link_t* new_memlink = memlinknew(m_mem_base_size, phead->mem_link);

                    // set 数据
                    memlinkcopy(new_memlink, buff, m_postinglist_cell_size);

                    phead->list_num++;
                    phead->list_buffer_size += m_mem_base_size;

                    // 把新的mem_link接入headlist
                    phead->mem_link = new_memlink;
                }
            }
            break;
        }
        else
        {
            head_list_offset = phead->next;
        }
    }
    if (! found)
    {
        // 分配一个 headlist cell
        if (m_headlist_used == m_headlist_size)
        {
            // 冰冻住set操作，已经没有空余空间了。
            // 以后可以考虑使用一个无穷hash的方式，就是需要排序时麻烦一点
            // return sth.
            return FULL;
        }
        else
        {
            // 设置 head
            m_headlist[m_headlist_used].sign64 = key;
            m_headlist[m_headlist_used].list_num = 0;
            m_headlist[m_headlist_used].next = m_bucket[bucket_no];
            m_headlist[m_headlist_used].list_buffer_size = 0;
            m_headlist[m_headlist_used].mem_link = NULL;

            // 申请 mem_link
            mem_link_t* new_memlink = memlinknew(m_mem_base_size, m_headlist[m_headlist_used].mem_link);
            m_headlist[m_headlist_used].list_buffer_size += m_mem_base_size;
            m_headlist[m_headlist_used].mem_link = new_memlink;

            // set 数据
            memlinkcopy(new_memlink, buff, m_postinglist_cell_size);
            m_headlist[m_headlist_used].list_num ++;

            // 接入 bucket
            m_bucket[bucket_no] = m_headlist_used;
        }
        m_headlist_used++;
    }
    return OK;
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
