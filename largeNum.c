/*log:
    2021/1/19 完成并测试newLNum, printLNum
        一开始想用2^32~4*10^10进制,但是发现无论是转换还是计算都比较困难,所以退而求其次: 使用1*10^10进制
    2022/1/24 16:45 开始 以期完成carryLNum、addLNum、subLNum、equalTo、equalLTo、biggerThan
        19:40 完成
    2022/1/25 14:00 开始 以期完成mulLNum(涉及dataShift)，未完成。仅完成dataShift。

    2022/1/29 15:00 开始，以期完成mulLNum(预先: dataMult)
        19:30 完成.

    2022/2/6 18:00 开始，以期完成divLNum
        19:30 完成

    主要耗时部份：
    1.大内存空间的malloc
    2.dataMult
    3.addLNum
*/

#include <time.h>
clock_t t, tTmp;
clock_t totalT, totalTTmp;
// tTmp = clock();
// t += clock() - tTmp;
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "largeNum.h"
#if !defined(MAGNITUDE)
#error please define MAGNITUDE !!\n
#endif
#if !defined(LARGE_NUM_DIGITS_MAX)
#error please define LARGE_NUM_DIGITS_MAX !!\n
#endif
#if !defined(LARGENUM_INTS)
#error please define LARGENUM_INTS !!\n
#endif
#if !defined(LARGENUM_DATA_TYPE)
#error please define LARGENUM_DATA_TYPE
#endif

#include "strtoln.h"

#define DEBUG
#include "myAssert.h"
#undef DEBUG

#ifndef __UINT32_MAX__
#define __UINT32_MAX__ 0xffffffffU
#endif
#define LARGENUM_DATA_TYPE_MAX 999999999L
#define LARGENUM_PRINT_INTS (LARGENUM_INTS + 1)

/*take elements by reversed order*/
#define REVERSE_ELEMENT(arr, index, total) (arr[(total)-1 - (index)])
#define REVERSE_STRING_ELEMENT(arr, index, total, interval) (arr[(total) - (interval) - ((index) * (interval))])
#define SIGN(X) ((X) >= 0 ? 1 : -1)
/**
 * @param Dst : destination largeNum pointer
 * @param Src : source largeNum pointer
 */
#define LARGENUM_DATA_CPY(Dst, Src)             \
    do                                          \
    {                                           \
        for (int i = 0; i < LARGENUM_INTS; i++) \
        {                                       \
            (Dst)->data[i] = (Src)->data[i];    \
        }                                       \
    } while (0)

/*谨慎使用,因为数组退化成指针*/
#define ARRSIZE(X) (sizeof(X) / sizeof(typeof(X[0])))
/*0xffffffff ~ 10^9,need 10 chars to save a dataType to print*/
#define LARGENUM_DATA_PRINT_CHARS 9
static char printStr[LARGENUM_DATA_PRINT_CHARS * LARGENUM_PRINT_INTS + 1] = {'\0'};
static unsigned long long printLLs[LARGENUM_PRINT_INTS] = {0};
#define LARGENUM_DATA_PRINT_SCALE 1000000000L

/*set largeNum to 0*/
#define SET_LARGENUM_0(X) memset((X)->data, 0, sizeof(typeof((X)->data[0])) * LARGENUM_INTS)

/*************************functions for out indirectly*************************/
static void assignStr(struct largeNum *self, const char *n);
static void assign(struct largeNum *self, long long n);

static void addNum(struct largeNum *self, long long n);
static void subNum(struct largeNum *self, long long n);
static void mulNum(struct largeNum *self, long long n);
static void divNum(struct largeNum *self, long long n, long long *premainder);
static int biggerThan(struct largeNum *self, long long n);

static void addLNum(struct largeNum *self, struct largeNum n);
static void subLNum(struct largeNum *self, struct largeNum n);
static void mulLNum(struct largeNum *self, struct largeNum n);
static void divLNum(struct largeNum *self, struct largeNum n, struct largeNum *remainder);
static int biggerLThan(struct largeNum *self, struct largeNum n);

