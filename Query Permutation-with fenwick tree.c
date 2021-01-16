#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

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

/* the following are two different ways to shuffle queries,
    one is used Fisher Yates shuffle, and 
    the other is just randomly choose elements in queries.
  */
int Fisher_Yates_shuffle(int **ind, int *m)
{
    srand(time(NULL));
    *m = (rand() % 10) + 1;  /* 1 <= m <= 10^3, choose a random m */
    int size = (rand() % (*m)) + 1; /* 1 <= size <= m, choose a random queries size */
    *ind = (int *)malloc(sizeof(int) * size);
    /* initialize queries to be [1,2,...,size] */
    for (int i = 0; i < size; i++) {
        *(*ind + i) = i+1;
    }
    printf("m is %d\n", *m);
    printf("query size is %d\n", size);
    srand(time(NULL));

    printf("queries: [");
    for (int i = 0; i < size - 1; i++) {
        int j = rand() % (size - i) + i; /* i <= j <= n-1 */
        int tmp = *(*ind + j);
        *(*ind + j) = *(*ind + i);
        *(*ind + i) = tmp;

        printf("%d, ", *(*ind + i));
    }
    printf("%d]\n", *(*ind + size - 1));

    return size;
}

int randomCreateArr(int **ind, int *m)
{
    srand(time(NULL));
    *m = (rand() % 1000) + 1;  /* 1 <= m <= 10^3, choose a random m */
    int size = (rand() % (*m)) + 1; /* 1 <= size <= m, choose a random queries size */
    *ind = (int *)malloc(sizeof(int) * size);

    printf("m is %d\n", *m);
    printf("query size is %d\n", size);

    srand(time(NULL));

    printf("queries: [");
    for (int i = 0; i < size; i++) {
        *(*ind + i) = rand()%(*m) + 1;
        if (i == size - 1) {
            printf("%d]\n", *(*ind + i));
            break;
        }
        printf("%d, ", *(*ind + i));
    }

    return size;
}

int main()
{
    int *queries;
    int m;
    int queriesSize = Fisher_Yates_shuffle(&queries, &m);
    int returnSize;
    int *result;
    result = processQueries(queries, queriesSize, m, &returnSize);

    printf("answer: [");
    for (int i = 0; i < queriesSize; i++) {
        if (i == queriesSize - 1) {
            printf("%d]\n", result[i]);
            break;
        }
        printf("%d,", result[i]);
    }

    free(queries);
    free(result);
    
    return 0;
}