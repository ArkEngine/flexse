#ifndef _FLEXSE_PLUGIN_H_
#define _FLEXSE_PLUGIN_H_

#include "secore.h"
#include "structmask.h"
#include <json/json.h>

class flexse_plugin
{
    private:
        static const char* const CONFIGCATEGORY_FLEXINDEX;
        static const char* const CONFIGCATEGORY_STRUCTMASK;

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

    public:
        secore* mysecore;

        flexse_plugin(const char* config_path, secore* insecore);
        ~flexse_plugin();
        int add(  const char* jsonstr, uint32_t& doc_id, vector<term_info_t> & termlist);
        int mod(  const char* jsonstr, uint32_t& doc_id, vector<term_info_t> & termlist);
        int del(  const char* jsonstr, vector<uint32_t> & id_list);
        int undel(const char* jsonstr, vector<uint32_t> & id_list);
};
#endif