static void dstry(struct largeNum **pself);

/*******************************only for inside****************************/
static void _carryLNum(struct largeNum *self, int begin);
static void carryLNum(struct largeNum *self);
#define SHIFT_LEFT 0
#define SHIFT_RIGHT 1
static void _dataShift(struct largeNum *self, char dir);
static void dataShift(struct largeNum *self, char dir, int n);
static void dataMult(struct largeNum *self, LARGENUM_DATA_TYPE data);
static int sameDatasNumDiv(struct largeNum *n_dived, struct largeNum n_div);
static int dataPartCMP(struct largeNum *self, struct largeNum n);
static int validDatasNum(struct largeNum *self);

/********************************functions for out*************************/
int equalTo(struct largeNum *self, long long n);
int equalLTo(struct largeNum *self, struct largeNum *n);
int printLNum(struct largeNum *self);

/*****************************functions implementations********************/
largeNum *newLNum(long long n)
{
    largeNum *output = malloc(sizeof(largeNum));
    output->sign = SIGN(n);
    SET_LARGENUM_0(output);

    output->assign = assign;
    output->assignStr = assignStr;

    output->addNum = addNum;
    output->subNum = subNum;
    output->mulNum = mulNum;
    output->divNum = divNum;
    output->biggerThan = biggerThan;

    output->addLNum = addLNum;
    output->subLNum = subLNum;
    output->mulLNum = mulLNum;
    output->divLNum = divLNum;
    output->biggerLThan = biggerLThan;

    output->printLNum = printLNum;
    output->dstry = dstry;

    if (n == 0)
    {
        return output;
    }

    long long ntmp = n * SIGN(n);

    int i = 0;
    for (i = 0; ntmp / (LARGENUM_DATA_TYPE_MAX + 1) != 0; i++)
    {
        REVERSE_ELEMENT(output->data, i, LARGENUM_INTS) = ntmp % (LARGENUM_DATA_TYPE_MAX + 1);
        ntmp /= (LARGENUM_DATA_TYPE_MAX + 1);
    }
    REVERSE_ELEMENT(output->data, i, LARGENUM_INTS) = ntmp;

    return output;
}

largeNum *newLNumStr(const char *n)
{
    largeNum *output = strToLN(n);
    return output;
}

largeNum *newLNumCopy(const largeNum *n)
{
    largeNum *output = newLNum(0);
    output->sign = n->sign;
    // memcpy(output->data, n->data, sizeof(typeof(n->data[0])) * LARGENUM_INTS);
    for (int i = 0; i < LARGENUM_INTS; i++)
    {
        output->data[i] = n->data[i];
    }
    return output;
}

static void assignStr(struct largeNum *self, const char *n)
{
    largeNum *LNTmp = strToLN(n);
    self->sign = LNTmp->sign;
    for (int i = 0; i < LARGENUM_INTS; i++)
    {
        self->data[i] = LNTmp->data[i];
    }
    LNTmp->dstry(&LNTmp);
}
static void assign(struct largeNum *self, long long n)
{
    largeNum *LNTmp = newLNum(n);
    self->sign = LNTmp->sign;
    for (int i = 0; i < LARGENUM_INTS; i++)
    {
        self->data[i] = LNTmp->data[i];
    }
    LNTmp->dstry(&LNTmp);
}

