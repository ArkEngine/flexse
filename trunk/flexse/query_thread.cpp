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
    // 用'id'的structmask得到单位index_t的大小，用于copy内存
    mask_item_t doc_id_mask;

    MySuicideAssert( 0 == mysecore->m_post_maskmap->get_mask_item("id", &doc_id_mask));
    if (!root[QUERY_KEY_TERMLIST].isArray() || root[QUERY_KEY_TERMLIST].size() <= 0)
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
            if (!term[QUERY_KEY_TERMLIST_TERM].isString() || !term[QUERY_KEY_TERMLIST_WEIGHT].isInt())
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
            if ( term_info.list_size == 0)
            {
                term_info.posting_list = NULL;
            }
            else
            {
                char* dst_index_buff = (char*) malloc(term_info.list_size*doc_id_mask.uint32_count*sizeof(uint32_t));
                if (dst_index_buff == NULL)
                {
                    FATAL("malloc failed.");
                    term_info.posting_list = NULL;
                    term_info.list_size = 0;
                }
                else
                {
                    memmove(dst_index_buff, tmpBuff, term_info.list_size * doc_id_mask.uint32_count * sizeof(uint32_t));
                    term_info.posting_list = dst_index_buff;
                }
            }
            term_vector.push_back(term_info);
            PRINT("term[%s] weight[%u] list_num[%d]",
                    term[QUERY_KEY_TERMLIST_TERM].asCString(), term_info.weight, list_num);
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
            if (!filt[QUERY_KEY_FILTLIST_FIELD].isString() || !filt[QUERY_KEY_FILTLIST_METHOD].isInt())
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

    query_param.offset =  0;
    query_param.size   = 2000;
    if (root["offset"].isInt())
    {
        query_param.offset = root["offset"].asInt() >= 0 && root["offset"].asInt() <= 2000 ? root["offset"].asInt() : query_param.offset;
    }
    if (root["size"].isInt())
    {
        query_param.size = root["size"].asInt() > 0 && root["size"].asInt() <= 2000 ? root["size"].asInt() : query_param.size;
    }
    if (root["orderby"].isString())
    {
        query_param.orderby = root["orderby"].asString();
    }

    if (0 != get_term_list(root, ptd->plugin->mysecore, dststr, ptd->SendBuffSize - sizeof(xhead_t), query_param.term_vector)
            || 0 != get_filt_list(root, ptd->plugin->mysecore, query_param.filt_vector))
    {
        // 对query的分析过程出现了错误
        return 0;
    }

    flexse_plugin* pflexse_plugin = ptd->plugin;
    int ret = pflexse_plugin->query(&query_param, dststr, ptd->SendBuffSize - sizeof(xhead_t));

    // 释放posting-list的临时内存
    for (uint32_t i=0; i<query_param.term_vector.size(); i++)
    {
        free(query_param.term_vector[i].posting_list);
        query_param.term_vector[i].posting_list = NULL;
    } 

    ptd->SendHead->all_num = query_param.all_num;
    ptd->SendHead->detail_len = ret <= 0? 0 : ret;
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
