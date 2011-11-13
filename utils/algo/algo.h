#ifndef _ALGO_H_
#define _ALGO_H_
#include "structmask.h"
#include <set>
#include <vector>

enum TYPE{
    EQUAL   = 0,
    BIGGER,
    SMALLER,
    ZONE,
    SET,
    // NOT EQUAL?
    // NOT IN SET?
};

struct filter_logic_t
{
    mask_item_t   key_mask;       ///> for ALL
    uint32_t      type;           ///> filter enum, for ALL
    uint32_t      value;          ///> for EQUAL/BIGGER/SMALLER
    uint32_t      min_value;      ///> for ZONE
    uint32_t      max_value;      ///> for ZONE
    set<uint32_t> vset;           ///> for SET
};

enum OPERATION{
    ADD = 0,
    MUL,
    // NOT EQUAL?
    // NOT IN SET?
};

struct ranking_logic_t
{
    mask_item_t   key_mask;       ///> for ALL
    uint32_t      type;           ///> ranking enum, for ALL
    uint32_t      operation;      ///> operation enum, for ALL
    uint32_t      weight;         ///> for ALL
    uint32_t      value;          ///> for EQUAL/BIGGER/SMALLER
    uint32_t      min_value;      ///> for ZONE
    uint32_t      max_value;      ///> for ZONE
    set<uint32_t> vset;           ///> for SET
};

struct list_info_t
{
    void*    posting_list;
    uint32_t list_size;
    uint32_t weight;
};

struct result_pair_t
{
    uint32_t id;
    uint32_t weight;
};


/**
 * @brief : filter posting-list by mutil-logic
 *
 * @param pposting_list   : posting-list stored here and filtered one also stored here.
 * @param doc_id_mask     : the doc_id mask_item of posting-list
 * @param nmemb           : the posting number of this posting-list.
 * @param pattrlist       : document-attribute buffer.
 * @param logic_list      : logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t filter(
        void* pposting_list,
        const uint32_t nmemb,
        const mask_item_t& doc_id_mask,
        const void* pattrlist,
        const vector<filter_logic_t>& logic_list
        );

/**
 * @brief : filter posting-list by mutil-logic
 *
 * @param pair_list  : posting-list stored here and filtered one also stored here.
 * @param pattrlist  : document-attribute buffer.
 * @param logic_list : logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */

void filter(
        vector<result_pair_t>& pair_list,
        const void* pattrlist,
        const vector<filter_logic_t>& logic_list
        );

/**
 * @brief : ranking posting-list by mutil-logic
 *
 * @param pposting_list   : posting-list stored here and weight also stored here.
 * @param doc_id_mask     : the doc_id mask_item of posting-list
 * @param weight_mask     : the weight mask_item of posting-list
 * @param nmemb           : the posting number of this posting-list.
 * @param pattrlist       : document-attribute buffer.
 * @param logic_list      : logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t ranking(
        void* pposting_list,
        const uint32_t nmemb,
        const mask_item_t& doc_id_mask,
        const mask_item_t& weight_mask,
        const void* pattrlist,
        const vector<ranking_logic_t>& logic_list
        );


/**
 * @brief : ranking posting-list by mutil-logic
 *
 * @param pair_list   : posting-list stored here and filtered one also stored here.
 * @param pattrlist   : document-attribute buffer.
 * @param logic_list  : logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t ranking(
        vector<result_pair_t>& pair_list,
        const void* pattrlist,
        const vector<ranking_logic_t>& logic_list
        );

/**
 * @brief : merge the posting-list in 'weight' way.
 *
 * @param terminfo_list    : vector of posting-lists
 * @param id_mask          : the 'id' mask
 * @param wt_mask          : the 'weight' mask
 * @param result_list      : result_pair stored here
 * @param result_list_size : the size of result_list
 *                                      
 * @return void
 */

void weight_merge(
        const vector<list_info_t>& terminfo_list,
        const mask_item_t id_mask,
        const mask_item_t wt_mask,
        vector<result_pair_t>& result_pair_list,
        const uint32_t result_list_size
        );

/**
 * @brief : merge the posting-list in 'or' way.
 *
 * @param terminfo_list    : batch of posting-lists
 * @param id_mask          : the 'id' mask
 * @param result_list      : result_pair stored here
 * @param result_list_size : the size of result_list
 *                                      
 * @return void
 */

uint32_t or_merge(
        const vector<list_info_t>& terminfo_list,
        const mask_item_t id_mask,
        void* result_list,
        const uint32_t result_list_size
        );

/**
 * @brief : sort by field. return the top N sorted result.
 *
 * @param base         : list to sort
 * @param size         : number of list
 * @param key_mask     : the 'field' mask
 * @param partial_size : return the top partial_size sorted elements
 *                                      
 * @return void
 */

void field_partial_sort(
        void* base,
        const uint32_t size,
        const mask_item_t key_mask,
        const uint32_t partial_size
        );

/**
 * @brief : group count by field.
 *
 * @param base            : list to count
 * @param size            : number of list
 * @param key_mask        : the 'field' mask
 * @param group_count_map : the result map
 *                                      
 * @return void
 */

void group_count(
        const void* base,
        const uint32_t size,
        const mask_item_t key_mask,
        map<uint32_t, uint32_t>& group_count_map
        );


#endif
