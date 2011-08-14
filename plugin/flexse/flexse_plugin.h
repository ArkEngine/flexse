#ifndef _FLEXSE_PLUGIN_H_
#define _FLEXSE_PLUGIN_H_

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
        static const char* const m_StrAttrDataDir;

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
        char     m_attr_data_dir[128];

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
        index_group* getIndexGroup();
        int add(  const char* jsonstr, uint32_t& doc_id, vector<string> & vstr);
        int mod(  const char* jsonstr, uint32_t& doc_id, vector<string> & vstr);
        int del(  const char* jsonstr, vector<uint32_t> & id_list);
        int undel(const char* jsonstr, vector<uint32_t> & id_list);
};
#endif
