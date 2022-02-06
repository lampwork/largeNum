#ifndef MY_ASSERT_H_
#define MY_ASSERT_H_
// #include<assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

/*使用断言测试*/
#ifdef DEBUG
/*处理函数原型*/
void Assert(char *filename, unsigned int lineno);
#define ASSERT(condition) \
    if (condition)        \
        NULL;             \
    else                  \
        Assert(__FILE__, __LINE__)
/*不使用断言测试*/
#else
#define ASSERT(condition) NULL
#endif
void Assert(char *filename, unsigned int lineno);

#define DBG(str, ...)                                                                  \
    do                                                                                 \
    {                                                                                  \
        fprintf(stdout, "[*DBG*]:%s Line:%d " str, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

#define ERR(str, ...)                                                                  \
    do                                                                                 \
    {                                                                                  \
        fprintf(stderr, "[*ERR*]:%s Line:%d " str, __FILE__, __LINE__, ##__VA_ARGS__); \
    } while (0)

/*内部函数调试用*/
#ifdef DEBUG
#define __MARCO_TEST__ 127
#define TOSTRING(X) (__MARCO_TEST__##X)
#define TEST_MARCO_EMPTY(X) (TOSTRING(X) == 127)
#endif
#if 0
/*Test if MARCO is defined and empty*/
#if (defined(MARCO) && TEST_MARCO_EMPTY(MARCO))
#endif
#endif


#endif