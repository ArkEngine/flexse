#include "his_merger_thread.h"
#include "config.h"
#include "mylog.h"
#include "index_group.h"
#include "flexse_plugin.h"
#include <unistd.h>


void* his_merger_thread(void* args)
{
    flexse_plugin* pflexse_plugin = (flexse_plugin*)args;
    index_group* myIndexGroup = pflexse_plugin->mysecore->m_pindex_group;
    while(1)
    {
        myIndexGroup->update_his_indexer();
        sleep(10);
    }
    return NULL;
}
