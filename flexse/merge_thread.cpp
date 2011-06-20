#include "merge_thread.h"
#include "Config.h"
#include "mylog.h"
#include <unistd.h>

void* merge_thread(void*)
{
    while(1)
    {
        sleep(1000);
        ROUTN("this is merge thread");
    }
    return NULL;
}
