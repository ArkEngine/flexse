#include "algo.h"
#include <algorithm>
#include "mylog.h"
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
		const uint32_t nmemb, 
		const mask_item_t& doc_id_mask,
		const void* pattrlist,
        const vector<filter_logic_t>& logic_list
		)
{

    const uint32_t logic_num = (uint32_t)logic_list.size();

    if ( NULL == pposting_list || NULL == pattrlist || 0 == nmemb )
    {
		ALARM ("pposting_list[%p] pattrlist[%p] nmemb[%u], stop kidding me.\n",
                pposting_list, pattrlist, nmemb);
		return -1;
    }

    // store the result in orginal buffer.
	uint32_t* pdstuint = (uint32_t*)pposting_list;
	uint32_t* pinxuint = (uint32_t*)pposting_list;
    const uint32_t post_uint_count = doc_id_mask.uint32_count;
	const uint32_t post_char_count = (uint32_t)(post_uint_count * sizeof(uint32_t));
	int32_t result_nmemb = 0;

	for (uint32_t i=0; i<nmemb; i++)
	{
        uint32_t doc_id = _GET_SOLO_VALUE_(pinxuint, doc_id_mask);
        bool skip = false;
        for (uint32_t k=0; k<logic_num; k++)
        {
            const filter_logic_t* plogic = &logic_list[k];
            uint32_t _value = _GET_LIST_VALUE_(pattrlist, doc_id, plogic->key_mask);
            switch(plogic->type)
            {
                case EQUAL:
//                    PRINT("EQUAL: id[%u] value[%u] _value[%u]", doc_id, plogic->value, _value);
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
                    ALARM("type[%u] KIDDING ME!", plogic->type);
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
		)
{

    const uint32_t logic_num = (uint32_t)logic_list.size();
    const uint32_t nmemb     = (uint32_t)pair_list.size();
	int32_t result_nmemb = 0;

	for (uint32_t i=0; i<nmemb; i++)
	{
        uint32_t doc_id = pair_list[i].id;
        bool skip = false;
        for (uint32_t k=0; k<logic_num; k++)
        {
            const filter_logic_t* plogic = &logic_list[k];
            uint32_t _value = _GET_LIST_VALUE_(pattrlist, doc_id, plogic->key_mask);
            switch(plogic->type)
            {
                case EQUAL:
//                    PRINT("EQUAL: id[%u] value[%u] _value[%u]", doc_id, plogic->value, _value);
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
                    ALARM("type[%u] KIDDING ME!", plogic->type);
            }

        }
        if (! skip)
        {
            pair_list[result_nmemb] = pair_list[i];
            result_nmemb ++;
        }
    }
    pair_list.resize(result_nmemb);
    return;
}

/**
 * @brief : ranking posting-list by mutil-logic
 *
 * @param pposting_list   : posting-list stored here and filtered one also stored here.
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
		)
{
    const uint32_t logic_num = (uint32_t)logic_list.size();

    if ( NULL == pposting_list || NULL == pattrlist || 0 == nmemb )
    {
		ALARM ("pposting_list[%p] pattrlist[%p] nmemb[%u], stop kidding me.\n",
                pposting_list, pattrlist, nmemb);
		return -1;
    }

    // store the result in orginal buffer.
	uint32_t* pinxuint = (uint32_t*)pposting_list;
    const uint32_t post_uint_count = doc_id_mask.uint32_count;

	for (uint32_t i=0; i<nmemb; i++)
	{
        uint32_t doc_id = _GET_SOLO_VALUE_(pinxuint, doc_id_mask);
        uint32_t weight = _GET_SOLO_VALUE_(pinxuint, weight_mask);
        for (uint32_t k=0; k<logic_num; k++)
        {
            const ranking_logic_t* plogic = &logic_list[k];
            uint32_t _value = _GET_LIST_VALUE_(pattrlist, doc_id, plogic->key_mask);
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
                    ALARM("type[%u] KIDDING ME!", plogic->type);
            }

        }
        _SET_SOLO_VALUE_(pinxuint, weight_mask, weight);
        pinxuint += post_uint_count;
    }
    return 0;
}

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
		)
{
    const uint32_t logic_num = (uint32_t)logic_list.size();
    const uint32_t nmemb     = (uint32_t)pair_list.size();

	for (uint32_t i=0; i<nmemb; i++)
	{
        uint32_t doc_id = pair_list[i].id;
        uint32_t weight = pair_list[i].weight;
        for (uint32_t k=0; k<logic_num; k++)
        {
            const ranking_logic_t* plogic = &logic_list[k];
            uint32_t _value = _GET_LIST_VALUE_(pattrlist, doc_id, plogic->key_mask);
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
                    ALARM("type[%u] KIDDING ME!", plogic->type);
            }

        }
        pair_list[i].weight = weight;
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
 * @param result_pair_list : result_pair stored here
 * @param result_list_size : keep the top result_list_size
 *                                      
 * @return the result posting number stored in posting-list if OK, else -1.
 */

void weight_merge(
        const vector<list_info_t>& terminfo_list,
        const mask_item_t id_mask,
        const mask_item_t wt_mask,
        vector<result_pair_t>& result_pair_list,
        const uint32_t result_list_size
        )
{
    result_pair_list.clear();
    const uint32_t list_size = (uint32_t)terminfo_list.size();
    uint32_t finish_term_num = 0;    ///< 有多少个term遍历完了
    int current_pointer_list[list_size];   ///< 每个term的拉链，当前指向的位置
    uint32_t result_guess_size = 0;
    for(uint32_t i=0;i<list_size;i++)
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

    while(finish_term_num < list_size)
    {
        ///< 先找到，当前每个拉链中指针指向位置中的最大的id
        uint32_t max_id = 0x0;
        for(unsigned int i=0;i<list_size;i++)
        {
            int32_t current_offset = current_pointer_list[i];
            if(current_offset>=0)  ///< 这个拉链还没有遍历完毕
            {
                uint32_t id = _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, id_mask);
                max_id = (max_id > id) ? max_id : id;
            }
        }

        ///< 把等于最大id的，算出它的weight，然后把对应的指针往下移动
        uint32_t weight = 0;

        for(uint32_t i=0; i<list_size; i++)
        {
            int32_t& current_offset = current_pointer_list[i];
            if(current_offset >= 0)  ///<  这个拉链还没有遍历完毕
            {
                uint32_t id = _GET_LIST_VALUE_(terminfo_list[i].posting_list, current_offset, id_mask);
                if(max_id == id) ///< 对应最小id
                {
                    uint32_t t_weight = _GET_LIST_VALUE_(terminfo_list[i].posting_list,
                            current_offset, wt_mask);
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
        result_num = (uint32_t)result_pair_list.size();
        sort(result_pair_list.begin(),result_pair_list.end(),compare_result);
    }
    else
    {
        result_num = result_list_size;
        partial_sort(result_pair_list.begin(),
                result_pair_list.begin()+result_num, result_pair_list.end(), compare_result);
        result_pair_list.resize(result_list_size);
    }
}

void heap_adjust(uint32_t* list, const int32_t nLength, int32_t k, const mask_item_t key_mask)
{
    int32_t nChild;
    int32_t i=k;
    char tmpbuff[key_mask.uint32_count*sizeof(uint32_t)];
    memmove(tmpbuff, list + i*key_mask.uint32_count, key_mask.uint32_count*sizeof(uint32_t));
    uint32_t nTemp;

//    printf("[%u]-[%u] 2*i+1:[%u] vs l:%u\n", k, _GET_LIST_VALUE_(list, i, key_mask), 2*i+1, nLength);
    for (nTemp = _GET_LIST_VALUE_(list, i, key_mask); 2*i+1 < nLength; i = nChild)
    {
//        printf("[%u]+[%u]\n", k, _GET_LIST_VALUE_(list, i, key_mask));
        nChild = 2*i+1;

        if ((nChild != nLength - 1)
                && (_GET_LIST_VALUE_(list, (nChild + 1), key_mask) > _GET_LIST_VALUE_(list, nChild, key_mask)))
        {
            ++nChild;
        }

        if (nTemp < _GET_LIST_VALUE_(list, nChild, key_mask))
        {
            memmove(list + i*key_mask.uint32_count, list + nChild*key_mask.uint32_count, key_mask.uint32_count*sizeof(uint32_t));
//            printf("[%u] [%u]\n", k, _GET_LIST_VALUE_(list, i, key_mask));
        }
        else
        {
            break;
        }
    }

    memmove(list + i*key_mask.uint32_count, tmpbuff, key_mask.uint32_count*sizeof(uint32_t));
}

void field_partial_sort(void* base, const uint32_t size, const mask_item_t key_mask, const uint32_t partial_size)
{
    uint32_t* list = (uint32_t*) base;
    for (int32_t i = size / 2 - 1; i >= 0; --i)
    {
        heap_adjust(list, size, i, key_mask);
    }

//    for (uint32_t i=0; i<size; i++)
//    {
//        printf ("-- %u --\n", _GET_LIST_VALUE_(list, i, key_mask));
//    }

    char tmpbuff[key_mask.uint32_count*sizeof(uint32_t)];
    int32_t stop = size - 1 - partial_size;
    stop = stop < 0 ? 0 : stop;
    for (int32_t i = size - 1; i > stop; --i)
    {
        memmove(tmpbuff, list, key_mask.uint32_count*sizeof(uint32_t));
        memmove(list, list + i*key_mask.uint32_count, key_mask.uint32_count*sizeof(uint32_t));
        memmove(list + i*key_mask.uint32_count, tmpbuff, key_mask.uint32_count*sizeof(uint32_t));

        heap_adjust(list, i, 0, key_mask);
    }

    uint32_t middle = size / 2;
    if (partial_size > middle)
    {
        for (uint32_t i=0; i<middle; i++)
        {
            memmove(tmpbuff, list + (i*key_mask.uint32_count), key_mask.uint32_count*sizeof(uint32_t));
            memmove(list + (i*key_mask.uint32_count), list + (size-1-i)*key_mask.uint32_count, key_mask.uint32_count*sizeof(uint32_t));
            memmove(list + (size-1-i)*key_mask.uint32_count, tmpbuff, key_mask.uint32_count*sizeof(uint32_t));
        }
    }
    else
    {
        for (uint32_t i=0; i<partial_size; i++)
        {
            memmove(list + (i*key_mask.uint32_count), list + (size-1-i)*key_mask.uint32_count, key_mask.uint32_count*sizeof(uint32_t));
        }
    }
}

void group_count(
        const void* base,
        const uint32_t nmemb,
        const mask_item_t key_mask,
        map<uint32_t, uint32_t>& group_count_map
        )
{
    group_count_map.clear();
    const uint32_t* puint = (uint32_t*)base;
	for (uint32_t i=0; i<nmemb; i++)
    {
        uint32_t group_id = _GET_SOLO_VALUE_(puint, key_mask);
        if (group_count_map.find(group_id) == group_count_map.end())
        {
            group_count_map[group_id] = 1;
        }
        else
        {
            group_count_map[group_id] += 1;
        }
        puint += key_mask.uint32_count;
    }
}

