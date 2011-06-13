#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include "structmask.h"
#include "Log.h"
#include "MyException.h"
using namespace std;
using namespace flexse;

structmask::structmask(const char* path, const char* name, const char* section)
{
	char strPath[128];
	snprintf(strPath, sizeof(strPath), "%s/%s", path, name);
	ifstream fin(strPath);

	if ( strlen(section) != (uint32_t)snprintf(m_section_name, sizeof(m_section_name), "%s", section) )
	{
		FATAL ("section[%s] too looooong for m_section_name[%u]", section, (uint32_t)sizeof(m_section_name));
		MyToolThrow("section too looooong");
	}
	uint32_t bit_count  = 0;
	m_section_size = 0;
	char strline[128];
	//        for (uint32_t i=0; i<conf[section]["SEG_LIST"].size(); i++)
	while(fin.getline(strline, sizeof(strline)))
	{
		char     key[128];
		uint32_t seg;
		mask_item_t mask_item;

		if(2 != sscanf(strline, "%s %u", key, &seg) )
		{
			FATAL("SEG_LIST FORMAT ERROR. [%s]", strline);
			MyToolThrow("SEG_LIST FORMAT ERROR.");
		}
		if (seg == 0 || seg > 32 || ( bit_count + seg ) > 32)
		{
			FATAL("SEG_LIST CONFIG ERROR. key[%s] seg[%u]", key, seg);
			MyToolThrow("SEG_LIST CONFIG ERROR.");
		}
		// ¿¿¿¿
		mask_item.item_mask = 0;
		for (uint32_t mi=bit_count; mi<bit_count+seg; mi++)
		{
			mask_item.item_mask |= 1 << mi;
		}
		// ¿¿¿¿¿¿¿
		mask_item.move_count = bit_count;
		// ¿¿¿¿¿¿uint32_t¿
		mask_item.uint_offset = m_section_size;

		bit_count += seg;
		if (bit_count == 32)
		{
			bit_count = 0;
			m_section_size ++;
		}
		m_mask_map[string(key)] = mask_item;
	}
	if (bit_count != 0 || m_mask_map.size() == 0)
	{
		WARNING("map size = [%u]. section[%s] bit_count[%u] section_size[%u]",
				(uint32_t)m_mask_map.size(), section, bit_count, m_section_size);
		MyToolThrow("SEG_LIST CONFIG ERROR.");
	}
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
int structmask::get_section_name(char* name, const uint32_t size) const
{
	name[0] = 0;
	if (strlen(m_section_name) >= size)
	{
		return -1;
	}
	snprintf(name, size, "%s", m_section_name);
	return 0;
}
uint32_t structmask::get_section_size() const
{
	return m_section_size;
}
uint32_t structmask::get_segment_size() const
{
	return m_mask_map.size();
}



















/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
