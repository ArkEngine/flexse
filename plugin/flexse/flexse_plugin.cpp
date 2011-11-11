#include "flexse_plugin.h"
#include "MyException.h"
#include "algo.h"
#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
using namespace std;

const char* const flexse_plugin:: CONFIGCATEGORY_FLEXINDEX      = "FLEXINDEX";
const char* const flexse_plugin:: FLEXINDEX_KEY_OP              = "op";
const char* const flexse_plugin:: FLEXINDEX_KEY_TYPE            = "type";
const char* const flexse_plugin:: FLEXINDEX_KEY_TOKEN           = "token";
const char* const flexse_plugin:: FLEXINDEX_KEY_FIELD           = "field";
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

    m_post_maskmap = insecore->m_post_maskmap;
    m_attr_maskmap = insecore->m_attr_maskmap;

    Json::Value flexindex = root[CONFIGCATEGORY_FLEXINDEX];
    MySuicideAssert (!flexindex.isNull());
    Json::Value::const_iterator iter;          //迭代器
    iter = flexindex.begin();

    // 读取倒排索引的字段处置对应表
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
            MyThrowAssert(value[FLEXINDEX_KEY_TOKEN].isString());
            MyThrowAssert(0 < snprintf(mkey_op.token, sizeof(mkey_op.token), "%s", value[FLEXINDEX_KEY_TOKEN].asCString()));
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
}

// 对传入的字符串进行解析
// doc_id赋值为外部ID
// termlist中存放倒排索引描述信息，也就是分词之后的term及term描述信息列表
// attrlist中存放文档属性信息
int flexse_plugin:: add(const char* jsonstr, uint32_t& doc_id,
        map<string, term_info_t> & term_map, vector<attr_field_t>& attrlist)
{
    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return -1;
    }

    doc_id = 0;

    // 从 m_key_op_map 迭代处理
    map<string, key_op_t>::iterator it;
    term_map.clear();
    for (it=m_key_op_map.begin(); it!=m_key_op_map.end(); it++)
    {
        const char* strkey = it->first.c_str();
        if (root[strkey].isNull())
        {
            ALARM("jsonstr NOT contain '%s'.", strkey);
            return -1;
        }
        switch (it->second.op)
        {
            case OP_DOC_ID:
                if (!root[strkey].isInt())
                {
                    ALARM("'%s' is NOT Int.", strkey);
                    return -1;
                }
                doc_id = root[strkey].asInt();
                break;
            case OP_PREFIX:
                char tmpstr[128];
                if (it->second.type == T_INT)
                {
                    snprintf(tmpstr, sizeof(tmpstr), "%s%u", it->second.token, root[strkey].asInt());
                    term_info_t term_info = {tmpstr, 0, {0,0,0,0}};
                    if (term_map.find(tmpstr) == term_map.end())
                    {
                        term_map[tmpstr] = term_info;
                    }
                    else
                    {
                        // PREFIX应该不会重复吧
                    }
                }
                else if (it->second.type == T_STR)
                {
                    snprintf(tmpstr, sizeof(tmpstr), "%s%s", it->second.token, root[strkey].asCString());
                    term_info_t term_info = {tmpstr, 0, {0,0,0,0}};
                    if (term_map.find(tmpstr) == term_map.end())
                    {
                        term_map[tmpstr] = term_info;
                    }
                    else
                    {
                        // PREFIX应该不会重复吧
                    }
                }
                else if (it->second.type == T_LIST_INT)
                {
                    if( root[strkey].isNull() || !root[strkey].isArray())
                    {
                        ALARM("'%s' Not a list jsonstr[%s]", strkey, jsonstr);
                        return -1;
                    }
                    const uint32_t json_idlist_size = root[strkey].size();
                    for (uint32_t i=0; i<json_idlist_size; i++)
                    {
                        snprintf(tmpstr, sizeof(tmpstr), "%s%u", it->second.token, root[strkey][i].asInt());
                        term_info_t term_info = {tmpstr, 0, {0,0,0,0}};
                        if (term_map.find(tmpstr) == term_map.end())
                        {
                            term_map[tmpstr] = term_info;
                        }
                    }
                }
                else if (it->second.type == T_LIST_STR)
                {
                    if( root[strkey].isNull() || !root[strkey].isArray())
                    {
                        ALARM("'%s' Not a list jsonstr[%s]", strkey, jsonstr);
                        return -1;
                    }
                    const uint32_t json_idlist_size = root[strkey].size();
                    for (uint32_t i=0; i<json_idlist_size; i++)
                    {
                        snprintf(tmpstr, sizeof(tmpstr), "%s%s", it->second.token, root[strkey][i].asCString());
                        term_info_t term_info = {tmpstr, 0, {0,0,0,0}};
                        if (term_map.find(tmpstr) == term_map.end())
                        {
                            term_map[tmpstr] = term_info;
                        }
                    }
                }
                else
                {
                    MySuicideAssert(0);
                }
                break;
            case OP_NLP:
                // TODO 这里应该考虑有些field的文本需要存储offset
                // offset 暂定只计算标题吧，这样计算的效率可以得到保证
                if (root[strkey].isNull() || !root[strkey].isString())
                {
                    ALARM("jsonstr NOT contain '%s' [%u].", strkey, root[strkey].isNull());
                    return -1;
                }
                else
                {
                    // 在分词的算法中，设置各个term_info_t的描述，比如tf/idf/weight/offset
                    //                    mysecore->m_pnlp_processor->split((char*)root[strkey].asCString(), termlist);
                    //                    PRINT("txt:[%s] siz[%u]", root[strkey].asCString(), root[strkey].size());

                    TokenListPtr tokens = mysecore->segmenter->Segment(root[strkey].asCString(), (int)strlen(root[strkey].asCString()),
                            SEMANTIC_TOKEN|RETRIEVAL_TOKEN, "utf8");
                    TokenPtr token = tokens->first();

                    while (token != NULL) {

//                        printf("[%s] -> [%s]\n", root[strkey].asCString(), token->as_string().c_str());
                        if (token->as_string().length() && term_map.find(token->as_string()) == term_map.end())
                        {
                            term_info_t term_info = {token->as_string(), 0, {0,0,0,0}};
                            term_map[token->as_string()] = term_info;
                            TokenPtr son = token->sub_token_;
                            while (son != NULL) {
                                if (son->as_string().length() && term_map.end() == term_map.find(son->as_string()))
                                {                   
//                                    printf("[%s] -> [%s] -> [%s]\n",
//                                            root[strkey].asCString(), token->as_string().c_str(), son->as_string().c_str());
                                    term_info_t sub_term_info = {son->as_string(), 0, {0,0,0,0}};
                                    term_map[son->as_string()] = term_info;
                                }                                           
                                else
                                {
                                    // TODO
                                    // 重复的情况需要额外处理
                                }
                                // PRINT("son[%s]", son->as_string().c_str());
                                son = son->next_;
                            }

                        }
                        else
                        {
                            // ranking同学请注意 TODO
                            // 如果重复了，这里需要另外的处理，比如设置field_hit，或者offset信息
                        }
                        token = token->next_;
                    }
                }
                break;
            default:
                MySuicideAssert(0);
                break;
        }
    }
    // 看看都分啥了
