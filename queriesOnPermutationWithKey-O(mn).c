#include <stdlib.h>
#include <stdio.h>

int* processQueries(int* queries, int queriesSize, int m, int* returnSize){
    int pos[m+1]; /* pos[i] is the position (indexing from 0) of number i*/
    for (int i = 0; i < m+1; i++) {  /* initialize pos[] */
        pos[i] = i-1;
    }
    
    int *result = (int *)malloc(sizeof(int) * (queriesSize + 1));
    for (int i = 0; i < queriesSize; i++) {  /* traverse elements in queries[] */
        int num = queries[i];
        int numPos = pos[num];
        result[i] = numPos;
        pos[num] = 0;
        for (int j = 1; j < m+1; j++) {
            if (pos[j] < numPos && j != num)
                pos[j] +=1;
        }
    }
    *returnSize = queriesSize;
    return result;
}

int main()
{
    int queries[] = {3,1,2,1};
    int m = 5;
    int queriesSize = sizeof(queries) / sizeof(queries[0]);
    int returnSize;
    int *ptr;
    ptr = processQueries(queries, queriesSize, m, &returnSize);

    printf("[");
    for (int i = 0; i < queriesSize; i++) {
        if (i == queriesSize - 1) {
            printf("%d]\n", ptr[i]);
            break;
        }
        printf("%d,", ptr[i]);
    }
    return 0;
}