#include "flexse_plugin.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
#include <string.h>
using namespace std;

const char* const flexse_plugin:: CONFIGCATEGORY_FLEXINDEX      = "FLEXINDEX";
const char* const flexse_plugin:: FLEXINDEX_KEY_OP              = "op";
const char* const flexse_plugin:: FLEXINDEX_KEY_TYPE            = "type";
const char* const flexse_plugin:: FLEXINDEX_KEY_TOKEN           = "token";
const char* const flexse_plugin:: FLEXINDEX_KEY_FIELD           = "field";
const char* const flexse_plugin:: CONFIGCATEGORY_STRUCTMASK     = "STRUCTMASK";
const char* const flexse_plugin:: STRUCTMASK_POST               = "posting_list_cell";
const char* const flexse_plugin:: STRUCTMASK_ATTR               = "document_attribute";
const char* const flexse_plugin:: FLEXINDEX_VALUE_OP_NLP        = "NLP";
const char* const flexse_plugin:: FLEXINDEX_VALUE_OP_DOC_ID     = "DOC_ID";
const char* const flexse_plugin:: FLEXINDEX_VALUE_OP_PREFIX     = "PREFIX";
const char* const flexse_plugin:: FLEXINDEX_VALUE_TYPE_INT      = "INT";
const char* const flexse_plugin:: FLEXINDEX_VALUE_TYPE_STR      = "STR";
const char* const flexse_plugin:: FLEXINDEX_VALUE_TYPE_LIST_INT = "LIST_INT";
const char* const flexse_plugin:: FLEXINDEX_VALUE_TYPE_LIST_STR = "LIST_STR";

flexse_plugin:: flexse_plugin(const char* config_path, secore* insecore)
{
    MySuicideAssert (config_path != NULL && insecore != NULL);
    mysecore = insecore;
    // PLUGIN CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(config_path);
    MySuicideAssert (reader.parse(in, root));

    m_post_maskmap = new structmask(root[CONFIGCATEGORY_STRUCTMASK][STRUCTMASK_POST]);
    m_attr_maskmap = new structmask(root[CONFIGCATEGORY_STRUCTMASK][STRUCTMASK_ATTR]);

    Json::Value flexindex = root[CONFIGCATEGORY_FLEXINDEX];
    MySuicideAssert (!flexindex.isNull());
    Json::Value::const_iterator iter;          //迭代器
    iter = flexindex.begin();

    for (uint32_t i=0; i<flexindex.size(); i++)
    {
        key_op_t mkey_op;
        memset(&mkey_op, 0, sizeof(mkey_op));

        Json::Value field = flexindex[i];
        Json::Value::Members member=(*iter).getMemberNames();

        string strkey(*(member.begin()));
        MyThrowAssert(m_key_op_map.end() == m_key_op_map.find(strkey));
        iter++;

        // 解析处置的方法
        Json::Value value = field[strkey.c_str()];
        MyThrowAssert(!value.isNull() && value[FLEXINDEX_KEY_OP].isString());
        if (0 == strcmp(value[FLEXINDEX_KEY_OP].asCString(), FLEXINDEX_VALUE_OP_DOC_ID))
        {
            // 该字段作为ID处理
            mkey_op.op   = OP_DOC_ID;
            mkey_op.type = T_INT;
            MyThrowAssert(value[FLEXINDEX_KEY_FIELD].isString());
            MyThrowAssert(0 == m_post_maskmap->get_mask_item(value[FLEXINDEX_KEY_FIELD].asCString(), &mkey_op.key_mask));
        }
        else if (0 == strcmp(value[FLEXINDEX_KEY_OP].asCString(), FLEXINDEX_VALUE_OP_NLP))
        {
            // 该字段要做分词处理
            mkey_op.op   = OP_NLP;
            mkey_op.type = T_STR;
        }
        else if (0 == strcmp(value[FLEXINDEX_KEY_OP].asCString(), FLEXINDEX_VALUE_OP_PREFIX))
        {
            // 该字段要做前缀
            mkey_op.op   = OP_PREFIX;
            MyThrowAssert(value[FLEXINDEX_KEY_TYPE].isString());
            if (0 == strcmp(value[FLEXINDEX_KEY_TYPE].asCString(), FLEXINDEX_VALUE_TYPE_INT))
            {
                mkey_op.type = T_INT;
            }
            else if (0 == strcmp(value[FLEXINDEX_KEY_TYPE].asCString(), FLEXINDEX_VALUE_TYPE_STR))
            {
                mkey_op.type = T_STR;
            }
            else if (0 == strcmp(value[FLEXINDEX_KEY_TYPE].asCString(), FLEXINDEX_VALUE_TYPE_LIST_INT))
            {
                mkey_op.type = T_LIST_INT;
            }
            else if (0 == strcmp(value[FLEXINDEX_KEY_TYPE].asCString(), FLEXINDEX_VALUE_TYPE_LIST_STR))
            {
                mkey_op.type = T_LIST_STR;
            }
            else
            {
                FATAL("unknown type type[%s]", value[FLEXINDEX_KEY_TYPE].asCString());
                MySuicideAssert(0);
            }
        }
        else
        {
            FATAL("unknown op type[%s]", value[FLEXINDEX_KEY_OP].asCString());
            MySuicideAssert(0);
        }

        m_key_op_map[strkey] = mkey_op;
    }
}

