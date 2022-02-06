#include"myAssert.h"

void Assert(char *filename, unsigned int lineno)
{
    fflush(stdout);
    fprintf(stderr,"\nAssert failed： %s, line %u\n",filename, lineno);
    fflush(stderr);

    system("pause");
    abort();
}