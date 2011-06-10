#ifndef  __STRUCTMASK_H_
#define  __STRUCTMASK_H_

#define _GET_SOLO_VALUE_(puint,uint_offset,item_mask,move_count) \
                (((((uint32_t*)puint)[uint_offset]) & (item_mask)) >> (move_count))
#define _GET_LIST_VALUE_(puint,index,uint_count, uint_offset,item_mask,move_count) \
                (((((uint32_t*)puint)[index*uint_count+uint_offset]) & (item_mask)) >> (move_count))
#define _SET_SOLO_VALUE_(puint,uint_offset,item_mask,move_count,ivalue) \
                (((uint32_t*)puint)[uint_offset] = (((uint32_t*)puint)[uint_offset] & (~item_mask)) | ((ivalue)<<(move_count)))
#define _SET_LIST_VALUE_(puint,index,uint_count, uint_offset,item_mask,move_count,ivalue) \
                ((uint32_t*)puint)[index*uint_count+uint_offset] = \
                    (((uint32_t*)puint)[index*uint_count+uint_offset] & (~item_mask)) | (ivalue << move_count)

#include <map>
#include <string>
#include <stdint.h>
using namespace std;

typedef struct _mask_item_t
{
    uint32_t item_mask;
    uint32_t uint_offset : 10; // sizeof(struct) max is 1K
    uint32_t move_count  :  5; // 0 - 31
    uint32_t reserved    : 17; // 
}mask_item_t;

class structmask
{
    private:
        map<string, mask_item_t> m_mask_map;
        char     m_section_name[128];
        uint32_t m_section_size;

        structmask();
    public:
        /**
         * @brief : constructor of structmask
         *
         * @param path : dir-path of config-file
         * @param name : name of config-file
         * @param section : section name of this structmask
         */
        structmask(const char* path, const char* name, const char* section);
        ~structmask();
        
        /**
         * @brief : get the mask item by key
         *
         * @param key : the key
         * @param mask_item : the mask item stored in this buffer
         *
         * @return 0 if OK else -1
         */
        int get_mask_item(const char* key, mask_item_t* mask_item);
        /**
         * @brief : get the section name
         *
         * @param name : section name stored in this buffer
         * @param size : the length of this buffer
         *
         * @return 0 if OK else -1
         */
        int get_section_name(char* name, const uint32_t size) const;
        /**
         * @brief : get the uint32_t number of this section
         *
         * @return : uint32_t number of this section
         */
        uint32_t get_section_size() const;
        /**
         * @brief : get the item number of this section
         *
         * @return : item number of this section
         */
        uint32_t get_segment_size() const ;
};














#endif  //__STRUCTMASK_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
