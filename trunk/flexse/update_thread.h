#ifndef _UPDATE_THREAD_H_
#define _UPDATE_THREAD_H_
#include "flexse_plugin.h"

int add(flexse_plugin* pflexse_plugin, const char* jsonstr);
int mod(flexse_plugin* pflexse_plugin, const char* jsonstr);
int del(flexse_plugin* pflexse_plugin, const char* jsonstr);
int undel(flexse_plugin* pflexse_plugin, const char* jsonstr);

void* update_thread(void* args);

#endif
