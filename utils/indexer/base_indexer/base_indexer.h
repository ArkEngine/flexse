#ifndef _BASE_INDEXER_H_
#define _BASE_INDEXER_H_
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

class base_indexer
{
    private:
        base_indexer(const base_indexer&);
    public:
        base_indexer();
        virtual ~base_indexer() = 0;
        virtual int32_t get_posting_list(const char* strTerm, void* buff, const uint32_t length) = 0;
        virtual void clear() = 0;
        virtual void set_readonly() = 0;
};
#endif