//    map<string, term_info_t>::iterator iit;
//    for (iit=term_map.begin(); iit!=term_map.end(); iit++)
//    {
//        printf("<%s> ", iit->first.c_str());
//    }
//    printf("\n");
//    printf("[%s]\n", jsonstr);

    // 迭代 m_attr_maskmap , 保存文档属性的数据
    char key[128];
    mask_item_t key_mask;
    attrlist.clear();
    for(m_attr_maskmap->begin(); !m_attr_maskmap->is_end(); m_attr_maskmap->next())
    {
        m_attr_maskmap->itget(key, sizeof(key), &key_mask);
        //        PRINT("key[%s] uint_off[%u] item_mask[0x%08x] move_count[%02u] uint32_count[%02u]\n",
        //                key,
        //                key_mask.uint_offset, 
        //                key_mask.item_mask,
        //                key_mask.move_count,
        //                key_mask.uint32_count);
        if (root[key].isInt())
        {
            attr_field_t attr_field;
            attr_field.value = root[key].asInt();
            attr_field.key_mask = key_mask;
            attrlist.push_back(attr_field);
        }
    }

    return 0;
}

int flexse_plugin:: mod(const char* jsonstr, uint32_t& doc_id,
        map<string, term_info_t> & term_map, vector<attr_field_t>& attrlist)
{
    return add(jsonstr, doc_id, term_map, attrlist);
}

// 取得外部ID列表即可，剩下的置位操作交给框架
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

