
#include <stdio.h>
#include <string.h>
#include <time.h>
extern clock_t t;
extern clock_t tTmp;
extern clock_t totalT;
extern clock_t totalTTmp;

#include "largeNum.h"

#define DEBUG
#include "myAssert.h"
#undef DEBUG

#define DEBUG_FOR_OUT
#include "strtoln.h"
#undef DEBUG_FOR_OUT

void modTest();

int main()
{
    t = 0;
    totalT = 0;
    totalTTmp = clock();
    // modTest();
    totalT += clock() - totalTTmp;
    // let's try a factorial(10000),it need the MAGNITUDE >= 300
    largeNum *LNum = newLNum(1);
    for (int i = 1; i <= 10000; i++)
    {
        LNum->mulNum(LNum, i);
    }
    printLNum(LNum);
    LNum->dstry(&LNum);
    printf("\ntest part time %fs\n", (double)t / CLOCKS_PER_SEC);
    printf("\ntotal time: %fs\n", (double)totalT / CLOCKS_PER_SEC);
    getchar();
    return 0;
}

void printff(const char *str)
{
    printf("%s\n", str);
    printf("len=%lld\n", strlen(str));
    return;
}

void modTest()
{
    largeNum_modTest();
}