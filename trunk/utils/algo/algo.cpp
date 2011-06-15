#include "algo.h"
#include <algorithm>
#include "Log.h"
#include <stdio.h>
#include <string.h>
#include <set>
#include <vector>
#include <cstdio>
#include <cstring>
using namespace std;

/**
 * @brief : filter posting-list by mutil-logic
 *
 * @param pposting_list   : posting-list stored here and filtered one also stored here.
 * @param post_uint_count : the uint_count of each post
 * @param doc_id_mask     : the doc_id mask_item of posting-list
 * @param nmemb           : the posting number of this posting-list.
 * @param pattrlist       : document-attribute buffer.
 * @param attr_uint_count : uint_count of each attribute
 * @param logic_list      : logic list
 * @param logic_num       : number of logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t filter(
		void* pposting_list,
        const uint32_t post_uint_count,
		const mask_item_t& doc_id_mask,
		const uint32_t nmemb, 
		const void* pattrlist,
        const uint32_t attr_uint_count,
        const filter_logic_t* logic_list,
        const uint32_t logic_num
		)
{

    if (       NULL == pposting_list || NULL == pattrlist
            || NULL == logic_list || 0 == logic_num || 0 == nmemb )
    {
		WARNING ("pposting_list[%p] pattrlist[%p] logic_list[%p] logic_num[%u] nmemb[%u], stop kidding me.\n",
                pposting_list, pattrlist, logic_list, logic_num, nmemb);
		return -1;
    }

    // store the result in orginal buffer.
	uint32_t* pdstuint = (uint32_t*)pposting_list;
	uint32_t* pinxuint = (uint32_t*)pposting_list;
	const uint32_t post_char_count = post_uint_count * sizeof(uint32_t);
	int32_t result_nmemb = 0;

	for (uint32_t i=0; i<nmemb; i++)
	{
        uint32_t doc_id = _GET_SOLO_VALUE_(pinxuint, doc_id_mask);
        bool skip = false;
        for (uint32_t k=0; k<logic_num; k++)
        {
            const filter_logic_t* plogic = &logic_list[k];
            uint32_t _value = _GET_LIST_VALUE_(pattrlist, doc_id, attr_uint_count, plogic->key_mask);
            switch(plogic->type)
            {
                case EQUAL:
//                    printf("EQUAL:  i[%u] : o[%u]\n", _value, plogic->value);
                    if (_value != plogic->value)
                    {
                        skip = true;
                    }
                    break;
                case SET:
                    if (plogic->vset.end() == plogic->vset.find(_value))
                    {
                        skip = true;
                    }
                    break;
                case BIGGER:
                    if (_value < plogic->value)
                    {
                        skip = true;
                    }
                    break;
                case SMALLER:
                    if (_value > plogic->value)
                    {
                        skip = true;
                    }
                    break;
                case ZONE:
                    if ((_value < plogic->min_value) || (_value > plogic->max_value))
                    {
                        skip = true;
                    }
                    break;
                default:
                    WARNING("type[%u] KIDDING ME!", plogic->type);
            }

        }
        if (! skip)
        {
            memmove(pdstuint, pinxuint, post_char_count);
            pdstuint += post_uint_count;
            result_nmemb ++;
        }
        pinxuint += post_uint_count;
    }
    return result_nmemb;
}

/**
 * @brief : ranking posting-list by mutil-logic
 *
 * @param pposting_list   : posting-list stored here and filtered one also stored here.
 * @param post_uint_count : the uint_count of each post
 * @param doc_id_mask     : the doc_id mask_item of posting-list
 * @param weight_mask     : the weight mask_item of posting-list
 * @param nmemb           : the posting number of this posting-list.
 * @param pattrlist       : document-attribute buffer.
 * @param attr_uint_count : uint_count of each attribute
 * @param logic_list      : logic list
 * @param logic_num       : number of logic list
 *
 * @return the result posting number stored in posting-list if OK, else -1.
 */
