#include "flexse_plugin.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
using namespace std;

const char* const flexse_plugin:: CONFIGCATEGORY_FLEXINDEX  = "FLEXINDEX";
const char* const flexse_plugin:: CONFIGCATEGORY_STRUCTMASK = "STRUCTMASK";

flexse_plugin:: flexse_plugin(const char* config_path, secore* insecore)
{
    MySuicideAssert (config_path != NULL && insecore != NULL);
    mysecore = insecore;
    // PLUGIN CONFIG
    Json::Value root;
    Json::Reader reader;
    ifstream in(config_path);
    MySuicideAssert (reader.parse(in, root));

    Json::Value flexindex = root[CONFIGCATEGORY_FLEXINDEX];
    MySuicideAssert (!flexindex.isNull());
    Json::Value::const_iterator iter;          //迭代器
    iter = flexindex.begin();

    printf ("size: %d\n", flexindex.size());          // 输出 key1,key2
    for (uint32_t i=0; i<flexindex.size(); i++)
    {
        Json::Value field = flexindex[i];
        Json::Value::Members member=(*iter).getMemberNames();

        string strkey(*(member.begin()));
        printf ("k: %s\n", strkey.c_str());          // 输出 key1,key2
        iter++;
    }

//    for(iter = flexindex.begin();iter != flexindex.end();iter++ )
//    {
//        Json::Value::Members member=(*iter).getMemberNames();
//        printf ("k: %s\n", string(*(member.begin())).c_str());          // 输出 key1,key2
//        //        (*iter)[*(member.begin())];     //输出 value1,value2
//    }
}

flexse_plugin:: ~flexse_plugin()
{
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

    if (root["DOC_ID"].isNull() || ! root["DOC_ID"].isInt() || 0 == root["DOC_ID"].asInt())
    {
        ALARM("jsonstr NOT contain 'DOC_ID' or Type error.");
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
