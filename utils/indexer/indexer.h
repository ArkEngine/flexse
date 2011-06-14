#include <stdint.h>

class indexer
{
    private:
        indexer(const indexer&);
    public:
        indexer();
        ~indexer();
        int32_t get_posting_list(const string& strterm, char* buff, const uint32_t buff_length) = 0;
        int32_t set_posting_list(const string& strterm, const char* buff) = 0;
};