static void addNum(struct largeNum *self, long long n)
{
    if (n != 0)
    {
        largeNum *LNTmp = newLNum(n);
        self->addLNum(self, *LNTmp);
        LNTmp->dstry(&LNTmp);
    }
    return;
}
static void subNum(struct largeNum *self, long long n)
{
    self->addNum(self, -n);
    return;
}
static void mulNum(struct largeNum *self, long long n)
{
    if (n != 1)
    {
        largeNum *LNTmp = newLNum(n);
        self->mulLNum(self, *LNTmp);
        LNTmp->dstry(&LNTmp);
    }
    return;
}
static void divNum(struct largeNum *self, long long n, long long *premainder)
{
    if (n != 0)
    {
        largeNum *LNTmp = newLNum(n);
        largeNum *LNremainder = NULL;
        if (premainder != NULL)
        {
            LNremainder = newLNum(0);
        }
        self->divLNum(self, *LNTmp, LNremainder);
        LNTmp->dstry(&LNTmp);
        if (premainder != NULL)
        {
            *premainder = REVERSE_ELEMENT(LNremainder->data, 1, LARGENUM_INTS) * (LARGENUM_DATA_TYPE_MAX + 1) +
                          REVERSE_ELEMENT(LNremainder->data, 0, LARGENUM_INTS);
        }
        LNremainder->dstry(&LNremainder);
    }
    return;
}
static int biggerThan(struct largeNum *self, long long n)
{
    largeNum *LNTmp = newLNum(n);
    int ret = self->biggerLThan(self, *LNTmp);
    return ret;
}
/**
 * @brief compare a largeNum and a long long nmber
 *
 * @param self the caller largeNum's pointer
 * @param n the comparing long long number
 * @return int 0 -- not equal; 1 -- equal
 */
int equalTo(struct largeNum *self, long long n)
{
    int ret = 1;
    largeNum *LNTmp = newLNum(n);
    ret = equalLTo(self, LNTmp);
    return ret;
}
static void addLNum(struct largeNum *self, struct largeNum n)
{
    int flag = 0;
    if (self->sign == n.sign)
    {
        for (int i = 0; i < LARGENUM_INTS; i++)
        {
            self->data[i] += n.data[i];
        }
    }
    else
    {
        // as for difficult sign symbol. involve comparing two largeNum
        flag = dataPartCMP(self, n);
        if (-1 == flag)
        {
            self->sign = 1;
            SET_LARGENUM_0(self);
        }
        else if (0 == flag)
        {
            self->sign *= -1;
            for (int i = 0; i < LARGENUM_INTS; i++)
            {
                self->data[i] = n.data[i] - self->data[i];
            }
        }
        else if (1 == flag)
        {
            for (int i = 0; i < LARGENUM_INTS; i++)
            {
                self->data[i] = self->data[i] - n.data[i];
            }
        }
    }
    carryLNum(self);
}
static void subLNum(struct largeNum *self, struct largeNum n)
{
    n.sign *= -1;
    self->addLNum(self, n);
    return;
}
/**
 * @brief largeNum乘法的先导，计算一个data乘以largeNum
 *
 * @param self
 * @param digit
 */
