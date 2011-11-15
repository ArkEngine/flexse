#include <stdio.h>
#include <json/json.h>
#include <sys/time.h>
#include "thread_data.h"
#include "query_thread.h"
#include "config.h"
#include "structmask.h"
#include "mylog.h"
#include "algo.h"
#include "secore.h"
#include "index_group.h"
#include "xhead.h"
#include "myutils.h"
#include <string.h>

#define TIME_US_COST(pre, cur) (((cur.tv_sec)-(pre.tv_sec))*1000000 + \
		        (cur.tv_usec) - (pre.tv_usec))
 
extern Config* myConfig;

// 读取termlist及其对应的postinglist，每个term可以有权重
int get_term_list(
        Json::Value& root,
        secore* mysecore,
        char* tmpBuff,
        const uint32_t tmpBuffSize,
        vector<list_info_t>& term_vector
        )
{
    // term的数据格式
    // [
    //   ["term" : "a", "weight": 80, "synonyms": ["A", "A'"]],
    //   ["term" : "b", "weight": 20, "synonyms": ["B"]]
    // ]
    // 处理结果是一个vector<list_info_t>
    // 第0个list_info_t中，weight是80，postinglist是(a|A|A')
    // 第1个list_info_t中，weight是20，postinglist是(b|B)
    // 在执行(a|A|A')的过程中，遇到重复的保留前面的元素

    // 用'id'的structmask得到单位index_t的大小，用于copy内存
    mask_item_t doc_id_mask;

    MySuicideAssert( 0 == mysecore->m_post_maskmap->get_mask_item("id", &doc_id_mask));
    if (root[QUERY_KEY_TERMLIST].isNull() || !root[QUERY_KEY_TERMLIST].isArray() || root[QUERY_KEY_TERMLIST].size() <= 0)
    {
        // 不是数组，或者数组为空
        ALARM("termlist is empty.");
        return -1;
    }
    else
    {
        Json::Value termlist = root[QUERY_KEY_TERMLIST];

        Json::Value::const_iterator iter;
        iter = termlist.begin();
        index_group* myIndexGroup = mysecore->m_pindex_group;
        for (uint32_t i=0; i<termlist.size(); i++)
        {
            Json::Value term = termlist[i];
            if (term[QUERY_KEY_TERMLIST_TERM].isNull() || !term[QUERY_KEY_TERMLIST_TERM].isString()
                    || term[QUERY_KEY_TERMLIST_WEIGHT].isNull() || !term[QUERY_KEY_TERMLIST_WEIGHT].isInt())
            {
                ALARM("termlist[%u] error.", i);
                return -1;
            }
            list_info_t term_info;
            term_info.weight = term[QUERY_KEY_TERMLIST_WEIGHT].asInt();
            term_info.weight = term_info.weight ? term_info.weight : 1;
            // 根据term读取postinglist
            // 需要考虑内存分配
            // 先用一个临时的buffer存储拉链，然后得到大小，copy到合适的内存中
            int list_num = myIndexGroup->get_posting_list(
                    term[QUERY_KEY_TERMLIST_TERM].asCString(),
                    tmpBuff,
                    tmpBuffSize);
            term_info.list_size = list_num <= 0? 0 : list_num;
            // 没初始化有个崩溃的bug，要初始化啊!!!
            term_info.posting_list = NULL;
            if ( term_info.list_size > 0)
            {
                char* dst_index_buff = (char*) malloc(term_info.list_size*doc_id_mask.uint32_count*sizeof(uint32_t));
                if (dst_index_buff == NULL)
                {
                    FATAL("malloc for index[%s] [%u] failed.",
                            term[QUERY_KEY_TERMLIST_TERM].asCString(),
                            term_info.list_size*doc_id_mask.uint32_count*sizeof(uint32_t)
                            );
                    term_info.posting_list = NULL;
                    term_info.list_size = 0;
                }
                else
                {
                    memmove(dst_index_buff, tmpBuff, term_info.list_size * doc_id_mask.uint32_count * sizeof(uint32_t));
                    term_info.posting_list = dst_index_buff;
                }
            }
            PRINT("term[%s] has SYNONYMS[%u] isNull[%u]",
                    term[QUERY_KEY_TERMLIST_TERM].asCString(),
                    term[QUERY_KEY_SYNONYMS_LIST].isArray(),
                    term[QUERY_KEY_SYNONYMS_LIST].isNull());
            // 处理同义词，把所有同义词的postinglist全部以or的方式merge起来
            if (!term[QUERY_KEY_SYNONYMS_LIST].isNull() && term[QUERY_KEY_SYNONYMS_LIST].isArray())
            {
                Json::Value synonymslist = term[QUERY_KEY_SYNONYMS_LIST];
                Json::Value::iterator synonyms_iter = synonymslist.begin();
                vector<list_info_t> list_info;
                // 先把第一个放入vector中
                list_info_t li = {term_info.posting_list, term_info.list_size, 0};
                list_info.push_back(li);
                // 记住总长度，便于后面分配内存
                PRINT("synonymslist.size: %u", synonymslist.size());
                uint32_t list_size_count = term_info.list_size;
                for (uint32_t s=0; s<synonymslist.size() && s < 3; s++)
                {
                    if (!synonymslist[s].isNull() && synonymslist[s].isString())
                    {
                        // 重复的代码，shit
                        int slist_num = myIndexGroup->get_posting_list(
                                synonymslist[s].asCString(),
                                tmpBuff,
                                tmpBuffSize);
                        PRINT("org-term[%s] synonyms-term[%s] weight[%u] slist_num[%d]",
                                term[QUERY_KEY_TERMLIST_TERM].asCString(), synonymslist[s].asCString(),
                                term_info.weight, slist_num);
                        li.list_size = slist_num <= 0? 0 : slist_num;
                        li.posting_list = NULL;
                        if ( li.list_size > 0)
                        {
                            char* dst_index_buff = (char*) malloc(li.list_size*doc_id_mask.uint32_count*sizeof(uint32_t));
                            if (dst_index_buff == NULL)
                            {
                                FATAL("malloc for index[%s] [%u] failed.",
                                        synonymslist[s].asCString(),
                                        li.list_size*doc_id_mask.uint32_count*sizeof(uint32_t)
                                     );
                                li.posting_list = NULL;
                                li.list_size = 0;
                            }
                            else
                            {
                                memmove(dst_index_buff, tmpBuff, li.list_size * doc_id_mask.uint32_count * sizeof(uint32_t));
                                li.posting_list = dst_index_buff;
                                list_info.push_back(li);
                                list_size_count += li.list_size;
                            }
                        }
                    }
                    synonyms_iter++;
                }
                if (1 < list_info.size())
                {
                    // 需要执行or_merge操作
                    uint32_t buffsize = (uint32_t) list_size_count * doc_id_mask.uint32_count * sizeof(uint32_t);
                    char* dst_index_buff = (char*) malloc(buffsize);
                    if (dst_index_buff == NULL)
                    {
                        FATAL("malloc for or-index[%s] size[%u] failed.", 
                                term[QUERY_KEY_TERMLIST_TERM].asCString(), buffsize);
                        // 第一个就别释放了
                        for (uint32_t k=1; k<list_info.size(); k++)
                        {
                            free(list_info[k].posting_list);
                        }
                    }
                    else
                    {
                        uint32_t or_merged_count = or_merge(list_info, doc_id_mask, dst_index_buff, buffsize);
                        for (uint32_t k=0; k<list_info.size(); k++)
                        {
                            free(list_info[k].posting_list);
                        }
                        term_info.posting_list   = dst_index_buff;
                        term_info.list_size      = or_merged_count;
                    }
                }
            }
            if (term_info.posting_list != NULL && term_info.list_size > 0)
            {
                term_vector.push_back(term_info);
            }
            PRINT("term[%s] weight[%u] list_num[%d] or_merged_count[%u]",
                    term[QUERY_KEY_TERMLIST_TERM].asCString(), term_info.weight, list_num, term_info.list_size);
            iter++;
        }
    }
    return 0;
}

