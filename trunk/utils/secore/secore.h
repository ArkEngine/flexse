#ifndef _SECORE_H_
#define _SECORE_H_

#include "index_group.h"
#include "nlp_processor.h"
#include "bitmap.h"
#include "bitlist.h"
#include "idmap.h"
#include <json/json.h>

class secore
{
    private:

        static const char* const CONFIGCATEGORY_INDEXDESC;
        static const char* const CONFIGCATEGORY_FLEXINDEX;
        static const char* const CONFIGCATEGORY_ATTR;
        static const char* const CONFIGCATEGORY_NLP;
        static const char* const CONFIGCATEGORY_GENERAL;

        static const char* const m_StrMaxOuterID;
        static const char* const m_StrMaxInnerID;
        static const char* const m_StrIdmapDataDir;
        static const char* const m_StrAttrDataDir;

        static const char* const m_StrCellSize;
        static const char* const m_StrBucketSize;
        static const char* const m_StrHeadListSize;
        static const char* const m_StrMemBlockNumList;

        // general config
        uint32_t m_max_inner_id;
        uint32_t m_max_outer_id;
        char     m_idmap_data_dir[128];
        char     m_attr_data_dir[128];

        // indexdesc config
        uint32_t m_cell_size;           // postinglist cell size
        uint32_t m_bucket_size;         // postinglist hash bucket size = 1 << m_bucket_size;
        uint32_t m_headlist_size;       // postinglist headlist size;
        uint32_t m_memblocknumlist[32]; // postinglist memblocks config
        uint32_t m_memblocknumlistsize;

        secore();
        secore(const secore&);
    public:

        static const char* const m_StrInsideKey_DocIDList;
        static const char* const m_StrInsideKey_DocID;

        index_group*   m_pindex_group;
        nlp_processor* m_pnlp_processor;
        bitmap*        m_mod_bitmap;
        bitmap*        m_del_bitmap;
        idmap*         m_idmap;
        bitlist*       m_docattr_bitlist;

        secore(const char* config_path);
        ~secore();
};
#endif