static void dataMult(struct largeNum *self, LARGENUM_DATA_TYPE data)
{
    ASSERT(data <= LARGENUM_DATA_TYPE_MAX && data >= -LARGENUM_DATA_TYPE_MAX);
    if (data == 0)
    {
        SET_LARGENUM_0(self);
        return;
    }
    if (data < 0)
    {
        self->sign *= -1;
        data *= -1;
    }
    long long llTmp = 0;
    largeNum *LNTmp = newLNum(0);
    LNTmp->sign = self->sign; //注意符号要复制，否则最后的进位部分相加会出问题
    // 999999999 * 999999999 % 10^9 = 999999998 000000001
    //可以发现，多出来的前9位哪怕再加上999999999也不会超过int32的最大值7fff ffff=2,147,483,647
    for (int i = 0; i < LARGENUM_INTS - 1; i++)
    { // todo 还可以继续优化
        //这里的(long long)强转不能省略,否则就是两个int相乘,会溢出
        llTmp = REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) * (long long)data;
        //进位部份暂时不能加到前一位，否则又会被乘一遍
        REVERSE_ELEMENT(LNTmp->data, i + 1, LARGENUM_INTS) += llTmp / (LARGENUM_DATA_TYPE_MAX + 1);
        REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) = llTmp % (LARGENUM_DATA_TYPE_MAX + 1);
        // _carryLNum(self, i);//由于已经将进位部分放到另一个数字中,所以这个语句并没有任何效果,反而会拖慢很多时间
    }
    //将乘法导致的进位部份补齐
    self->addLNum(self, *LNTmp);
    carryLNum(self);
    LNTmp->dstry(&LNTmp);
    return;
}
static void mulLNum(struct largeNum *self, struct largeNum n)
{
    // long long llTmp
    char selfOutputSign = self->sign * n.sign;
    int selfvalidDatasNum = validDatasNum(self);
    int nvalidDatasNum = validDatasNum(&n);
    int minvalidDatasNum = selfvalidDatasNum < nvalidDatasNum ? selfvalidDatasNum : nvalidDatasNum;
    largeNum *longer = NULL, *shorter = NULL;
    largeNum *longerCpy = NULL, *shorterCpy = NULL;
    /*
    2022/1/31：优化，由于longer和shorter都不会被改变（前者被复制，后者则不是乘积的结果存放点），所以不需要创建新的largeNum
    */
    if (minvalidDatasNum == selfvalidDatasNum)
    {
        // shorter = newLNumCopy(self);
        // longer = newLNumCopy(&n);
        shorter = self;
        longer = &n;
    }
    else
    {
        // longer = newLNumCopy(self);
        // shorter = newLNumCopy(&n);
        longer = self;
        shorter = &n;
    }
    // convert to positive!
    shorter->sign *= shorter->sign;
    longer->sign *= longer->sign;
    shorterCpy = shorter;

    largeNum *LNTmp = newLNum(0);
    longerCpy = newLNum(0);
    for (int i = 0; i < minvalidDatasNum; i++)
    {
        LARGENUM_DATA_CPY(longerCpy, longer);
        dataShift(longerCpy, SHIFT_LEFT, i);
        dataMult(longerCpy, REVERSE_ELEMENT(shorterCpy->data, i, LARGENUM_INTS));
        LNTmp->addLNum(LNTmp, *longerCpy);
    }
    self->sign = selfOutputSign;
    LARGENUM_DATA_CPY(self, LNTmp);
    longerCpy->dstry(&longerCpy);
    LNTmp->dstry(&LNTmp);
    return;
}
/**
 * @brief 两个拥有相同数位（一个数位指(LARGENUM_DATA_TYPE_MAX+1)进制下的一位数）,或者被除数多一位的除法
 *
 * @param n_dived 被除数, 并且在调用后会变成求余（n_dived % n_div）的结果
 * @param n_div 除数
 * @return int 返回商
 */
