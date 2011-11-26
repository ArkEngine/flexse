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
    MyThrowAssert(m_postinglist_cell_size > 0);
    // 初始化bucket
    m_bucket_size = 1;
    MyThrowAssert(bucket_size < 32);
    m_bucket_size <<= (bucket_size < 20 ) ? 20 : bucket_size;
    m_bucket_mask = m_bucket_size - 1;
    m_bucket = (uint32_t*)malloc(m_bucket_size*sizeof(uint32_t));
    // 初始化headlist
    m_headlist_size = headlist_size < 1000000 ? 2000000 : headlist_size;
    m_headlist = (term_head_t*)malloc(m_headlist_size*sizeof(term_head_t));
    m_headlist_used = 0;
    m_headlist_sort = NULL;

    // 要求最小能放下 4 个cell
    uint32_t mem_base_min = (uint32_t)(m_postinglist_cell_size * 4 + sizeof(mem_link_t));
    for (uint32_t i=0; i<32; i++)
    {
        if (uint32_t(1 << i) >= mem_base_min)
        {
            m_mem_base_size = 1 << i;
            break;
        }
    }
    uint32_t memsize[8];
    uint32_t mem_total_size = 0;
    for (uint32_t i=0; i<mblklist_size; i++)
    {
        memsize[i] = (i == 0) ? m_mem_base_size : memsize[i-1]*2;
        mem_total_size += memsize[i] * mblklist[i];
    }
    
    m_memblocks = new memblocks(memsize, mblklist, mblklist_size);
    // because of END_OF_LIST == 0xFFFFFFFF
    memset(m_bucket, 0xFF, m_bucket_size*sizeof(uint32_t));
    m_readonly = false;

    pthread_rwlock_init(&m_mutex, NULL);
    ROUTN("cell_size[%u] bucket_size[%u] headlist_size[%u] "
            "memblocks_min[%u] memblocks_num[%u] mem_total_size[%u]",
            m_postinglist_cell_size, m_bucket_size, m_headlist_size,
            m_mem_base_size, memsize[0], mem_total_size);
}

postinglist :: ~postinglist()
{
    // 你确认没人使用了postinglist哦，否则就 SIGNAL 11
    // 遍历所有的memblocks，释放内存
    for (uint32_t head_list_offset=0; head_list_offset<m_headlist_used; head_list_offset++)
    {
        term_head_t* phead = &m_headlist[head_list_offset];
        mem_link_t* mem_link = phead->mem_link;
        MyThrowAssert(mem_link != NULL);
        while(mem_link != NULL)
        {
            mem_link_t* tmp_mem_link = mem_link;
            mem_link = mem_link->next;
            m_memblocks->FreeMem(tmp_mem_link);
        }
    }
    delete m_memblocks;
    m_memblocks = NULL;
    free(m_headlist);
    m_headlist = NULL;
    free(m_bucket);
    m_bucket = NULL;
}

void postinglist :: memlinkcopy(mem_link_t* mem_link, const void* buff, const uint32_t length)
{
    // 倒着使用内存，从尾部向头部写
    int32_t woffset = (int32_t)(mem_link->self_size - mem_link->used_size - (length+sizeof(mem_link_t)));
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
    return memlink;
}

int32_t postinglist :: get (const uint64_t& key, void* buff, const uint32_t length)
{
    int32_t  result_num = 0;
    uint32_t left_size = length;
    const uint32_t bucket_no = (uint32_t)(key & m_bucket_mask);
    uint32_t head_list_offset = m_bucket[bucket_no];
    pthread_rwlock_rdlock(&m_mutex);
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
                    ALARM("key[%llu] buffer length[%u] is NOT enough.", key, length);
                    break;
                }
                uint32_t coffset = mem_link->self_size - mem_link->used_size;
                coffset -= (uint32_t)sizeof(mem_link_t);
                char* src = &(((char*)&mem_link[1])[coffset]);
                uint32_t copy_length =
                    (left_size > mem_link->used_size) ? mem_link->used_size: left_size;
                memmove(&((char*)buff)[length - left_size], src, copy_length);
                result_num += copy_length / m_postinglist_cell_size;
                left_size  -= copy_length;
                //                DEBUG("mem_link[%p] next[%p] bid[%u]", mem_link, mem_link->next, *(uint32_t*)src);
                mem_link = mem_link->next;
            }
            break;
        }
        else
        {
            head_list_offset = phead->next;
        }
    }
    pthread_rwlock_unlock(&m_mutex);

    return result_num;
}

