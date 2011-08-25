#ifndef _NLP_PROCESSOR_H_
#define _NLP_PROCESSOR_H_
#include <string>
#include <vector>
#include "myutils.h"
using namespace std;
using namespace flexse;

class nlp_processor
{
    private:
        nlp_processor(const nlp_processor&);
    public:
        nlp_processor();
        ~nlp_processor();
        void split(char* str, vector<term_info_t>& termlist);
};
#endif
