#ifndef _INDEX_GROUP_H_
#define _INDEX_GROUP_H_

class index_group
{
    private:
        enum{
            MEM = 0,
            DAY = 1,
            MON = 2
        };
    public:
        index_group();
        ~index_group();
};

#endif
