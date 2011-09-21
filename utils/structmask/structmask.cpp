#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <json/json.h>
#include "structmask.h"
#include "mylog.h"
#include "MyException.h"
using namespace std;
using namespace flexse;

//structmask::structmask(const char* path, const char* name, const char* section)
structmask::structmask(const Json::Value& field_array)
{
    MyThrowAssert(! field_array.isNull());
    Json::Value::const_iterator iter;
    iter = field_array.begin();

    uint32_t bit_count  = 0;
    m_section_size = 0;
    for (uint32_t i=0; i<field_array.size(); i++)
    {
        Json::Value::Members member=(*iter).getMemberNames();

        string strkey(*(member.begin()));
        uint32_t seg = ((*iter)[*(member.begin())]).asInt();
        iter++;

        mask_item_t mask_item;

        if (seg == 0 || seg > 32 || ( bit_count + seg ) > 32 || 0 == strkey.size() || strkey.size() > 100)
        {
            FATAL("SEG_LIST CONFIG ERROR. key[%s] seg[%u]", strkey.c_str(), seg);
            MyToolThrow("SEG_LIST CONFIG ERROR.");
        }
        // 计算掩码
        mask_item.item_mask = 0;
        for (uint32_t mi=bit_count; mi<bit_count+seg; mi++)
        {
            mask_item.item_mask |= 1 << mi;
        }
        // 计算移动的位数
        mask_item.move_count = bit_count;
        // 计算filed的位置
        mask_item.uint_offset = m_section_size;

        bit_count += seg;
        if (bit_count == 32)
        {
            bit_count = 0;
            m_section_size ++;
        }

        if (m_mask_map.end() == m_mask_map.find(strkey))
        {
            m_mask_map[strkey] = mask_item;
        }
        else
        {
            FATAL("REPEAT. key[%s] seg[%u]", strkey.c_str(), seg);
            MyToolThrow("SEG_LIST CONFIG ERROR.");
        }

    }

    if (bit_count != 0 || m_mask_map.size() == 0)
    {
        ALARM("map size = [%u]. bit_count[%u] section_size[%u]",
                (uint32_t)m_mask_map.size(), bit_count, m_section_size);
        MyToolThrow("SEG_LIST CONFIG ERROR.");
    }

    MyThrowAssert((2<<10) >= m_section_size);

    map<string, mask_item_t>::iterator it;
    for (it=m_mask_map.begin(); it!=m_mask_map.end(); it++)
    {
        it->second.uint32_count = m_section_size;
    }

    m_map_it = m_mask_map.begin();
}

structmask::~structmask()
{
}

int structmask::get_mask_item(const char* key, mask_item_t* mask_item)
{
    map<string, mask_item_t>::iterator it= m_mask_map.find(key);
    if (it != m_mask_map.end())
    {
        *mask_item = it->second;
        return 0;
    }
    else
    {
        return -1;
    }
}
uint32_t structmask::get_section_size() const
{
    return m_section_size;
}
uint32_t structmask::get_segment_size() const
{
    return m_mask_map.size();
}

void structmask :: begin()
{
    m_map_it = m_mask_map.begin();
}
void structmask :: next()
{
    m_map_it++;
}
bool structmask :: is_end()
{
    return m_map_it == m_mask_map.end();
}
bool structmask :: itget(char* keyname, const uint32_t bufsiz, mask_item_t* mask_item)
{
    memmove(mask_item, &m_map_it->second, sizeof(mask_item_t));
    return (int)strlen(m_map_it->first.c_str()) == snprintf(keyname, bufsiz, "%s", m_map_it->first.c_str());
}


















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
