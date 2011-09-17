#ifndef _UPDATE_THREAD_H_
#define _UPDATE_THREAD_H_
#include "flexse_plugin.h"
#include "myutils.h"

int add(const uint32_t file_no, const uint32_t block_id, flexse_plugin* pflexse_plugin, const char* jsonstr);
int del(flexse_plugin* pflexse_plugin, const char* jsonstr);
int undel(flexse_plugin* pflexse_plugin, const char* jsonstr);

void* update_thread(void* args);

#endif
