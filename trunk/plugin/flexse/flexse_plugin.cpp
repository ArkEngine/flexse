#include "flexse_plugin.h"
#include "MyException.h"
#include <iostream>
#include <fstream>
using namespace std;

flexse_plugin:: flexse_plugin(const char* config_path, secore* insecore)
{
    MyThrowAssert (config_path != NULL && insecore != NULL);
    mysecore = insecore;
}

flexse_plugin:: ~flexse_plugin()
{
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
        mysecore->m_pnlp_processor->split((char*)root["CONTENT"].asCString(), vstr);
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
        mysecore->m_pnlp_processor->split((char*)root["CONTENT"].asCString(), vstr);
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
