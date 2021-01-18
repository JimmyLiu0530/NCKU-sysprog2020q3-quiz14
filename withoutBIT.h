#ifndef __WITHOUTBIT_H_
#define __WITHOUTBIT_H_


#include <stdlib.h>

int *processQueries_without(int* queries, int queriesSize, int m, int* returnSize){
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

#endif