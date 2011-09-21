#include "detaildb.h"
#include "mylog.h"
#include "MyException.h"

detaildb :: detaildb(const char* dir, const char* iname)
    : m_fileblock(dir, iname, sizeof(dt_index_t)), m_diskv(dir, iname)
{
}

detaildb :: ~detaildb()
{
}

int32_t detaildb :: get(const uint32_t ino, void* buff, const uint32_t length)
{
    dt_index_t dt;
    MyThrowAssert(sizeof(dt_index_t) == m_fileblock.get(ino, &dt, sizeof(dt_index_t)));
    return m_diskv.get(dt.idx, buff, length);
}

int32_t detaildb :: set(const uint32_t ino, const void* buff, const uint32_t length)
{
    dt_index_t dt;
    MyThrowAssert(0 == m_diskv.set(dt.idx, buff, length));
    dt.ino = ino;
    MyThrowAssert(0 == m_fileblock.set(ino, &dt));
    return 0;
}
