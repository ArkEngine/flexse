#ifndef _IDMAP_H_
#define _IDMAP_H_

#include "bitmap.h"

class idmap
{
    private:

        static const uint32_t MAX_FILE_LENGTH = 128;
        static const char* const STR_IDMAP_FILE_PREFIX;
        static const char* const STR_IDMAP_CUR_INNERID;
        static const char* const STR_IDMAP_O2I_SUFFIX;
        static const char* const STR_IDMAP_I2O_SUFFIX;

        bitmap*  m_po2imap;
        bitmap*  m_pi2omap;
        uint32_t m_cur_innerid;
        uint32_t m_max_innerid;
        uint32_t m_max_outerid;
        int32_t  m_innerid_fd;
        char     m_innerid_file[MAX_FILE_LENGTH];

        idmap();
        idmap(const idmap&);
    public:
        idmap(const char* dir, const uint32_t maxOuterID, const uint32_t maxInnerID);
        ~idmap();

        /* 以下三个接口，返回0表示失败，返回>0表示成功 */
        uint32_t getInnerID(const uint32_t outerID);
        uint32_t getOuterID(const uint32_t innerID);
        uint32_t allocInnerID(const uint32_t outerID);
};
#endif