// 读取过滤条件
int get_filt_list(Json::Value root, secore* mysecore, vector<filter_logic_t>& filt_vector)
{
    if (root[QUERY_KEY_FILTLIST].isArray() && root[QUERY_KEY_FILTLIST].size() > 0)
    {
        Json::Value filtlist = root[QUERY_KEY_FILTLIST];

        Json::Value::const_iterator iter;
        iter = filtlist.begin();
        for (uint32_t i=0; i<filtlist.size(); i++)
        {
            Json::Value filt = filtlist[i];
            if (filt[QUERY_KEY_FILTLIST_FIELD].isNull() || !filt[QUERY_KEY_FILTLIST_FIELD].isString()
                    || filt[QUERY_KEY_FILTLIST_METHOD].isNull() || filt[QUERY_KEY_FILTLIST_METHOD].isInt())
            {
                ALARM("filtlist[%u] error.", i);
                return -1;
            }
            filter_logic_t filter_logic;
            if (0 != mysecore->m_attr_maskmap->get_mask_item(filt[QUERY_KEY_FILTLIST_FIELD].asCString(), &filter_logic.key_mask))
            {
                ALARM("filt item[%s] NOT exist in attr-field-list.", filt[QUERY_KEY_FILTLIST_FIELD].asCString());
                return -1;
            }
            filter_logic.type = filt[QUERY_KEY_FILTLIST_METHOD].asInt();
            if ( EQUAL == filter_logic.type || BIGGER == filter_logic.type || SMALLER == filter_logic.type )
            {
                if (!filt[QUERY_KEY_FILTLIST_VALUE].isInt())
                {
                    ALARM("filt item[%s]'s value is NOT int.", filt[QUERY_KEY_FILTLIST_FIELD].asCString());
                    return -1;
                }
                filter_logic.value = filt[QUERY_KEY_FILTLIST_VALUE].asInt();
            }
            else if ( SET == filter_logic.type )
            {
                if (!filt[QUERY_KEY_FILTLIST_VALUE].isArray())
                {
                    ALARM("filt item[%s]'s value is NOT array.", filt[QUERY_KEY_FILTLIST_FIELD].asCString());
                    return -1;
                }
                Json::Value valuelist = filt[QUERY_KEY_FILTLIST_VALUE];
                for (uint32_t vi=0; vi<valuelist.size(); vi++)
                {
                    if (!valuelist[vi].isInt())
                    {
                        ALARM("filt item[%s]'s value is NOT int's array.", filt[QUERY_KEY_FILTLIST_FIELD].asCString());
                        return -1;
                    }
                    filter_logic.vset.insert(valuelist[vi].asInt());
                }
            }
            else if ( ZONE == filter_logic.type )
            {
                if (!filt[QUERY_KEY_FILTLIST_MAX].isInt() || !filt[QUERY_KEY_FILTLIST_MIN].isInt())
                {
                    ALARM("filt item[%s]'s value is NOT array.", filt[QUERY_KEY_FILTLIST_FIELD].asCString());
                    return -1;
                }
                filter_logic.min_value = filt[QUERY_KEY_FILTLIST_MIN].asInt();
                filter_logic.max_value = filt[QUERY_KEY_FILTLIST_MAX].asInt();
            }
            else
            {
                ALARM("filt item[%s]'s method is NOT exist.", filt[QUERY_KEY_FILTLIST_FIELD].asCString());
                return -1;
            }
            PRINT("field[%s] method[%u]",
                    filt[QUERY_KEY_FILTLIST_FIELD].asCString(),  filt[QUERY_KEY_FILTLIST_METHOD].asInt());
            filt_vector.push_back(filter_logic);
            iter++;
        }
    }
    return 0;
}

