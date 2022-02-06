#ifndef STR_TO_LARGE_NUM_H_
#define STR_TO_LARGE_NUM_H_

struct largeNum;
/**
 * @brief : 1. create a largeNum from a specific string
 *          2. notice that the result is from malloc!! notice the memory management
 *          3. It is not suggested that using the function:(strToLN) to assign a largeNum already existed!
 *          4. we suggest that you use function:(assignStr) to assign a largeNum, or initialize a largeNum with function:(newLNumStr)
 * @param n : the specific string that can be converted to a number
 * @return largeNum* : the result
 */
largeNum *strToLN(const char *n);

#endif
