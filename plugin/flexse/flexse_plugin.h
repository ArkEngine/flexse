#ifndef _FLEXSE_PLUGIN_H_
#define _FLEXSE_PLUGIN_H_

#include "secore.h"
#include "structmask.h"
#include <json/json.h>

class flexse_plugin
{
    private:
        static const char* const CONFIGCATEGORY_FLEXINDEX;
        static const char* const FLEXINDEX_KEY_OP;
        static const char* const FLEXINDEX_KEY_TYPE;
        static const char* const FLEXINDEX_KEY_TOKEN;
        static const char* const FLEXINDEX_KEY_FIELD;
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
            uint32_t    op;
            uint32_t    type;
            char        token[32];
            mask_item_t key_mask;
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
        int add   (const char* jsonstr, uint32_t& doc_id, vector<term_info_t> & termlist, vector<attr_field_t> & attrlist);
        int mod   (const char* jsonstr, uint32_t& doc_id, vector<term_info_t> & termlist, vector<attr_field_t> & attrlist);
        int del   (const char* jsonstr, vector<uint32_t> & id_list);
        int undel (const char* jsonstr, vector<uint32_t> & id_list);
        int query (const char* jsonstr, char* retBuff, const uint32_t retBuffSize);
        // merger/filter/ranking的算法实现，参考algo.h
};
#endif
