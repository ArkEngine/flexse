#include "secore.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
using namespace std;

const char* const secore::m_StrInsideKey_DocIDList = "id_list";
const char* const secore::m_StrInsideKey_DocID     = "id";

const char* const secore::CONFIGCATEGORY_INDEXDESC = "INDEXDESC";
const char* const secore::CONFIGCATEGORY_FLEXINDEX = "FLEXINDEX";
const char* const secore::CONFIGCATEGORY_ATTR      = "ATTR";
const char* const secore::CONFIGCATEGORY_NLP       = "NLP";
const char* const secore::CONFIGCATEGORY_GENERAL   = "GENERAL";

const char* const secore::m_StrMaxOuterID   = "MaxOuterID";
const char* const secore::m_StrMaxInnerID   = "MaxInnerID";
const char* const secore::m_StrIdmapDataDir = "IdmapDataDir";
const char* const secore::m_StrAttrDataDir  = "AttrDataDir";

const char* const secore::m_StrCellSize = "PostingListCellSize";
const char* const secore::m_StrBucketSize = "PostingBucketSize";
const char* const secore::m_StrHeadListSize = "PostingHeadListSize";
const char* const secore::m_StrMemBlockNumList = "PostingMemBlockNumList";

secore:: secore(const char* config_path)
{
    // PLUGIN CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(config_path);
    MyThrowAssert (reader.parse(in, root));

    // GENERAL CONFIG
    Json::Value generalConfig = root[CONFIGCATEGORY_GENERAL];
    MyThrowAssert (! generalConfig.isNull());

    MyThrowAssert (! generalConfig[m_StrMaxInnerID].isNull());
    m_max_inner_id = generalConfig[m_StrMaxInnerID].asInt();
    MyThrowAssert(m_max_inner_id > 0);

    MyThrowAssert (! generalConfig[m_StrMaxOuterID].isNull());
    m_max_outer_id = generalConfig[m_StrMaxOuterID].asInt();
    MyThrowAssert(m_max_outer_id > 0);

    MyThrowAssert (! generalConfig[m_StrIdmapDataDir].isNull());
    snprintf(m_idmap_data_dir, sizeof(m_idmap_data_dir), "%s", generalConfig[m_StrIdmapDataDir].asCString());

    // ATTR CONFIG
    Json::Value attrConfig = root[CONFIGCATEGORY_ATTR];
    MyThrowAssert (! attrConfig.isNull());

    MyThrowAssert (! attrConfig[m_StrAttrDataDir].isNull());
    snprintf(m_attr_data_dir, sizeof(m_attr_data_dir), "%s", attrConfig[m_StrAttrDataDir].asCString());

    // INDEXDESC CONFIG
    Json::Value indexDescConfig = root[CONFIGCATEGORY_INDEXDESC];
    MyThrowAssert (! indexDescConfig.isNull());

    MyThrowAssert (! indexDescConfig[m_StrCellSize].isNull());
    m_cell_size = indexDescConfig[m_StrCellSize].asInt();
    MyThrowAssert (! indexDescConfig[m_StrBucketSize].isNull());
    m_bucket_size = indexDescConfig[m_StrBucketSize].asInt();
    MyThrowAssert (! indexDescConfig[m_StrHeadListSize].isNull());
    m_headlist_size = indexDescConfig[m_StrHeadListSize].asInt();
    Json::Value memblocknumlist = indexDescConfig[m_StrMemBlockNumList];
    MyThrowAssert (!memblocknumlist.isNull() && memblocknumlist.isArray());
    m_memblocknumlistsize = memblocknumlist.size();
    for (uint32_t i=0; i<m_memblocknumlistsize; i++)
    {
        m_memblocknumlist[i] = memblocknumlist[i].asInt();
        MyThrowAssert(m_memblocknumlist[i] > 0);
    }
    MyThrowAssert(m_cell_size > 0 && m_bucket_size > 0 && m_headlist_size > 0 && m_memblocknumlistsize > 0);

    m_pnlp_processor = new nlp_processor();
    m_pindex_group   = new index_group(m_cell_size, m_bucket_size,
            m_headlist_size, m_memblocknumlist, m_memblocknumlistsize);
    m_del_bitmap = new bitmap(m_attr_data_dir, "del_bitmap", m_max_inner_id/8); // 一个字节 = 8 个bit
    m_mod_bitmap = new bitmap(m_attr_data_dir, "mod_bitmap", m_max_inner_id/8);
    m_idmap      = new idmap(m_idmap_data_dir, m_max_outer_id, m_max_inner_id);
    m_docattr_bitlist = NULL;
}

secore:: ~secore()
{
    delete m_del_bitmap;
    delete m_mod_bitmap;
    delete m_pindex_group;
    delete m_idmap;
    delete m_pnlp_processor;
    delete m_docattr_bitlist;
}
