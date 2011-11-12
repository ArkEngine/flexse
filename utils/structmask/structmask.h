#ifndef  __STRUCTMASK_H_
#define  __STRUCTMASK_H_

#define _MAX_VALUE_(mask_item) \
                ((mask_item.item_mask) >> (mask_item.move_count))

#define _INC_UTILL_MAX_SOLO_(puint,mask_item,inc) \
                if (inc + (_GET_SOLO_VALUE_(puint,mask_item)) <= _MAX_VALUE_(mask_item))\
                {\
                    (((uint32_t*)puint)[mask_item.uint_offset] += ((inc)<<(mask_item.move_count)));\
                }\
                else\
                { \
                    _SET_SOLO_VALUE_(puint,mask_item,_MAX_VALUE_(mask_item));\
                }

#define _INC_UTILL_MAX_LIST_(puint,index,mask_item,inc) \
                if (inc + (_GET_LIST_VALUE_(puint,index,mask_item)) <= _MAX_VALUE_(mask_item))\
                {\
                    ((uint32_t*)puint)[(index)*mask_item.uint32_count+mask_item.uint_offset] += ((inc)<<(mask_item.move_count));\
                }\
                else\
                { \
                    _SET_LIST_VALUE_(puint,index,mask_item,_MAX_VALUE_(mask_item));\
                }


#define _GET_SOLO_VALUE_(puint,mask_item) \
                (((((uint32_t*)puint)[mask_item.uint_offset]) & (mask_item.item_mask)) >> (mask_item.move_count))

#define _GET_LIST_VALUE_(puint,index,mask_item) \
                (((((uint32_t*)puint)[(index)*mask_item.uint32_count+mask_item.uint_offset]) &\
                (mask_item.item_mask)) >> (mask_item.move_count))

#define _SET_SOLO_VALUE_(puint,mask_item,ivalue) \
                (((uint32_t*)puint)[mask_item.uint_offset] = \
                (((uint32_t*)puint)[mask_item.uint_offset] & \
                (~mask_item.item_mask)) | ((ivalue)<<(mask_item.move_count)))

#define _SET_LIST_VALUE_(puint,index,mask_item,ivalue) \
                ((uint32_t*)puint)[(index)*mask_item.uint32_count+mask_item.uint_offset] = \
                (((uint32_t*)puint)[(index)*mask_item.uint32_count+mask_item.uint_offset] &\
                (~mask_item.item_mask)) | ((ivalue) << mask_item.move_count)

#include <map>
#include <string>
#include <stdint.h>
#include <json/json.h>
using namespace std;

typedef struct _mask_item_t
{
    uint32_t item_mask;
    uint32_t uint_offset  : 10; // sizeof(struct) max is 1K*4, 一个结构体最多容纳1024个uint32_t
    uint32_t move_count   :  5; // 0 - 31
    uint32_t uint32_count : 10; // 
    uint32_t reserved     :  7; // 保留位
}mask_item_t;

class structmask
{
    private:
        map<string, mask_item_t> m_mask_map;
        map<string, mask_item_t>::iterator m_map_it;
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
        //structmask(const char* path, const char* name, const char* section);
        structmask(const Json::Value& field_array);
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
        // 迭代的接口
        void begin();
        void next();
        bool is_end();
        bool itget(char* keyname, const uint32_t bufsiz, mask_item_t* mask_item);
};














#endif  //__STRUCTMASK_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
