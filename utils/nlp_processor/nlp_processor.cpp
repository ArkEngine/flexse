#include "nlp_processor.h"
#include <set>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

nlp_processor:: nlp_processor()
{}

nlp_processor:: ~nlp_processor()
{}

void nlp_processor:: split(const char* str, vector<term_info_t>& termlist)
{
    uint32_t len = (uint32_t)strlen(str);
    char* mystr = (char*)malloc(len);
    snprintf(mystr, len, "%s", str);
    char* mptr = mystr;
    char* strb = NULL;
    char *strt = NULL;
    set<string> strset;


    while(NULL != (strb = strtok_r(mystr, " ", &strt)))
    {
        if (strset.end() != strset.find(string(strb)))
        {
            continue;
        }
        term_info_t term_info = {string(strb), 0, {0,0,0,0}};
        termlist.push_back(term_info);
        strset.insert(string(strb));
//        printf("uni key: %s\n", strb);
        mystr=NULL;
    }

    free(mptr);
}

