#include "nlp_processor.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        printf("./test \"this is a message\"\n");
        exit(1);
    }
    nlp_processor nlp;
    vector<term_info_t> termlist;
    nlp.split(argv[1], termlist);
    for (uint32_t i=0; i<termlist.size(); i++)
    {
        printf("[%s] ", termlist[i].strTerm.c_str());
    }
    printf("\n");
    return 0;
}
