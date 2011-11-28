#ifndef _FLEXSE_PLUGIN_H_
#define _FLEXSE_PLUGIN_H_

#include "secore.h"
#include "structmask.h"
#include "myutils.h"
#include <json/json.h>
#include <bson.h>

class flexse_plugin
{
    private:
        static const uint32_t HIT = 1;

        static const char* const CONFIGCATEGORY_FLEXINDEX;
        static const char* const FLEXINDEX_KEY_OP;
        static const char* const FLEXINDEX_KEY_TYPE;
        static const char* const FLEXINDEX_KEY_TOKEN;
        static const char* const FLEXINDEX_KEY_FIELD;
        static const char* const FLEXINDEX_KEY_WEIGHT;
        static const char* const FLEXINDEX_KEY_HIT_COUNT;
        static const char* const FLEXINDEX_KEY_OFFSET1;
        static const char* const FLEXINDEX_KEY_OFFSET2;
        static const char* const FLEXINDEX_KEY_OPTIONAL;
        static const char* const FLEXINDEX_KEY_POSITION;
        static const char* const FLEXINDEX_KEY_HIT_FIELD;
        static const char* const FLEXINDEX_KEY_HIT_WEIGHT;
        static const char* const FLEXINDEX_VALUE_OP_NLP;
        static const char* const FLEXINDEX_VALUE_OP_DOC_ID;
        static const char* const FLEXINDEX_VALUE_OP_PREFIX;
        static const char* const FLEXINDEX_VALUE_TYPE_INT;
        static const char* const FLEXINDEX_VALUE_TYPE_STR;
        static const char* const FLEXINDEX_VALUE_TYPE_LIST_INT;
        static const char* const FLEXINDEX_VALUE_TYPE_LIST_STR;
        static const char* const CONFIGCATEGORY_STRUCTMASK;
        static const char* const STRUCTMASK_POST;
        static const char* const STRUCTMASK_ATTR;

        flexse_plugin();
        flexse_plugin(const flexse_plugin&);

        enum {
            OP_DOC_ID = 0,
            OP_PREFIX,
            OP_NLP,
        };
        enum {
            T_INT = 0,
            T_STR,
            T_LIST_INT,
            T_LIST_STR,
        };
        struct key_op_t
        {
            uint32_t    op;                ///< 处置类型
            uint32_t    type;              ///< 数据类型
            uint32_t    optional;          ///< 是否可选
            uint32_t    position;          ///< 是否需要记录position
            char        hit_field[32];     ///< 是否需要记录field_hit
            uint32_t    hit_weight;        ///< field_hit的权重是多少
            char        token[32];         ///< 字段名称
            mask_item_t key_mask;          ///< 在postinglist中的structmask描述
        };
        struct term_desc_t
        {
            string      field_name;        ///< posting_list中的field_name
            uint32_t    value;             ///< field_hit的权重是多少
            mask_item_t key_mask;          ///< 在postinglist中的structmask描述
        };

        map <string, key_op_t> m_key_op_map;

        structmask* m_post_maskmap;
        structmask* m_attr_maskmap;

        // 这个json对象可以当作一个通用的容器
        Json::Value m_json_hell;

    public:
        secore* mysecore;

        flexse_plugin(const char* config_path, secore* insecore);
        ~flexse_plugin();
        int add   (const Json::Value& root, uint32_t& doc_id,
                map<string, term_info_t> & term_map, vector<attr_field_t> & attrlist);
        int mod   (const Json::Value& root, uint32_t& doc_id,
                map<string, term_info_t> & term_map, vector<attr_field_t> & attrlist);
        int del   (const Json::Value& root, vector<uint32_t> & id_list);
        int undel (const Json::Value& root, vector<uint32_t> & id_list);
        int query ( query_param_t* query_param, char* retBuff, const uint32_t retBuffSize);
        // merger/filter/ranking的算法实现，参考algo.h
};
#endif
