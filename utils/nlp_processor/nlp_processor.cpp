#include "nlp_processor.h"
#include <set>
#include <string.h>
#include <stdio.h>

nlp_processor:: nlp_processor()
{}

nlp_processor:: ~nlp_processor()
{}

void nlp_processor:: split(char* str, vector<term_info_t>& termlist)
{
    char* stri = str;
    char* strb = str;
    set<string> strset;
    while(NULL != (strb=strchr(str, ' ')))
    {
        *strb = '\0';
        if (strset.end() == strset.find(string(str)))
        {
            term_info_t term_info;
            term_info.strTerm = string(str);
            termlist.push_back(term_info);
            strset.insert(string(str));
//            printf("uni key: %s\n", str);
        }
//        else
//        {
//            printf("dup key: %s - leftstring: %s --------\n", str, strb+1);
//        }
        str = strb + 1;
        *strb = ' ';
    }
    if (strset.end() == strset.find(string(stri)))
    {
        term_info_t term_info;
        term_info.strTerm = string(stri);
        termlist.push_back(term_info);
    }
}

