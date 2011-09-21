#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "idmap.h"
#include "MyException.h"
#include "mylog.h"
#include "myutils.h"

using namespace flexse;

const char* const idmap::STR_IDMAP_FILE_PREFIX = "idmap";
const char* const idmap::STR_IDMAP_CUR_INNERID = "innerid.max";
const char* const idmap::STR_IDMAP_O2I_SUFFIX  = "o2i.list";
const char* const idmap::STR_IDMAP_I2O_SUFFIX  = "i2o.list";

idmap::idmap(const char* dir, const uint32_t maxOuterID, const uint32_t maxInnerID)
{
    m_max_outerid = maxOuterID;
    m_max_innerid = maxInnerID;

    char filename[MAX_FILE_LENGTH];
    snprintf(filename, sizeof(filename), "%s.%s",
            STR_IDMAP_FILE_PREFIX, STR_IDMAP_O2I_SUFFIX);
    m_po2imap = new bitmap(dir, filename, maxOuterID*sizeof(uint32_t));

    snprintf(filename, sizeof(filename), "%s.%s",
            STR_IDMAP_FILE_PREFIX, STR_IDMAP_I2O_SUFFIX);
    m_pi2omap = new bitmap(dir, filename, maxInnerID*sizeof(uint32_t));

    // load the cur innerid
    // 这个文件中存放的是已经分配出去的最大内部ID
    snprintf(m_innerid_file, sizeof(m_innerid_file), "%s/%s.%s",
            dir, STR_IDMAP_FILE_PREFIX, STR_IDMAP_CUR_INNERID);
    if (0 == access(m_innerid_file, F_OK))
    {
        char load_content[MAX_FILE_LENGTH];
        MySuicideAssert( 0 < read_file_all(m_innerid_file, load_content, sizeof(load_content)));
        MySuicideAssert( 1 == sscanf (load_content, "cur_innerid : %u", &m_cur_innerid));
        MySuicideAssert( m_cur_innerid > 0 );
    }
    else
    {
        m_cur_innerid = 0;
    }

    m_innerid_fd = -1;
}

idmap::~idmap()
{
    if (m_innerid_fd > 0)
    {
        close(m_innerid_fd);
    }
    delete m_pi2omap;
    delete m_po2imap;
}

uint32_t idmap::getInnerID(const uint32_t outerID)
{
    return (outerID >= m_max_outerid) ? 0 : m_po2imap->puint[outerID];
}

uint32_t idmap::getOuterID(const uint32_t innerID)
{
    return ((innerID + 1) >= m_max_innerid) ? 0 : m_pi2omap->puint[innerID];
}

uint32_t idmap::allocInnerID(const uint32_t outerID)
{
    if (outerID >= m_max_outerid || (m_cur_innerid + 1) >= m_max_innerid)
    {
        ALARM("outerID[%u] m_max_outerid[%u] m_cur_innerid+1[%u] m_max_innerid[%u]",
                outerID, m_max_outerid, m_cur_innerid+1, m_max_innerid);
        return 0;
    }

    if (m_innerid_fd < 0)
    {
        mode_t amode = (0 == access(m_innerid_file, F_OK)) ? O_WRONLY : O_WRONLY|O_CREAT;
        m_innerid_fd = open(m_innerid_file, amode, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
        MySuicideAssert(m_innerid_fd > 0);
    }
    else
    {
        lseek(m_innerid_fd, SEEK_SET, 0);
    }

    char save_content[MAX_FILE_LENGTH];
    int wlen = snprintf(save_content, sizeof(save_content), "cur_innerid : %u\n", ++m_cur_innerid);
    MySuicideAssert(wlen == write(m_innerid_fd, save_content, wlen));
    MySuicideAssert( 0 == ftruncate(m_innerid_fd, wlen));

    m_pi2omap->puint[m_cur_innerid] = outerID;
    m_po2imap->puint[outerID] = m_cur_innerid;
    PRINT("outerid[%u] <> innerid[%u]", outerID, m_cur_innerid);

    return m_cur_innerid;
}
