#ifndef _TERMBLOCKCACHE_H_
#define _TERMBLOCKCACHE_H_
// 这个cache有点怪
// (1) 没有实现cache淘汰算法
// (2) 没有内存池
// (3) 总之不专业
#include <map>
using namespace std;
class termblockcache
{
    private:
        map<string, void*> m_block_map;
    public:
        termblockcache();
        ~termblockcache();
        set(const string& key, const void* value, const uint32_t valuesize);
        get(const string& key, (void*)& value, uint32_t& valuesize);
};
#endif
