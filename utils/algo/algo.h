#include "structmask.h"
#include <set>

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

struct terminfo_t
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
 * @param logic_num       : number of logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t filter(
        void* pposting_list,
        const mask_item_t& doc_id_mask,
        const uint32_t nmemb,
        const void* pattrlist,
        const filter_logic_t* logic_list,
        const uint32_t logic_num
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
 * @param logic_num       : number of logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t ranking(
        void* pposting_list,
        const mask_item_t& doc_id_mask,
        const mask_item_t& weight_mask,
        const uint32_t nmemb,
        const void* pattrlist,
        const ranking_logic_t* logic_list,
        const uint32_t logic_num
        );

/**
 * @brief : merge the posting-list in 'weight' way.
 *
 * @param terminfo_list    : bench of posting-lists
 * @param terminfo_size    : number of posting-list
 * @param id_mask          : the 'id' mask
 * @param wt_mask          : the 'weight' mask
 * @param result_list      : result_pair stored here
 * @param result_list_size : the size of result_list
 *                                      
 * @return the result posting number stored in posting-list if OK, else -1.
 */

int32_t weight_merge(
        const terminfo_t* terminfo_list,
        const uint32_t terminfo_size,
        const mask_item_t id_mask,
        const mask_item_t wt_mask,
        result_pair_t* result_list,
        const uint32_t result_list_size
        );
