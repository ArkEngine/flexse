#ifndef _INDEXER_H_
#define _INDEXER_H_
#include <stdint.h>

struct ikey_t
{
    union{
        uint64_t sign64;
        struct{
            uint32_t uint1;
            uint32_t uint2;
        };
    };
};

class indexer
{
    private:
        indexer(const indexer&);
    public:
        indexer();
        virtual ~indexer();
        virtual int32_t get_posting_list(const char* strTerm, char* buff, const uint32_t length) = 0;
};
#endif
