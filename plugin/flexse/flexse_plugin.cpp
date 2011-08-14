#include "flexse_plugin.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
using namespace std;

const char* const flexse_plugin::m_StrInsideKey_DocIDList = "id_list";

const char* const flexse_plugin::CONFIGCATEGORY_INDEXDESC = "INDEXDESC";
const char* const flexse_plugin::CONFIGCATEGORY_FLEXINDEX = "FLEXINDEX";
const char* const flexse_plugin::CONFIGCATEGORY_ATTR      = "ATTR";
const char* const flexse_plugin::CONFIGCATEGORY_NLP       = "NLP";
const char* const flexse_plugin::CONFIGCATEGORY_GENERAL   = "GENERAL";

const char* const flexse_plugin::m_StrMaxDocID = "MaxDocID";
const char* const flexse_plugin::m_StrAttrDataDir  = "AttrDataDir";

const char* const flexse_plugin::m_StrCellSize = "PostingListCellSize";
const char* const flexse_plugin::m_StrBucketSize = "PostingBucketSize";
const char* const flexse_plugin::m_StrHeadListSize = "PostingHeadListSize";
const char* const flexse_plugin::m_StrMemBlockNumList = "PostingMemBlockNumList";

flexse_plugin:: flexse_plugin(const char* config_path)
{
    // PLUGIN CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(config_path);
    MyThrowAssert (reader.parse(in, root));

    // GENERAL CONFIG
    Json::Value generalConfig = root[CONFIGCATEGORY_GENERAL];
    MyThrowAssert (! generalConfig.isNull());

    MyThrowAssert (! generalConfig[m_StrMaxDocID].isNull());
    m_max_doc_id = generalConfig[m_StrMaxDocID].asInt();

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
        ROUTN("[%u] num[%u]", i, m_memblocknumlist[i]);
        MyThrowAssert(m_memblocknumlist[i] > 0);
    }
    MyThrowAssert(m_cell_size > 0 && m_bucket_size > 0 && m_headlist_size > 0 && m_memblocknumlistsize > 0);

    m_pnlp_processor = new nlp_processor();
    m_pindex_group   = new index_group(m_cell_size, m_bucket_size,
            m_headlist_size, m_memblocknumlist, m_memblocknumlistsize);
    m_del_bitmap = new bitmap(m_attr_data_dir, "del_bitmap", m_max_doc_id/8);
    m_mod_bitmap = new bitmap(m_attr_data_dir, "mod_bitmap", m_max_doc_id/8);
}

int flexse_plugin:: add(const char* jsonstr, uint32_t& doc_id, vector<string> & vstr)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }

    if (root["DOC_ID"].isNull())
    {
        ALARM("jsonstr NOT contain 'DOC_ID'.");
        return -1;
    }
    else
    {
        doc_id = root["DOC_ID"].asInt();
    }

    if (root["CONTENT"].isNull() || !root["CONTENT"].isString())
    {
        ALARM("jsonstr NOT contain 'CONTENT'.");
        return -1;
    }
    else
    {
        m_pnlp_processor->split((char*)root["CONTENT"].asCString(), vstr);
    }

    return 0;
}

int flexse_plugin:: mod(const char* jsonstr, uint32_t& doc_id, vector<string> & vstr)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }

    if (root["DOC_ID"].isNull())
    {
        ALARM("jsonstr NOT contain 'DOC_ID'.");
        return -1;
    }
    else
    {
        doc_id = root["DOC_ID"].asInt();
    }

    if (root["CONTENT"].isNull() || !root["CONTENT"].isString())
    {
        ALARM("jsonstr NOT contain 'CONTENT'.");
        return -1;
    }
    else
    {
        m_pnlp_processor->split((char*)root["CONTENT"].asCString(), vstr);
    }

    return 0;
}

int flexse_plugin:: del(const char* jsonstr, vector<uint32_t> & id_list)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }

    id_list.clear();
    Json::Value json_idlist = root[m_StrInsideKey_DocIDList];
    MyThrowAssert (!json_idlist.isNull() && !json_idlist.isArray());
    const uint32_t json_idlist_size = json_idlist.size();
    for (uint32_t i=0; i<json_idlist_size; i++)
    {
        id_list.push_back(json_idlist[i].asInt());
    }

    return 0;
}

int flexse_plugin:: undel(const char* jsonstr, vector<uint32_t> & id_list)
{
    return del(jsonstr, id_list);
}

index_group* flexse_plugin:: getIndexGroup()
{
    return m_pindex_group;
}
