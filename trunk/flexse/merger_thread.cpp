#include "merger_thread.h"
#include "config.h"
#include "mylog.h"
#include "index_group.h"
#include "flexse_plugin.h"
#include <unistd.h>


void* merger_thread(void* args)
{
    flexse_plugin* pflexse_plugin = (flexse_plugin*)args;
    index_group* myIndexGroup = pflexse_plugin->mysecore->m_pindex_group;
    while(1)
    {
        ROUTN("this is merger thread");
        myIndexGroup->update_day_indexer();
        myIndexGroup->update_his_indexer();
    }
    return NULL;
}
