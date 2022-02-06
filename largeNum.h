#ifndef LARGE_NUM_H_
#define LARGE_NUM_H_
/*
largeNum:
    range:-2^512~2^512 
        --2021/1/19--> -10^150 ~ 10^150 
        --2021/1/29-->10^(150*MAGNITUDE) ~ 10^(150*MAGNITUDE)
    note:2^512=(2^32)^64 ~ 10^153 ~ (10^9)^17 
        --2021/1/19--> -10^150 ~ 10^150 
        --2021/1/29-->10^(150*MAGNITUDE) ~ 10^(150*MAGNITUDE)
    Big-Endian
*/

/**
 * @brief you can **only** change MAGNITUDE to extend digits number
 * 
 */
#define MAGNITUDE (300)
#define LARGENUM_INTS (16 * MAGNITUDE)
#define LARGE_NUM_DIGITS_MAX (150 * MAGNITUDE)
#define LARGENUM_DATA_TYPE int

typedef struct largeNum
{
    char sign;
    // unsigned int data[LARGENUM_INTS];
    LARGENUM_DATA_TYPE data[LARGENUM_INTS];

    void (*assignStr)(struct largeNum *self, const char *n);
    void (*assign)(struct largeNum *self, long long n);

    void (*addNum)(struct largeNum *self, long long n);
    void (*subNum)(struct largeNum *self, long long n);
    void (*mulNum)(struct largeNum *self, long long n);
    /**
     * @brief
     *
     * @param self :the function caller, which will change to the result
     * @param n :the divisor
     * @param premainder :if not NULL, it will return the remainder's pointer
     */
    void (*divNum)(struct largeNum *self, long long n, long long *premainder);
    int (*biggerThan)(struct largeNum *self, long long n);

    void (*addLNum)(struct largeNum *self, struct largeNum n);
    void (*subLNum)(struct largeNum *self, struct largeNum n);
    void (*mulLNum)(struct largeNum *self, struct largeNum n);
    /**
     * @brief almost same to function:(divNum)
     */
    void (*divLNum)(struct largeNum *self, struct largeNum n, struct largeNum *remainder);
    int (*biggerLThan)(struct largeNum *self, struct largeNum n);

    int (*printLNum)(struct largeNum *self);
    void (*dstry)(struct largeNum **pself);
} largeNum;
/**
 * @brief :Using a long long int to initialize a largeNum
 *
 * @param n :long long int
 * @return largeNum* :
 */
largeNum *newLNum(long long n);
/**
 * @brief :Using a string standing for a number to initialize a largeNum
 *
 * @param n :string
 * @return largeNum* :
 */
largeNum *newLNumStr(const char *n);
/**
 * @brief :Using an already existed largeNum to initialize a new largeNum
 *
 * @param n :existed largeNum
 * @return largeNum* :
 */
largeNum *newLNumCopy(const largeNum *n);
/**
 * @brief :comparing if largeNum equals to long long
 *
 * @param self :largeNum
 * @param n :long long int
 * @return int : 0 -- not equal; 1 -- equal
 */
int equalTo(struct largeNum *self, long long n);
/**
 * @brief :comparing if largeNum equals to the other one
 *
 * @param self :largeNum
 * @param n :the other largeNum
 * @return int : 0 -- not equal; 1 -- equal
 */
int equalLTo(struct largeNum *self, struct largeNum *n);
/**
 * @brief :print a largeNum only
 *
 * @param self :the largeNum
 * @return int :the number of characters printed
 */
int printLNum(struct largeNum *self);
/**
 * @brief :extending %N to printf
 *
 * @param fmt :format
 * @param ...
 * @return int :number of characters printed
 */
int printLNf(const char *fmt, ...);

/**
 * @brief just for debug
 *
 */
void largeNum_modTest();

#endif
