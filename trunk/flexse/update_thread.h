#ifndef _UPDATE_THREAD_H_
#define _UPDATE_THREAD_H_
#include "flexse_plugin.h"
#include "myutils.h"

int add(flexse_plugin* pflexse_plugin, const char* jsonstr);
int del(flexse_plugin* pflexse_plugin, const char* jsonstr);
int undel(flexse_plugin* pflexse_plugin, const char* jsonstr);

int _insert(flexse_plugin* pflexse_plugin, const uint32_t id, const vector<term_info_t>& termlist);

void* update_thread(void* args);

#endif
