#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "withBIT.h"
#include "withoutBIT.h"

int Fisher_Yates_shuffle(int **ind, int *m)
{
    srand(time(NULL));
    *m = (rand() % 100) + 1;  /* 1 <= m <= 10^3, choose a random m */
    int size = (rand() % (*m)) + 1; /* 1 <= size <= m, choose a random queries size */
    *ind = (int *)malloc(sizeof(int) * size);
    /* initialize queries to be [1,2,...,size] */
    for (int i = 0; i < size; i++) {
        *(*ind + i) = i+1;
    }

    srand(time(NULL)); 

    for (int i = 0; i < size - 1; i++) {
        int j = rand() % (size - i) + i; /* i <= j <= n-1 */
        int tmp = *(*ind + j);
        *(*ind + j) = *(*ind + i);
        *(*ind + i) = tmp;
    }

    return size;
}

int main()
{
    int count = 1000000;
    while (count-- > 0) {
        int *queries;
        int m;
        int queriesSize = Fisher_Yates_shuffle(&queries, &m); //testcase
        int returnSize1, returnSize2;
        int *result1;
        int *result2;
        result1 = processQueries(queries, queriesSize, m, &returnSize1);
        result2 = processQueries_without(queries, queriesSize, m, &returnSize2);

        if (returnSize1 != returnSize2)
            printf("Two results don't have the same size.\n");
        else
        {
            for (int i = 0; i < returnSize1; i++)
            {
                if (result1[i] != result2[i]) {
                    printf("Two results don't have the same answer.\n");
                    continue;
                } 
            }
            printf("Two results have the same answers.\n");
        }
    }
    return 0;
    
}  