int32_t postinglist :: set (const uint64_t& key, const void* buff)
{
    if (m_readonly)
    {
        return FULL;
    }
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
            left_size -= (uint32_t)sizeof(mem_link_t);

            if (left_size >= m_postinglist_cell_size)
            {
                memlinkcopy(phead->mem_link, buff, m_postinglist_cell_size);
            }
            else
            {
                // 判断一下是否需要merge内存
                // 执行merge之后，就不需要继续申请内存了，因为肯定空出来一个mem_link_t
                // 新的数据就放在这个mem_link_t里好了。
                //
                // 当 mem_link_t 可能小于 m_postinglist_cell_size 时，我们需要这么做
                // 放入新元素时检查是否能放下，
                // 如果能放下则是万幸
                // 如果不能放下，则需要merge内存，这将是一个连续的操作直到触发如下条件
                // -1- 合并后，已经能够放下 m_postinglist_cell_size 大小的数据
                // -2- 后面只有一块内存，无法继续合并(需要两块内存才能合并)，则申请一个最小单元的即可

                bool has_copy = false;
                pthread_rwlock_wrlock(&m_mutex);
                while ( (NULL != phead->mem_link->next)
                        && (phead->mem_link->self_size == phead->mem_link->next->self_size))
                {
                    mem_link_t* merge_memlink = memlinknew(phead->mem_link->self_size*2, phead->mem_link->next->next);
                    DEBUG("merge the memblocks seg1[%u] seg2[%u]",
                            phead->mem_link->used_size, phead->mem_link->next->used_size);

                    uint32_t coffset = 0;
                    char*    src = NULL;
                    coffset = phead->mem_link->next->self_size - phead->mem_link->next->used_size;
                    coffset -= (uint32_t)sizeof(mem_link_t);
                    src = &(((char*)&phead->mem_link->next[1])[coffset]);
                    memlinkcopy(merge_memlink, src, phead->mem_link->next->used_size);

                    coffset = phead->mem_link->self_size - phead->mem_link->used_size;
                    coffset -= (uint32_t)sizeof(mem_link_t);
                    src = &(((char*)&phead->mem_link[1])[coffset]);
                    memlinkcopy(merge_memlink, src, phead->mem_link->used_size);

                    // set 数据
//                    memlinkcopy(merge_memlink, buff, m_postinglist_cell_size);

                    // 把新的mem_link接入headlist
                    mem_link_t* toBeFree = phead->mem_link;
                    phead->mem_link = merge_memlink;

                    // 释放被合并的内存块
                    // 这里其实存在一个危险的竟态条件的
                    // 这些准备被释放的内存，可能正在被其他线程使用。
                    m_memblocks->FreeMem(toBeFree->next);
                    m_memblocks->FreeMem(toBeFree);

                    // 如果合并后的内存块能放下 m_postinglist_cell_size 大小的数据，则退出merge
                    uint32_t merged_left_size = phead->mem_link->self_size - phead->mem_link->used_size;
                    // 继续去掉mem_link_t头部占用的大小
                    merged_left_size -= (uint32_t)sizeof(mem_link_t);

                    if (merged_left_size >= m_postinglist_cell_size)
                    {
                        memlinkcopy(phead->mem_link, buff, m_postinglist_cell_size);
                        has_copy = true;
                        break;
                    }
                }
                pthread_rwlock_unlock(&m_mutex);

                // 如果是无法继续合并了
                if (false == has_copy)
                {
                    assert ( (NULL == phead->mem_link->next)
                            || (phead->mem_link->self_size < phead->mem_link->next->self_size));
                    // 申请 mem_link_t
                    mem_link_t* new_memlink = memlinknew(m_mem_base_size, phead->mem_link);
                    // set 数据
                    memlinkcopy(new_memlink, buff, m_postinglist_cell_size);
                    // 把新的mem_link接入headlist
                    phead->mem_link = new_memlink;
                    has_copy = true;
                }
                assert(has_copy);
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
        if  (m_headlist_used == m_headlist_size)
        {
            // 冰冻住set操作，已经没有空余空间了。
            // 以后可以考虑使用一个无穷hash的方式，就是需要排序时麻烦一点
            set_readonly(true);
            return FULL;
        }
        else
        {
            // 设置 head
            m_headlist[m_headlist_used].sign64 = key;
            m_headlist[m_headlist_used].next = m_bucket[bucket_no];
            m_headlist[m_headlist_used].mem_link = NULL;

            // 申请 mem_link
            mem_link_t* new_memlink = memlinknew(m_mem_base_size, m_headlist[m_headlist_used].mem_link);
            m_headlist[m_headlist_used].mem_link = new_memlink;

            // set 数据
            memlinkcopy(new_memlink, buff, m_postinglist_cell_size);

            // 接入 bucket
            m_bucket[bucket_no] = m_headlist_used;
        }
        m_headlist_used++;
    }
    return (m_headlist_size - m_headlist_used) >= HEAD_LIST_WATER_LINE ? OK : NEARLY_FULL;
}

