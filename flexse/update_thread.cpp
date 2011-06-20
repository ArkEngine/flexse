#include "update_thread.h"
#include "Config.h"
#include "mylog.h"
#include <unistd.h>

void* update_thread(void*)
{
    while(1)
    {
        sleep(1000);
        ROUTN("this is update thread");
    }
    return NULL;
}
