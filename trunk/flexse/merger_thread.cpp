#include "merger_thread.h"
#include "Config.h"
#include "mylog.h"
#include "index_group.h"
#include <unistd.h>

extern index_group* myIndexGroup;

void* merger_thread(void*)
{
    while(1)
    {
        ROUTN("this is merger thread");
        myIndexGroup->update_day_indexer();
        myIndexGroup->update_his_indexer();
    }
    return NULL;
}