void postinglist :: set_readonly(bool readonly)
{
    m_readonly = readonly;
}

bool postinglist :: iswritable()
{
    return m_readonly == false;
}

int postinglist::key_compare(const void *p1, const void *p2)
{
    int64_t value = ((term_head_t*)p1)->sign64 - ((term_head_t*)p2)->sign64;
    if (value < 0)
    {
        return -1;
    }
    else if (value > 0)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

void postinglist :: begin()
{
    // 设置为只读状态
    set_readonly(true);
    if (m_headlist_sort != NULL)
    {
        free(m_headlist_sort);
        m_headlist_sort = NULL;
    }
    m_headlist_sort = (term_head_t*)malloc(m_headlist_used * sizeof(term_head_t));
    MyThrowAssert(m_headlist_sort != NULL);
    memmove(m_headlist_sort, m_headlist, m_headlist_used * sizeof(term_head_t));
    qsort(m_headlist_sort, m_headlist_used, sizeof(term_head_t), key_compare);
    m_headlist_sort_it = 0;
    return;
}
int32_t postinglist :: itget(uint64_t& key, void* buff, const uint32_t length)
{
    key = m_headlist_sort[m_headlist_sort_it].sign64;
    return get(key, buff, length);
}

void postinglist :: next()
{
    m_headlist_sort_it++;
}

bool postinglist :: is_end()
{
    if (m_headlist_sort_it == m_headlist_used)
    {
        free(m_headlist_sort);
        m_headlist_sort = NULL;
        // 不必要在改为readonly = false了，finish完之后，应该销毁的
        return true;
    }
    else
    {
        return false;
    }
}

bool postinglist :: empty()
{
    return m_headlist_used == 0;
}

void postinglist :: clear()
{
    // 调用者保证执行这段代码时，没有人读写这个对象
    // 在flexse中，当这个postinglist持久化之后，就不需要访问这个对象了，然后就reset掉好了。
    //    MyThrowAssert(0);
    // 遍历所有的memblocks，释放内存
    // 不能直接delete m_memblocks完事，因为可能存在额外malloc的内存
    for (uint32_t head_list_offset=0; head_list_offset<m_headlist_used; head_list_offset++)
    {
        term_head_t* phead = &m_headlist[head_list_offset];
        mem_link_t* mem_link = phead->mem_link;
        MyThrowAssert(mem_link != NULL);
        while(mem_link != NULL)
        {
            mem_link_t* tmp_mem_link = mem_link;
            mem_link = mem_link->next;
            m_memblocks->FreeMem(tmp_mem_link);
        }
    }

    // 重置bucket
    memset(m_bucket, 0xFF, m_bucket_size*sizeof(uint32_t));
    set_readonly(false);
    m_headlist_used = 0;
    // 我就是舍不得释放这些已经申请的内存啊，谁能理解我的苦衷呢
}
