#include "nlp_processor.h"
#include <set>
#include <string.h>

nlp_processor:: nlp_processor()
{}

nlp_processor:: ~nlp_processor()
{}

void nlp_processor:: split(char* str, vector<string>& vstr)
{
    char* stri = str;
    char* strb = str;
    set<string> strset;
    while(NULL != (strb=strchr(str, ' ')))
    {
        *strb = '\0';
        if (strset.end() == strset.find(string(stri)))
        {
            vstr.push_back(string(stri));
            strset.insert(string(stri));
        }
        stri = strb + 1;
        *strb = ' ';
    }
    if (strset.end() == strset.find(string(stri)))
    {
        vstr.push_back(string(stri));
    }
}

