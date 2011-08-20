#include "day_merger_thread.h"
#include "config.h"
#include "mylog.h"
#include "index_group.h"
#include "flexse_plugin.h"
#include <unistd.h>


void* day_merger_thread(void* args)
{
    flexse_plugin* pflexse_plugin = (flexse_plugin*)args;
    index_group* myIndexGroup = pflexse_plugin->mysecore->m_pindex_group;
    // 对于更新特别缓慢的情况，是否需要考虑定时呢?
    while(1)
    {
        myIndexGroup->update_day_indexer();
    }
    return NULL;
}