static int sameDatasNumDiv(struct largeNum *n_dived, struct largeNum n_div)
{
    int n_dived_valid_datas_num = validDatasNum(n_dived);
    int n_div_valid_datas_num = validDatasNum(&n_div);
    ASSERT(n_dived_valid_datas_num == n_div_valid_datas_num || n_dived_valid_datas_num == n_div_valid_datas_num + 1);

    char output_sign = n_dived->sign * n_div.sign;
    n_dived->sign *= n_dived->sign;
    n_div.sign *= n_div.sign;

    int ret = 0;
    long long llTmp = 0;
    if (n_dived_valid_datas_num == n_div_valid_datas_num)
    {
        ret = REVERSE_ELEMENT(n_dived->data, n_dived_valid_datas_num - 1, LARGENUM_INTS) /
              (REVERSE_ELEMENT(n_div.data, n_div_valid_datas_num - 1, LARGENUM_INTS) + 1);
    }
    else
    {
        llTmp = (long long)REVERSE_ELEMENT(n_dived->data, n_dived_valid_datas_num - 1, LARGENUM_INTS) * (LARGENUM_DATA_TYPE_MAX + 1) +
                REVERSE_ELEMENT(n_dived->data, n_dived_valid_datas_num - 2, LARGENUM_INTS);
        ret = llTmp / (REVERSE_ELEMENT(n_div.data, n_div_valid_datas_num - 1, LARGENUM_INTS) + 1);
    }

    largeNum *n_div_tmp = newLNumCopy(&n_div);
    n_div_tmp->mulNum(n_div_tmp, ret);
    n_dived->subLNum(n_dived, *n_div_tmp);
    while (1)
    {
        if (n_dived->biggerLThan(n_dived, n_div))
        {
            n_dived->subLNum(n_dived, n_div);
            ret++;
        }
        else
        {
            break;
        }
    }
    n_dived->sign = output_sign;
    n_div_tmp->dstry(&n_div_tmp);
    return ret;
}
static void divLNum(struct largeNum *self, struct largeNum n, struct largeNum *remainder)
{
    char selfOutputSign = self->sign * n.sign;
    self->sign *= self->sign;
    n.sign *= n.sign;
    if (n.biggerLThan(&n, *self))
    {
        SET_LARGENUM_0(self);
        self->sign = selfOutputSign;
        return;
    }
    else if (equalTo(&n, 0))
    {
        printf("[**LargeNum Error**]:divisor can not be zero!!!!\n");
        return;
    }

    int selfValidDatasNum = validDatasNum(self);
    // int nValidDatasNum = validDatasNum(&n);
    largeNum *output = newLNum(0);
    largeNum *LNTmp = newLNum(0);
    int LNTmpCarry = 0;
    for (int i = 0; i < selfValidDatasNum; i++)
    { // todo 可以优化
        REVERSE_ELEMENT(LNTmp->data, 0, LARGENUM_INTS) = self->data[LARGENUM_INTS - selfValidDatasNum + i];
        LNTmpCarry = sameDatasNumDiv(LNTmp, n);
        if (i != selfValidDatasNum - 1)
        {
            dataShift(LNTmp, SHIFT_LEFT, 1);
        }
        dataShift(output, SHIFT_LEFT, 1);
        output->addNum(output, LNTmpCarry);
    }

    LARGENUM_DATA_CPY(self, output);
    self->sign = selfOutputSign;
    output->dstry(&output);
    if (remainder != NULL)
    {
        LARGENUM_DATA_CPY(remainder, LNTmp);
    }
    LNTmp->dstry(&LNTmp);
    return;
}
static int biggerLThan(struct largeNum *self, struct largeNum n)
{
    int ret = 1;
    int flag = -1; // stand for only data part's larger or smaller
    if (self->sign > n.sign)
    { // if self is positive and n is negative
        return 1;
    }
    else if (self->sign < n.sign)
    { // if self is negative and n is positive
        return 0;
    }
    else
    { // the same sign symbol
        flag = dataPartCMP(self, n);
        if (flag == -1)
        { // means they are equal,not bigger than!
            return 0;
        }
        if (self->sign == -1)
        { // minus number: reverse the ret!
            ret = flag ^ 1;
        }
        else
        {
            ret = flag;
        }
    }
    return ret;
}
/**
 * @brief compare if two largeNums are equal
 *
 * @param self the caller largeNum's pointer
 * @param n the camparing largeNum's pointer
 * @return int 0 -- not equal; 1 -- equal
 */
int equalLTo(struct largeNum *self, struct largeNum *n)
{
    int ret = 1;
    for (int i = 0; i < LARGENUM_INTS; i++)
    {
        if (self->data[i] != n->data[i])
        {
            ret = 0;
            break;
        }
    }
    return ret;
}

/*mind the output is from malloc!*/
static char *_9charsGet(unsigned long long n)
{
    char *output = malloc(sizeof(char) * LARGENUM_DATA_PRINT_CHARS + 1);
    memset(output, 0, LARGENUM_DATA_PRINT_CHARS + 1);
    ASSERT(n >= 0);
    sprintf(output, "%0*lld", LARGENUM_DATA_PRINT_CHARS, n);
    return output;
}

