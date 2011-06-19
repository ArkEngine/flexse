#include "disk_indexer.h"
#include "mylog.h"
#include "MyException.h"

disk_indexer :: disk_indexer(const char* dir, const char* iname)
    : m_fileblock(dir, iname, sizeof(fb_index_t)), m_diskv(dir, iname)
{
}

disk_indexer :: ~disk_indexer()
{
}

int32_t disk_indexer :: get_posting_list(const char* strTerm, char* buff, const uint32_t length)
{
    return 0;
}

int32_t disk_indexer :: set_posting_list(const uint32_t id, const ikey_t& ikey,
        const char* buff, const uint32_t length)
{
    fb_index_t fi;
    MyThrowAssert(0 == m_diskv.set(fi.idx, buff, length));
    fi.ikey = ikey;
    MyThrowAssert(0 == m_fileblock.set(id, &fi));
    return 0;
}
