#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define lowbit(x) (x & -x)

int inversionCnt(int *A, int arraySize)
{
    int ans = 0;
    int max = -1;

    for (int i = 0; i < arraySize; i++) {
        if (A[i] > max)
            max = A[i];
    }
    printf("max is %d\n", max);
    int record[max + 1];
    memset(record, 0, sizeof(record));

    for (int i = arraySize - 1; i >= 0; i--) {
        ans += record[A[i]];
        for (int j = A[i]; j <= max; j++)
            record[j]++;
    }
    return ans;
}

static inline void update(int *tree, int size, int index, int delta)
{
    for (; index < size; index += lowbit(index))
        tree[index] += delta;
}

static inline int sum(int *tree, int index)
{
    int ret = 0;
    for (; index; index -= lowbit(index))
        ret += tree[index];
    return ret;
}

int inversionCnt_BIT(int *A, int arraySize)
    {
        int ans = 0;
        int max = -1;
        
        /* 找出陣列 A 中最大的數字 */
        for (int i = 0; i < arraySize; i++) {
            if (A[i] > max)
                max = A[i];
        }
        int tree[max + 1]; // record[i] 紀錄當前在陣列 A 中，數字 i 的右邊有幾個數字比它小
        memset(tree, 0, sizeof(tree));
        
        for (int i = arraySize - 1; i >= 0; i--) {
            ans += sum(tree, A[i]);
            update(tree, max+1, A[i], 1);
        }
        return ans;
    }

int main()
{
    int A[5] = {2, 3, 8, 6, 1};
    int ans = inversionCnt_BIT(A, sizeof(A)/sizeof(A[0]));
    printf("%d\n", ans);

    return 0;
}