int32_t ranking(
		void* pposting_list,
        const uint32_t post_uint_count,
		const mask_item_t& doc_id_mask,
		const mask_item_t& weight_mask,
		const uint32_t nmemb, 
		const void* pattrlist,
        const uint32_t attr_uint_count,
        const ranking_logic_t* logic_list,
        const uint32_t logic_num
		)
{

    if (       NULL == pposting_list || NULL == pattrlist
            || NULL == logic_list || 0 == logic_num || 0 == nmemb )
    {
		WARNING ("pposting_list[%p] pattrlist[%p] logic_list[%p] logic_num[%u] nmemb[%u], stop kidding me.\n",
                pposting_list, pattrlist, logic_list, logic_num, nmemb);
		return -1;
    }

    // store the result in orginal buffer.
	uint32_t* pinxuint = (uint32_t*)pposting_list;

	for (uint32_t i=0; i<nmemb; i++)
	{
        uint32_t doc_id = _GET_SOLO_VALUE_(pinxuint, doc_id_mask);
        uint32_t weight = _GET_SOLO_VALUE_(pinxuint, weight_mask);
        for (uint32_t k=0; k<logic_num; k++)
        {
            const ranking_logic_t* plogic = &logic_list[k];
            uint32_t _value = _GET_LIST_VALUE_(pattrlist, doc_id, attr_uint_count, plogic->key_mask);
            switch(plogic->type)
            {
                case EQUAL:
                    if (_value == plogic->value)
                    {
                        weight += plogic->weight;
                    }
                    break;
                case SET:
                    if (plogic->vset.end() != plogic->vset.find(_value))
                    {
                        weight += plogic->weight;
                    }
                    break;
                case BIGGER:
                    if (_value >= plogic->value)
                    {
                        weight += plogic->weight;
                    }
                    break;
                case SMALLER:
                    if (_value <= plogic->value)
                    {
                        weight += plogic->weight;
                    }
                    break;
                case ZONE:
                    if ((plogic->min_value <= _value  ) && (_value <= plogic->max_value))
                    {
                        weight += plogic->weight;
                    }
                    break;
                default:
                    WARNING("type[%u] KIDDING ME!", plogic->type);
            }

        }
        _SET_SOLO_VALUE_(pinxuint, weight_mask, weight);
        pinxuint += post_uint_count;
    }
    return 0;
}

bool compare_result(const result_pair_t &left, const result_pair_t &right)
{
    return (left.weight > right.weight);
}

/**
 * @brief : merge the posting-list in 'and' way.
 *
 * @param terminfo_list    : bench of posting-lists
 * @param terminfo_size    : number of posting-list
 * @param id_mask          : the 'id' mask
 * @param wt_mask          : the 'weight' mask
 * @param post_uint_count  : the uint_count of each post
 * @param result_list      : result_pair stored here
 * @param result_list_size : the size of result_list
 *                                      
 * @return the result posting number stored in posting-list if OK, else -1.
 */

//int32_t and_merge(
//        const terminfo_t* terminfo_list,
//        const uint32_t terminfo_size,   
//        const mask_item_t id_mask,
//        const mask_item_t wt_mask,
//        const uint32_t post_uint_count,
//        result_pair_t* result_list, 
//        const uint32_t result_list_size
//        )
//{
//    uint32_t finish_term_num = 0;    ///< 有多少个term遍历完了
//    int current_pointer_list[terminfo_size];   ///< 每个term的拉链，当前指向的位置
//    uint32_t result_max_size = 0xFFFFFFFF;
//    for(uint32_t i=0;i<terminfo_size;i++)
//    {
//        result_max_size = (terminfo_list[i].list_size < result_max_size) ? terminfo_list[i].list_size : result_max_size;
//        if(terminfo_list[i].list_size == 0)
//        {
//            // AND 之后肯定是无结果
//            return 0;
//            finish_term_num ++;
//            current_pointer_list[i] = -1;
//        }
//        else
//        {
//            current_pointer_list[i] = 0;
//        }
//    }
//
//    // 因为是and操作，因此选择最短的即可
//    vector<result_pair_t> result_pair_list;
//    while(finish_term_num < terminfo_size)
//    {
//        ///< 先找到，当前每个拉链中指针指向位置中的最小的id
//        uint32_t min_id = 0xFFFFFFFF;
//        for(unsigned int i=0;i<terminfo_size;i++)
//        {
//            int32_t current_offset = current_pointer_list[i];
//            if(current_offset>=0)  ///< 这个拉链还没有遍历完毕
//            {
//                uint32_t id = _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, post_uint_count,
//                        id_mask.uint_offset, id_mask.item_mask, id_mask.move_count);
//                min_id = (min_id > id) ? id : min_id;
//            }
//        }
//
//        printf("min : %u\n", min_id);
//
//        ///< 把等于最小id的，算出它的weight，然后把对应的指针往下移动
//        uint32_t hit = 0;
//        uint32_t weight = 0;
//
//        for(uint32_t i=0; i<terminfo_size; i++)
//        {
//            int32_t& current_offset = current_pointer_list[i];
//            if(current_offset >= 0)  ///<  这个拉链还没有遍历完毕
//            {
//                uint32_t id = _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, post_uint_count,
//                        id_mask.uint_offset, id_mask.item_mask, id_mask.move_count);
//                if(min_id == id) ///< 对应最小id
//                {
//                    hit ++;
//                    weight += _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, post_uint_count,
//                            wt_mask.uint_offset, wt_mask.item_mask, wt_mask.move_count);
//                    current_offset ++;     ///< 指针向下移动一个
//                    if(current_offset == (int32_t)terminfo_list[i].list_size)  ///< 拉链遍历完了
//                    {
//                        printf("[%d] over.\n", i);
//                        current_offset = -1;
//                        finish_term_num ++;                    
//                    }
//                }
////                else
////                {
////                    current_offset ++;     ///< 指针向下移动一个
////                }
//            }
//        }
//        if (hit == terminfo_size) // 所有的数组里都出现了。
//        {
//            result_pair_t temp_result;
//            temp_result.id     = min_id;
//            temp_result.weight = weight;
//            result_pair_list.push_back(temp_result);
//            printf("--------in: %u %u\n", min_id, weight);
//        }
//    }
//
//
//    ///< 2. 对所有可能的结果尽心排序，得到最多MAX_RESULT_NUM个结果
//    int32_t result_num = 0;
//    if(result_pair_list.size() <= result_list_size)   ///< 整个vector完整排序一遍
//    {
//        result_num = result_pair_list.size();
//        sort(result_pair_list.begin(),result_pair_list.end(),compare_result);
//    }
//    else
//    {
//        result_num = result_list_size;
//        partial_sort(result_pair_list.begin(),
//                result_pair_list.begin()+result_num, result_pair_list.end(), compare_result);
//    }
//    ///< 把结果输出到数组中
//    for(unsigned int i=0;i<result_num;i++)
//    {
//        result_list[i] = result_pair_list[i];
//    }
//    return result_num;
//}

