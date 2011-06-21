#include "mylog.h"

int main()
{
    FATAL("this is message");
    ALARM("this is message");
    ROUTN("this is message");
    DEBUG("this is message");
    DEBUG("this is mydebug[%u]", 0);
    return 0;
}
