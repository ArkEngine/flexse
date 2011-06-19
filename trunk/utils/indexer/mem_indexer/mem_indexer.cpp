#include <string.h>
#include "mem_indexer.h"
#include "creat_sign.h"

mem_indexer :: mem_indexer(const uint32_t posting_cell_size, const uint32_t  bucket_size,
        const uint32_t  headlist_size, const uint32_t* mblklist, const uint32_t  mblklist_size)
    : m_posting_cell_size(posting_cell_size),
      m_postinglist(posting_cell_size, bucket_size, headlist_size, mblklist, mblklist_size) 
{
}

mem_indexer :: ~mem_indexer()
{
}

int32_t mem_indexer :: get_posting_list(const char* strTerm, char* buff, const uint32_t length)
{
    int len = strlen(strTerm);
    if (0 == len || NULL == buff || length == 0)
    {
        ALARM("strTerm[%s] buff[%p] length[%u]", strTerm, buff, length);
        return -1;
    }
    ikey_t ikey;
    creat_sign_64(strTerm, len, &ikey.uint1, &ikey.uint2);
    return m_postinglist.get(ikey.sign64, buff, length);
}

int32_t mem_indexer :: set_posting_list(const char* strTerm, const char* buff)
{
    int len = strlen(strTerm);
    if (0 == len || NULL == buff)
    {
        ALARM("strTerm[%s] buff[%p]", strTerm, buff);
        return -1;
    }
    ikey_t ikey;
    creat_sign_64(strTerm, len, &ikey.uint1, &ikey.uint2);
    return m_postinglist.set(ikey.sign64, buff);
}