int ServiceApp(thread_data_t* ptd)
{
    struct timeval tv1, tv2;
    gettimeofday(&tv1, NULL );

    memset(ptd->SendHead, 0, sizeof(*ptd->SendHead));
    ptd->SendHead->log_id     = ptd->RecvHead->log_id;
    ptd->SendHead->detail_len = 0;
    snprintf(ptd->SendHead->srvname, sizeof(ptd->SendHead->srvname), "%s", PROJNAME);

    char* dststr = (char*) (ptd->SendHead+1);
    char* jsonstr = (char*) (ptd->RecvHead+1);

    query_param_t query_param;
    query_param.jsonstr = jsonstr;

    Json::Value root;
    Json::Reader reader;
    if (! reader.parse(jsonstr, root))
    {
        ALARM("jsonstr format error. [%s]", jsonstr);
        return 0;
    }

    query_param.all_num = 0;
    query_param.offset  = 0;
    query_param.size    = 2000;
    if (!root[QUERY_KEY_OFFSET].isNull() && root[QUERY_KEY_OFFSET].isInt())
    {
        // 如果非法就直接置成0
        int32_t offset = root[QUERY_KEY_OFFSET].asInt();
        query_param.offset = (offset >= 0 && offset <= 2000) ?  offset : query_param.offset;
    }
    if (!root[QUERY_KEY_SIZE].isNull() && root[QUERY_KEY_SIZE].isInt())
    {
        uint32_t size = root[QUERY_KEY_SIZE].asInt();
        query_param.size = size > 0 && size <= 2000 ? size : query_param.size;
    }
    if (!root[QUERY_KEY_ORDERBY].isNull() && root[QUERY_KEY_ORDERBY].isString())
    {
        query_param.orderby = root[QUERY_KEY_ORDERBY].asString();
    }

    if (0 == get_term_list(root, ptd->plugin->mysecore, (char*)ptd->SendHead, ptd->SendBuffSize, query_param.term_vector)
            && 0 == get_filt_list(root, ptd->plugin->mysecore, query_param.filt_vector))
    {
        flexse_plugin* pflexse_plugin = ptd->plugin;
        int ret = pflexse_plugin->query(&query_param, dststr, ptd->SendBuffSize - sizeof(xhead_t));
        ptd->SendHead->detail_len = (ret <= 0)? 0 : ret;
    }

    // 释放posting-list的临时内存
    for (uint32_t i=0; i<query_param.term_vector.size(); i++)
    {
        free(query_param.term_vector[i].posting_list);
        query_param.term_vector[i].posting_list = NULL;
    } 

    ptd->SendHead->all_num = query_param.all_num;
    gettimeofday(&tv2, NULL );
    uint32_t timecost = TIME_US_COST(tv1, tv2);
    ROUTN( "log_id[%u] name[%s] cltip[%s] "
            "offset[%u] size[%u] timecost_us[%u] "
            "query[%s] detail_len[%d]",
            ptd->RecvHead->log_id, ptd->RecvHead->srvname, ptd->cltip,
            query_param.offset, query_param.size, timecost,
            jsonstr, ptd->SendHead->detail_len);
    PRINT( "log_id[%u] name[%s] cltip[%s] "
            "offset[%u] size[%u] timecost_us[%u] "
            "query[%s] detail_len[%d]",
            ptd->RecvHead->log_id, ptd->RecvHead->srvname, ptd->cltip,
            query_param.offset, query_param.size, timecost,
            jsonstr, ptd->SendHead->detail_len);

    return 0;
}