flexse_plugin:: ~flexse_plugin()
{
    delete m_post_maskmap;
    delete m_attr_maskmap;
}

int flexse_plugin:: add(const char* jsonstr, uint32_t& doc_id, vector<term_info_t> & termlist)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }

    if (root[FLEXINDEX_VALUE_OP_DOC_ID].isNull()
            || ! root[FLEXINDEX_VALUE_OP_DOC_ID].isInt()
            || 0 == root[FLEXINDEX_VALUE_OP_DOC_ID].asInt())
    {
        ALARM("jsonstr NOT contain 'DOC_ID' or Type error.");
        return -1;
    }
    else
    {
        doc_id = root[FLEXINDEX_VALUE_OP_DOC_ID].asInt();
    }

    // 从 m_key_op_map 迭代处理
    map<string, key_op_t>::iterator it;
    for (it=m_key_op_map.begin(); it!=m_key_op_map.end(); it++)
    {
        if (root[it->first.c_str()].isNull())
        {
            ALARM("jsonstr NOT contain '%s'.", it->first.c_str());
            return -1;
        }
    }
    // 从 m_attr_maskmap 迭代处理

    if (root["CONTENT"].isNull() || !root["CONTENT"].isString())
    {
        ALARM("jsonstr NOT contain 'CONTENT'.");
        return -1;
    }
    else
    {
        mysecore->m_pnlp_processor->split((char*)root["CONTENT"].asCString(), termlist);
    }

    return 0;
}

int flexse_plugin:: mod(const char* jsonstr, uint32_t& doc_id, vector<term_info_t> & termlist)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }

    if (root[FLEXINDEX_VALUE_OP_DOC_ID].isNull())
    {
        ALARM("jsonstr NOT contain 'DOC_ID'.");
        return -1;
    }
    else
    {
        doc_id = root[FLEXINDEX_VALUE_OP_DOC_ID].asInt();
    }

    if (root["CONTENT"].isNull() || !root["CONTENT"].isString())
    {
        ALARM("jsonstr NOT contain 'CONTENT'.");
        return -1;
    }
    else
    {
        mysecore->m_pnlp_processor->split((char*)root["CONTENT"].asCString(), termlist);
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
    Json::Value json_idlist = root[secore::m_StrInsideKey_DocIDList];
    if(json_idlist.isNull() || !json_idlist.isArray())
    {
        ALARM("Not a list jsonstr[%s]", jsonstr);
        return -1;
    }
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
