#ifndef _FLEXSE_PLUGIN_H_
#define _FLEXSE_PLUGIN_H_

#include "secore.h"
#include <json/json.h>

class flexse_plugin
{
    private:
        flexse_plugin();
        flexse_plugin(const flexse_plugin&);
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