/**
 * @brief : merge the posting-list in 'weight' way.
 *
 * @param terminfo_list    : bench of posting-lists
 * @param terminfo_size    : number of posting-list
 * @param id_mask          : the 'id' mask
 * @param wt_mask          : the 'weight' mask
 * @param post_uint_count  : the uint_count of each post
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
        const uint32_t post_uint_count,
        result_pair_t* result_list, 
        const uint32_t result_list_size
        )
{
    uint32_t finish_term_num = 0;    ///< 有多少个term遍历完了
    int current_pointer_list[terminfo_size];   ///< 每个term的拉链，当前指向的位置
    uint32_t result_guess_size = 0;
    for(uint32_t i=0;i<terminfo_size;i++)
    {
        result_guess_size = (terminfo_list[i].list_size > result_guess_size) ? terminfo_list[i].list_size : result_guess_size;
        if(terminfo_list[i].list_size == 0)
        {
            finish_term_num ++;
            current_pointer_list[i] = -1;
        }
        else
        {
            current_pointer_list[i] = 0;
        }
    }

    vector<result_pair_t> result_pair_list;
    while(finish_term_num < terminfo_size)
    {
        ///< 先找到，当前每个拉链中指针指向位置中的最大的id
        uint32_t max_id = 0x0;
        for(unsigned int i=0;i<terminfo_size;i++)
        {
            int32_t current_offset = current_pointer_list[i];
            if(current_offset>=0)  ///< 这个拉链还没有遍历完毕
            {
                uint32_t id = _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, post_uint_count, id_mask);
                max_id = (max_id > id) ? max_id : id;
            }
        }

        ///< 把等于最大id的，算出它的weight，然后把对应的指针往下移动
        uint32_t weight = 0;

        for(uint32_t i=0; i<terminfo_size; i++)
        {
            int32_t& current_offset = current_pointer_list[i];
            if(current_offset >= 0)  ///<  这个拉链还没有遍历完毕
            {
                uint32_t id = _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, post_uint_count, id_mask);
                if(max_id == id) ///< 对应最小id
                {
                    uint32_t t_weight = _GET_LIST_VALUE_(terminfo_list[i].posting_list,
                            current_offset, post_uint_count, wt_mask);
                    // 如果非0，则相乘, 如果为0，则变成1
                    if (t_weight == 0)
                    {
                        t_weight = 1;
                    }
                    weight += terminfo_list[i].weight * t_weight;

                    current_offset ++;     ///< 指针向下移动一个
                    if(current_offset == (int32_t)terminfo_list[i].list_size)  ///< 拉链遍历完了
                    {
                        current_offset = -1;
                        finish_term_num ++;                    
                    }
                }
            }
        }
        result_pair_t temp_result;
        temp_result.id     = max_id;
        temp_result.weight = weight;
        result_pair_list.push_back(temp_result);

//        printf("--------in: %u %u\n", max_id, weight);
    }

    ///< 2. 对所有可能的结果尽心排序，得到最多MAX_RESULT_NUM个结果
    int32_t result_num = 0;
    if(result_pair_list.size() <= result_list_size)   ///< 整个vector完整排序一遍
    {
        result_num = result_pair_list.size();
        sort(result_pair_list.begin(),result_pair_list.end(),compare_result);
    }
    else
    {
        result_num = result_list_size;
        partial_sort(result_pair_list.begin(),
                result_pair_list.begin()+result_num, result_pair_list.end(), compare_result);
    }
    ///< 把结果输出到数组中
    for(int32_t i=0; i<result_num; i++)
    {
        result_list[i] = result_pair_list[i];
    }

    return result_num;
}
