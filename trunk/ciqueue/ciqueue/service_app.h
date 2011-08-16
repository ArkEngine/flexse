#include "thread_data.h"

enum {
    RET_OK = 0,
    RET_EMPTY_MESSAGE,
    RET_IP_INVALID,
    RET_JSON_FORMAT_INVALID,
    RET_WRITE_MQUEUE_FAIL,
    RET_PROTOCOL_ERROR,
};

int ServiceApp(thread_data_t* ptd);