int printLNum(struct largeNum *self)
{ //很慢，有较大的优化空间
    int total = 0;
    char *strTmp = NULL;
    unsigned int carryTmp = 0;

    REVERSE_ELEMENT(printLLs, 0, LARGENUM_PRINT_INTS) = REVERSE_ELEMENT(self->data, 0, LARGENUM_INTS);
    if (-1 == self->sign)
    {
        total += printf("-");
    }
    for (int i = 0; i < LARGENUM_INTS - 1; i++)
    {
        if (1 || self->data[i] != 0)
        { /* 10^9进制 转 10进制*/
            // printf("%llu<=%u", REVERSE_ELEMENT(printLLs, i + 1, LARGENUM_PRINT_INTS), REVERSE_ELEMENT(self->data, i + 1, LARGENUM_INTS));
            REVERSE_ELEMENT(printLLs, i + 1, LARGENUM_PRINT_INTS) = REVERSE_ELEMENT(self->data, i + 1, LARGENUM_INTS);
            strTmp = _9charsGet(REVERSE_ELEMENT(printLLs, i, LARGENUM_PRINT_INTS) % LARGENUM_DATA_PRINT_SCALE);
            carryTmp = REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) / LARGENUM_DATA_PRINT_SCALE;
            /*这里使用更高一级的long long,是因为如果同样使用uint,可能导致溢出*/
            REVERSE_ELEMENT(printLLs, i + 1, LARGENUM_PRINT_INTS) += carryTmp;
            strncpy(&REVERSE_STRING_ELEMENT(printStr, i, ARRSIZE(printStr), LARGENUM_DATA_PRINT_CHARS),
                    strTmp, LARGENUM_DATA_PRINT_CHARS);
            free(strTmp);
        }
    }
    if (printLLs[1] != 0)
    {
        strTmp = _9charsGet(printLLs[1]);
        strncpy(&REVERSE_STRING_ELEMENT(printStr, LARGENUM_INTS - 1, ARRSIZE(printStr), LARGENUM_DATA_PRINT_CHARS),
                strTmp, LARGENUM_DATA_PRINT_CHARS);
        // strncpy(printStr, strTmp, LARGENUM_PRINT_INTS);
        free(strTmp);
    }

    int strInd = 0;
    strTmp = (char *)printStr;
    while ((printStr[strInd] == '0' || printStr[strInd] == '\0') && strInd < ARRSIZE(printStr) - 1)
    {
        strInd++;
        strTmp++;
    };
    if (strInd == ARRSIZE(printStr) - 1)
    {
        printStr[strInd - 1] = '0';
    }
    total += printf("%s", strTmp);

    return total;
}
static void dstry(struct largeNum **pself)
{
    free(*pself);
    return;
}
/**
 * @brief carry forward for largeNum
 *
 * @param self the caller largeNum's pointer
 * @param begin 从倒数第几位开始往前进位
 */
static void _carryLNum(struct largeNum *self, int begin)
{
    int carryTmp = 0;
    for (int i = begin; i < LARGENUM_INTS - 1; i++)
    {
        if (REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) <= LARGENUM_DATA_TYPE_MAX &&
            REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) >= 0)
        { // if positive and less than LARGENUM_DATA_MAX, we just do nothing
            continue;
        }
        else if (REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) >= 0)
        {
            carryTmp = REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) / (LARGENUM_DATA_TYPE_MAX + 1);
        }
        else if (REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) < 0)
        {
            carryTmp = -REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) / (LARGENUM_DATA_TYPE_MAX + 1) +
                       ((-REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) % (LARGENUM_DATA_TYPE_MAX + 1)) > 0);
            carryTmp *= -1;
        }
        REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) -= carryTmp * (LARGENUM_DATA_TYPE_MAX + 1);
        REVERSE_ELEMENT(self->data, i + 1, LARGENUM_INTS) += carryTmp;
    }
    if (REVERSE_ELEMENT(self->data, LARGENUM_INTS - 1, LARGENUM_INTS) < 0)
    {
        self->sign *= -1;
    }
    //考虑最高位“溢出”（指超过LARGENUM_DATA_TYPE_MAX的问题
    ASSERT(REVERSE_ELEMENT(self->data, LARGENUM_INTS - 1, LARGENUM_INTS) <= LARGENUM_DATA_TYPE_MAX);
}

