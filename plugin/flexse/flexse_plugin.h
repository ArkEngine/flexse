#ifndef _FLEXSE_PLUGIN_H_
#define _FLEXSE_PLUGIN_H_

#include "secore.h"
#include "structmask.h"
#include <json/json.h>

class flexse_plugin
{
    private:
        flexse_plugin();
        flexse_plugin(const flexse_plugin&);

        enum {
            DOC_ID = 0,
            PREFIX,
            NLP,
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
