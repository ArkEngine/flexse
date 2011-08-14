#include "index_group.h"
#include "nlp_processor.h"
#include "bitmap.h"
#include "bitlist.h"
#include <json/json.h>

class flexse_plugin
{
    private:
        static const char* const m_StrInsideKey_DocIDList;

        static const char* const CONFIGCATEGORY_INDEXDESC;
        static const char* const CONFIGCATEGORY_FLEXINDEX;
        static const char* const CONFIGCATEGORY_ATTR;
        static const char* const CONFIGCATEGORY_NLP;
        static const char* const CONFIGCATEGORY_GENERAL;

        static const char* const m_StrMaxDocID;
        static const char* const m_StrDataDir;

        static const char* const m_StrCellSize;
        static const char* const m_StrBucketSize;
        static const char* const m_StrHeadListSize;
        static const char* const m_StrMemBlockNumList;

        index_group*   m_pindex_group;
        nlp_processor* m_pnlp_processor;
        bitmap*        m_mod_bitmap;
        bitmap*        m_del_bitmap;
        bitlist*       m_docattr_bitlist;

        // general config
        uint32_t m_max_doc_id;
        char     m_data_dir[128];

        // indexdesc config
        uint32_t m_cell_size;           // postinglist cell size
        uint32_t m_bucket_size;         // postinglist hash bucket size = 1 << m_bucket_size;
        uint32_t m_headlist_size;       // postinglist headlist size;
        uint32_t m_memblocknumlist[32]; // postinglist memblocks config
        uint32_t m_memblocknumlistsize;

        flexse_plugin();
        flexse_plugin(const flexse_plugin&);
    public:
        flexse_plugin(const char* config_path);
        ~flexse_plugin();
        int insert( Json::Value root, uint32_t& doc_id, vector<string> & vstr);
        int update( Json::Value root, uint32_t& doc_id, vector<string> & vstr);
        int remove( Json::Value root, vector<uint32_t> & id_list);
        int restore(Json::Value root, vector<uint32_t> & id_list);
};