int flexse_plugin:: query (query_param_t* query_param, char* retBuff, const uint32_t retBuffSize)
{
    MySuicideAssert(retBuff != NULL && retBuffSize > 0);

    mask_item_t doc_id_mask;
    mask_item_t weight_mask;
    // -0- 先获得倒排索引描述信息中，'id'和'weight'的structmask信息
    MySuicideAssert( 0 == mysecore->m_post_maskmap->get_mask_item("id", &doc_id_mask));
    MySuicideAssert( 0 == mysecore->m_post_maskmap->get_mask_item("weight", &weight_mask));
    // -1- 执行归并算法
    // 归并的时候，该postinglist的综合term得分=(term权重*term系数)
    // 如果term权重最小为1，如果为0，自动设为1
    vector<result_pair_t> merged_vector;
    // 保留term归并时权重最高的2000个结果
    const uint32_t MAX_MERGE_KEEP_NUM = 2000;
    weight_merge(query_param->term_vector, doc_id_mask, weight_mask,
            merged_vector, MAX_MERGE_KEEP_NUM);
    int32_t merge_rst_num = (int32_t)merged_vector.size();
    PRINT("merged_num[%d]", merge_rst_num);
    // -2- 过滤掉修改过的和删除过的文档
    uint32_t fcount = 0;
    for (int i=0; i<merge_rst_num; i++)
    {
        //        printf("[%u]-m[%u]-d[%u] ", merged_vector[i].id,
        //                _GET_BITMAP_(*(mysecore->m_mod_bitmap), merged_vector[i].id),
        //                _GET_BITMAP_(*(mysecore->m_del_bitmap), merged_vector[i].id));
        if ((0 == _GET_BITMAP_(*(mysecore->m_mod_bitmap), merged_vector[i].id))
                && (0 == _GET_BITMAP_(*(mysecore->m_del_bitmap), merged_vector[i].id)))
        {
            merged_vector[fcount++] = merged_vector[i];
        }
    }
    merged_vector.resize(fcount);
    //    printf("\n");

    // -3- 对归并结果数组做用户自定义的过滤操作，如时长，上传时间，类型等等
    //     过滤的时机是可以优化性能的，当要求全名中时，先对最短的拉链过滤一下，可能获得性能上的提升

    // filter已经对vector做了resize()的操作
    filter( merged_vector, mysecore->m_docattr_bitlist->puint, query_param->filt_vector);
    uint32_t filter_num = (uint32_t)merged_vector.size();
    PRINT("filter_num[%u]", filter_num);

    // -4- 调权算法
    // algo中提供了ranking的灵活算法实现，可以对(等于/小于/大于/区间/集合)的情况进行加权

    // -5- 其他算法
    //     [1] gourp by field，暂未实现
    //     [2] order by field，algo中提供了按照某个属性选取TOP-N的排序结果的算法实现
    //     [3] limit begin, end
    // -6- 把文档属性复制到返回结果数组中
    const uint32_t attr_cell_char_size = (uint32_t)(mysecore->m_docattr_bitlist->m_cellsize * sizeof(uint32_t));
    uint32_t* puint = (uint32_t*)retBuff;
    uint32_t retnum = (filter_num*attr_cell_char_size > retBuffSize) ? retBuffSize/attr_cell_char_size : filter_num;
    query_param->all_num = retnum;
    PRINT("all_num[%u] attr_cell_size[%u]", query_param->all_num, mysecore->m_docattr_bitlist->m_cellsize);
    for (uint32_t i=0; i<retnum; i++)
    {
        char* srcBuf = (char*)(mysecore->m_docattr_bitlist->puint + merged_vector[i].id*mysecore->m_docattr_bitlist->m_cellsize);
        memmove(puint, srcBuf, attr_cell_char_size);
        puint += mysecore->m_docattr_bitlist->m_cellsize;
    }

    if (query_param->orderby.length() > 0)
    {
        mask_item_t sort_key_mask;
        if ( 0 == mysecore->m_attr_maskmap->get_mask_item(query_param->orderby.c_str(), &sort_key_mask))
        {
            field_partial_sort(retBuff, query_param->all_num, sort_key_mask, query_param->all_num);
//            for (uint32_t i=0; i<query_param->all_num; i++)
//            {
//                uint32_t _value = _GET_LIST_VALUE_(retBuff, i, sort_key_mask);
//                PRINT("orderby : %s value[%3u]", query_param->orderby.c_str(), _value);
//            }
        }
        else
        {
            PRINT("sort field NOT found[%s]", query_param->orderby.c_str());
        }
    }

    uint32_t begin_no = query_param->offset;
    uint32_t end_no = begin_no + query_param->size;
    uint32_t ret_count = 0;
    puint = (uint32_t*)retBuff;
    for (uint32_t i=begin_no; i<end_no && i<query_param->all_num; i++)
    {
        void* srcBuf = (((uint32_t*)retBuff) + i*mysecore->m_docattr_bitlist->m_cellsize);
        memmove(puint, srcBuf, attr_cell_char_size);
        puint += mysecore->m_docattr_bitlist->m_cellsize;
        ret_count ++;
    }

    return (int32_t)(ret_count*attr_cell_char_size);
}