static void carryLNum(struct largeNum *self)
{
    _carryLNum(self, 0);
}

static void _dataShift(struct largeNum *self, char dir)
{
    int i, end;
    switch (dir)
    {
    case SHIFT_LEFT:
        i = 1;
        end = LARGENUM_INTS;
        break;
    case SHIFT_RIGHT:
        i = 0;
        end = LARGENUM_INTS - 1;
        break;
    default:
        return;
        break;
    }

    for (; i < end; i++)
    {
        if (SHIFT_LEFT == dir)
        {
            self->data[i - 1] = self->data[i];
        }
        else if (SHIFT_RIGHT == dir)
        {
            REVERSE_ELEMENT(self->data, i, LARGENUM_INTS) = REVERSE_ELEMENT(self->data, i + 1, LARGENUM_INTS);
        }
    }
    if (SHIFT_LEFT == dir)
    {
        self->data[LARGENUM_INTS - 1] = 0;
    }
    else if (SHIFT_RIGHT == dir)
    {
        self->data[0] = 0;
    }
    return;
}

static void dataShift(struct largeNum *self, char dir, int n)
{
    for (int i = 0; i < n; i++)
    {
        _dataShift(self, dir);
    }
    return;
}

/**
 * @brief compare only data part(ignoring the sign symbol)
 *
 * @param self
 * @param n
 * @return int : -1 --- equal; 0 -- smaller than n; 1 -- bigger than n
 */
static int dataPartCMP(struct largeNum *self, struct largeNum n)
{
    int flag = -1;
    for (int i = 0; i < LARGENUM_INTS; i++)
    {
        if (self->data[i] > n.data[i])
        {
            flag = 1;
            break;
        }
        else if (self->data[i] < n.data[i])
        {
            flag = 0;
            break;
        }
    }
    return flag;
}

static int validDatasNum(struct largeNum *self)
{
    ////为了追求效率，先二分查询是否占用过半的data，从而确定查询方式
    //上面有错,如果中间有一段正好是零,那么就出错了!!
    int ret;
    ret = LARGENUM_INTS;
    for (int i = 0; i < LARGENUM_INTS; i++)
    {
        if (self->data[i] == 0)
        {
            ret--;
        }
        else
        {
            break;
        }
    }
    return ret;
}

#include <string.h>
/**
 * @brief extend %N to printf
 *
 * @param fmt format
 * @param ...
 * @return int return number of characters printed
 */
int printLNf(const char *fmt, ...)
{
    largeNum LNTmp;
    char str[4096] = {0};
    strncpy(str, fmt, 1024);
    char *pchr = str, *pstr = str;
    int arg_num = 0, total = 0;
    va_list valist;
    va_start(valist, fmt);
    while (*pchr != '\0')
    {
        if (*pchr == '%')
        {
            if (*(pchr + 1) == 'N')
            {
                *pchr = '\0';
                *(pchr + 1) = '\0';
                total += vprintf(pstr, valist);
                for (int i = 0; i < arg_num; i++)
                { //并不关心除了largeNum 外的变长参数，所以只是遍历完它们
                    //这里的void *，只是占一个类型位置，我们并不关心
                    va_arg(valist, void *);
                }
                LNTmp = va_arg(valist, largeNum);
                total += printLNum(&LNTmp);
                pchr += 2;
                arg_num = 0;
                pstr = pchr;
                continue;
            }
            else if (*(pchr + 1) == '%')
            {
                pchr++;
            }
            else
            {
                arg_num++;
            }
        }
        pchr++;
    }
    total += printf("%s", pstr);
    va_end(valist);
    return total;
}

