/*log:
    2021/1/24 完成并测试整个strtoln.c
*/
#include <stdio.h>
#include <string.h>

#define DEBUG
#include "myAssert.h"
#undef DEBUG

#include "largeNum.h"
#include "strtoln.h"

#define DEBUG_FOR_OUT 1
#if DEBUG_FOR_OUT
#define STATIC
#else
#define STATIC static
#endif

#define IS_NUM_CHAR(X) ((X <= '9' && X >= '0') ? 1 : 0)
#define SIGN(X) ((X < 0) ? -1 : 1)
const int digitsOfOneData = 9;
#define __digitsOfOneData 9

#define REVERSE_ELEMENT(arr, index, total) (arr[(total)-1 - (index)])
#define REVERSE_STRING_ELEMENT(arr, index, total, interval) (arr[(total) - (interval) - (index * interval)])

STATIC int isNum(const char *str);

largeNum *strToLN(const char *n)
{
    largeNum *output = newLNum(0);
    int digits = isNum(n);
    output->sign = SIGN(digits);
    digits *= SIGN(digits);
    ASSERT(digits <= LARGE_NUM_DIGITS_MAX);

    //往上取整
    int numOfOneData = digits / digitsOfOneData + ((digits % digitsOfOneData) > 0);
    ASSERT(numOfOneData <= LARGENUM_INTS);

    char strNum[LARGE_NUM_DIGITS_MAX + 1] = {0};
    strncpy(strNum + (numOfOneData * digitsOfOneData - digits), (output->sign < 0) ? n + 1 : n, digits);
    memset(strNum, '0', (numOfOneData * digitsOfOneData - digits));

    long lTmp = 0;
    char data9Chr[__digitsOfOneData + 1] = {0};
    char *pChr = NULL;
    // while (numOfOneData-- > 0)
    for (int i = 0; i < numOfOneData; i++)
    {
        //这里会导致一个问题，当输入超过LARGE_NUM_DIGITS_MAX，截断到LARGE_NUM_DIGITS_MAX，而后按9为间隔前溯就会导致不能整除而出现负数
        //或者是输入的数字本身就少于9位，同样会造成偏移为负数
        //解决方案，在40行左右的strncpy前，就往前补0，直到有效数位为9的倍数
        pChr = &REVERSE_STRING_ELEMENT(strNum, i, numOfOneData * digitsOfOneData, digitsOfOneData);
        ASSERT(pChr >= (char *)strNum);
        strncpy(data9Chr, pChr, digitsOfOneData);
        lTmp = strtol(data9Chr, NULL, 10); // string to decimal
        REVERSE_ELEMENT(output->data, i, LARGENUM_INTS) = lTmp;
    }

    return output;
}

/**
 * @brief judge str can be converted to a number or not.
 *
 * @param str : a string to be converted to a number
 * @return int : -1 if str can't convert to a number, otherwise the lenth of the num str(exclude '\0');
 */
STATIC int isNum(const char *str)
{
    int ret = 0;
    int flag = 1;
    char strTmp[LARGE_NUM_DIGITS_MAX + 1] = {0};
    if (NULL == str)
    {
        return ret;
    }

    strncpy(strTmp, str, LARGE_NUM_DIGITS_MAX);
    const char *pchr = strTmp;
    if (*pchr == '-')
    {
        pchr++;
        flag = -1;
    }
    while (*pchr != '\0')
    {
        if (!IS_NUM_CHAR(*pchr))
        {
            // return -1;
            break;
        }
        ret++;
        pchr++;
    }

    return ret * flag;
}

void strToLN_modTest()
{
    printf("isNum:%d\n", isNum("-1234567890a"));
    largeNum *pLNum = strToLN("-1234567891011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                              "51525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899100");
    pLNum->printLNum(pLNum);

    pLNum->dstry(&pLNum);
    return;
}