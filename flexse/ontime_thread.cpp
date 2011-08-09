#include "ontime_thread.h"
#include "config.h"
#include "mylog.h"
#include <unistd.h>

void* ontime_thread(void*)
{
    while(1)
    {
        sleep(1000);
        ROUTN("this is ontime thread");
    }
    return NULL;
}