#include <unistd.h>
//测试模块放在source文件里面，就无需考虑所谓的static问题了，不想给外部调用的就全部加上static
void largeNum_modTest()
{
    largeNum *pLNum = NULL;
    largeNum *pLNum2 = NULL;

    int i = 1;
    while (i-- > 0)
    {
        /********test assign & largeNum comparison***********/
        pLNum = newLNum(12345678910111213L);
        pLNum2 = newLNum(0);
        pLNum2->assign(pLNum2, 12345678910111213L);
        printf("comparing: %d\n", equalTo(pLNum, 12345678910111213L));
        printf("comparing: %d\n", equalLTo(pLNum, pLNum2));
        pLNum->assign(pLNum, -1234567891011121314L);
        pLNum->printLNum(pLNum);
        printf("\n");

        printf("comparing: %d\n", dataPartCMP(pLNum, *pLNum2));

        /*********test REVERSE_ELEMENT & carryLNum *******/
        REVERSE_ELEMENT(pLNum->data, 0, LARGENUM_INTS) *= -1;
        carryLNum(pLNum);
        pLNum->printLNum(pLNum);
        printf("\n");

        /**************test largeNum addition******************/
        pLNum2->assign(pLNum2, 99999999999999999LL);
        printLNum(pLNum);
        printf(" + ");
        printLNum(pLNum2);
        printf("=");
        pLNum->addLNum(pLNum, *pLNum2);
        pLNum->printLNum(pLNum);
        printf("\n");

        /***************test largeNum assignStr***********/
        pLNum->assignStr(pLNum, "-1234567891011121314151617181920212223242526272829303132333435363738394041424344454647484950"
                                "51525354555657585960616263646566676869707172737475767778798081828384858687888990919293949596979899100");
        pLNum->printLNum(pLNum);
        printf("\n");

        /***************test largeNum dataShift**********/
        pLNum->assign(pLNum, -123456789L);
        dataShift(pLNum, SHIFT_LEFT, 1);
        pLNum2->assignStr(pLNum2, "-123456789000000000");
        ASSERT(1 == equalLTo(pLNum, pLNum2));

        /*******************test largeNum validDataNum*******************/
        pLNum->assign(pLNum, -12345678910111213);
        printf("pLNum valid digits number: %d\n", validDatasNum(pLNum));

        /*******************test largeNum multiplication*********************/
        pLNum->assignStr(pLNum, "1234567891011121314151617181920");
        pLNum->printLNum(pLNum);
        printf(" * -2 = ");
        dataMult(pLNum, -2);
        pLNum->printLNum(pLNum);
        printf("\n");

        pLNum->assignStr(pLNum, "123456790000000000012345679");
        pLNum2->assignStr(pLNum2, "9999999999");
        pLNum->printLNum(pLNum);
        printf(" * 9999999999 = ");
        pLNum->mulLNum(pLNum, *pLNum2);
        pLNum->printLNum(pLNum);
        printf("\n");

        /***********************test largeNum division********************/
        pLNum->assignStr(pLNum, "1234567891011121314151617181920");
        pLNum2->assignStr(pLNum2, "12345679");
        printLNum(pLNum);
        printf(" / ");
        printLNum(pLNum2);
        printf(" = ");
        pLNum->divLNum(pLNum, *pLNum2, pLNum2);
        printLNum(pLNum);
        printf(" ...... ");
        printLNum(pLNum2);
        printf("\n");

        /********************test extended printLNf*****************/
        pLNum->assignStr(pLNum, "1234567891011121314151617181920");
        pLNum2->assignStr(pLNum2, "12345679");
        printLNf("%N / %N = ", *pLNum, *pLNum2);
        pLNum->divLNum(pLNum, *pLNum2, pLNum2);
        printLNf("%N ...... %N\n", *pLNum, *pLNum2);

        pLNum->dstry(&pLNum);
        pLNum2->dstry(&pLNum2);
        usleep(100000);
    }
    return;
}