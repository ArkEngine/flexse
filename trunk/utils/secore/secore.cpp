#include "secore.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
using namespace std;

const char* const secore::CONFIGCATEGORY_STRUCTMASK = "STRUCTMASK";
const char* const secore::STRUCTMASK_POST           = "posting_list_cell";
const char* const secore::STRUCTMASK_ATTR           = "document_attribute";

const char* const secore::m_StrInsideKey_DocIDList = "id_list";
const char* const secore::m_StrInsideKey_DocID     = "id";

const char* const secore::CONFIGCATEGORY_INDEXCONFIG = "INDEXCONFIG";
const char* const secore::CONFIGCATEGORY_FLEXINDEX   = "FLEXINDEX";
const char* const secore::CONFIGCATEGORY_ATTR        = "ATTR";
const char* const secore::CONFIGCATEGORY_DETAIL      = "DETAIL";
const char* const secore::CONFIGCATEGORY_NLP         = "NLP";
const char* const secore::CONFIGCATEGORY_GENERAL     = "GENERAL";

const char* const secore::m_StrMaxOuterID   = "MaxOuterID";
const char* const secore::m_StrMaxInnerID   = "MaxInnerID";
const char* const secore::m_StrIdmapDataDir = "IdmapDataDir";
const char* const secore::m_StrAttrDataDir  = "AttrDataDir";

const char* const secore::m_StrDetailDataDir  = "DetailDataDir";

const char* const secore::m_StrCellSize = "PostingListCellSize";
const char* const secore::m_StrBucketSize = "PostingBucketSize";
const char* const secore::m_StrHeadListSize = "PostingHeadListSize";
const char* const secore::m_StrMemBlockNumList = "PostingMemBlockNumList";

secore:: secore(const char* plugin_config_path)
{
    // PLUGIN CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(plugin_config_path);
    MySuicideAssert (reader.parse(in, root));

    // GENERAL CONFIG
    Json::Value generalConfig = root[CONFIGCATEGORY_GENERAL];
    MySuicideAssert (! generalConfig.isNull());

    MySuicideAssert (! generalConfig[m_StrMaxInnerID].isNull());
    m_max_inner_id = generalConfig[m_StrMaxInnerID].asInt();
    MySuicideAssert(m_max_inner_id > 0);

    MySuicideAssert (! generalConfig[m_StrMaxOuterID].isNull());
    m_max_outer_id = generalConfig[m_StrMaxOuterID].asInt();
    MySuicideAssert(m_max_outer_id > 0);

    MySuicideAssert (! generalConfig[m_StrIdmapDataDir].isNull());
    snprintf(m_idmap_data_dir, sizeof(m_idmap_data_dir), "%s", generalConfig[m_StrIdmapDataDir].asCString());

    // ATTR CONFIG
    Json::Value attrConfig = root[CONFIGCATEGORY_ATTR];
    MySuicideAssert (! attrConfig.isNull());

    MySuicideAssert (! attrConfig[m_StrAttrDataDir].isNull());
    snprintf(m_attr_data_dir, sizeof(m_attr_data_dir), "%s", attrConfig[m_StrAttrDataDir].asCString());

    // DETAIL CONFIG
    Json::Value detailConfig = root[CONFIGCATEGORY_DETAIL];
    MySuicideAssert (! detailConfig.isNull());

    MySuicideAssert (! detailConfig[m_StrDetailDataDir].isNull());
    snprintf(m_detail_data_dir, sizeof(m_detail_data_dir), "%s", detailConfig[m_StrDetailDataDir].asCString());

    // INDEXDESC CONFIG
    Json::Value indexDescConfig = root[CONFIGCATEGORY_INDEXCONFIG];
    MySuicideAssert (! indexDescConfig.isNull());

    MySuicideAssert (! indexDescConfig[m_StrCellSize].isNull());
    m_cell_size = indexDescConfig[m_StrCellSize].asInt();
    MySuicideAssert (! indexDescConfig[m_StrBucketSize].isNull());
    m_bucket_size = indexDescConfig[m_StrBucketSize].asInt();
    MySuicideAssert (! indexDescConfig[m_StrHeadListSize].isNull());
    m_headlist_size = indexDescConfig[m_StrHeadListSize].asInt();
    Json::Value memblocknumlist = indexDescConfig[m_StrMemBlockNumList];
    MySuicideAssert (!memblocknumlist.isNull() && memblocknumlist.isArray());
    m_memblocknumlistsize = memblocknumlist.size();
    for (uint32_t i=0; i<m_memblocknumlistsize; i++)
    {
        m_memblocknumlist[i] = memblocknumlist[i].asInt();
        MySuicideAssert(m_memblocknumlist[i] > 0);
    }
    MySuicideAssert(m_cell_size > 0 && m_bucket_size > 0 && m_headlist_size > 0 && m_memblocknumlistsize > 0);

    m_pnlp_processor = new nlp_processor();
    m_pindex_group   = new index_group(m_cell_size, m_bucket_size,
            m_headlist_size, m_memblocknumlist, m_memblocknumlistsize);
    m_del_bitmap = new bitmap(m_attr_data_dir, "del_bitmap", m_max_inner_id/8); // 一个字节 = 8 个bit
    m_mod_bitmap = new bitmap(m_attr_data_dir, "mod_bitmap", m_max_inner_id/8);
    m_idmap      = new idmap(m_idmap_data_dir, m_max_outer_id, m_max_inner_id);
    m_detaildb   = new detaildb(m_detail_data_dir, "detail");

    // 读取插件的配置
//    Json::Value rootPlugin;
//    Json::Reader readerPlugin;
//    ifstream inPlugin(plugin_config_path);
//    MySuicideAssert (readerPlugin.parse(in, root));
    m_attr_maskmap = new structmask(root[CONFIGCATEGORY_STRUCTMASK][STRUCTMASK_ATTR]);
    m_post_maskmap = new structmask(root[CONFIGCATEGORY_STRUCTMASK][STRUCTMASK_POST]);
    m_docattr_bitlist = new bitlist(m_attr_data_dir, "attr_bitlist", m_attr_maskmap->get_section_size(), m_max_inner_id);
}

secore:: ~secore()
{
    delete m_del_bitmap;
    delete m_mod_bitmap;
    delete m_pindex_group;
    delete m_idmap;
    delete m_pnlp_processor;
    delete m_docattr_bitlist;
    delete m_detaildb;
    delete m_post_maskmap;
    delete m_attr_maskmap;
}
