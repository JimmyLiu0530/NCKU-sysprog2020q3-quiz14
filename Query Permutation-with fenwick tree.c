#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define lowbit(x) (x & -x)

static inline int sum(int *tree, int index)
{
    int ret = 0;
    for (; index; index -= lowbit(index))
        ret += tree[index];
    return ret;
}
static inline void update(int *tree, int size, int index, int delta)
{
    for (; index < size; index += lowbit(index))
        tree[index] += delta;
}

int *processQueries(int *queries, int queries_size, int m, int *ret_size)
{
    int map[m + 1];
    for (int i = 1; i <= m; ++i)
        map[i] = i + queries_size;
    int tree_size = queries_size + m + 1;
    int tree[tree_size];
    memset(tree, 0, sizeof(tree));
    for (int i = m; i; i--)
        update(tree, tree_size, i + queries_size, 1);

    *ret_size = queries_size;
    int *ret = malloc(sizeof(int) * queries_size);
    for (int i = 0; i < queries_size; ++i) {
        ret[i] = sum(tree, map[queries[i]]) - 1;
        update(tree, tree_size, map[queries[i]], -1); /* set to zero */
        map[queries[i]] = queries_size - i;
        update(tree, tree_size, map[queries[i]], 1); /* move to front */
    }
    return ret;
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