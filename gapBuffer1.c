#include <stdlib.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct {
    char *buf;
    size_t size;
    size_t lb, rb;
} gapbuf_buffer_t;

typedef enum {
    GAPBUF_SUCCESS = 0,
    GAPBUF_ENOMEM, /* Out of memory    */
    GAPBUF_ERANGE, /* Range error      */
    GAPBUF_EARG,   /* Invalid argument */
    GAPBUF_EMAX    /* Max error number */
} gapbuf_error_t;

gapbuf_error_t gapbuf_insert(gapbuf_buffer_t *gb,
                             const char *buf,
                             const size_t n);

gapbuf_error_t gapbuf_delete(gapbuf_buffer_t *gap, size_t n);


/* Calculates the capacity of the gap buffer given a requested size. */
static size_t _gapbuf_calc_buf_capacity(const size_t n)
{
    size_t v = n - 1;
    v |= v >> 1;
    v |= v >> 2;
    v |= v >> 4;
    v |= v >> 8;
    v |= v >> 16;
#if (__WORDSIZE == 64)
    v |= v >> 32;
#endif
    v++;
    return v;
}

/**
 * Grows the buffer such that `n` more characters fit in the gap.
 *
 * The functions tests if adding `n` chars to the buffer would
 * overflow and then reallocates buffer capacity if necessary.
 *
 * @param gap Pointer to the gap buffer instance.
 * @param n   The additional number of chars to provision for.
 *
 * @return Nonzero on any failure. Consult `errno`.
 */
static int _gapbuf_provision_buf_capacity(gapbuf_buffer_t *gap, const size_t n)
{
    /* check if we need to extend the size of the buffer */
    if (gap->lb + n >= gap->rb) {
        /* allocate new buffer */
        size_t new_siz = _gapbuf_calc_buf_capacity(gap->size + n);
        char *new_buf;
        if (!(new_buf = malloc(new_siz)))
            return -1; /* ENOMEM */
        /* copy contents */
        size_t new_rb = gap->lb + new_siz - gap->size;
        size_t rlen = gap->size - (gap->rb + 1);
        if (gap->buf) {
            memcpy(new_buf, gap->buf, gap->lb);
            memcpy(new_buf + new_rb, gap->buf + gap->rb, rlen);
        }
        /* update gap buffer struct */
        char *tmp = gap->buf;
        gap->buf = new_buf;
        gap->rb = new_rb;
        gap->size = new_siz;
        free(tmp);
    }
    return 0;
}

gapbuf_error_t gapbuf_alloc(gapbuf_buffer_t **gb,
                            const char *content,
                            const size_t len)
{
    if (!(*gb = malloc(sizeof(gapbuf_buffer_t))))
        return GAPBUF_ENOMEM;
    (*gb)->size = 0;
    (*gb)->lb = 0, (*gb)->rb = 0;
    (*gb)->buf = NULL;
    if (content && len) return gapbuf_insert(*gb, content, len);
    return GAPBUF_SUCCESS;
}

gapbuf_error_t gapbuf_free(gapbuf_buffer_t *gb)
{
    if (gb) {
        free(gb->buf);
        free(gb);
    }
    return GAPBUF_SUCCESS;
}

size_t gapbuf_read(const gapbuf_buffer_t *gb, char *buf, const size_t bufsiz)
{
    if (!(gb && buf) || (buf && !bufsiz))
        return GAPBUF_EARG;
    /* copy lhs */
    size_t lsiz = gb->lb;
    size_t n = (bufsiz < lsiz) ? bufsiz : lsiz;
    memcpy(buf, gb->buf, n);
    /* copy rhs */
    size_t rsiz = (bufsiz > 0) ? gb->size - gb->rb - 1 : 0;
    n = (n < rsiz) ? n : rsiz;
    memcpy(buf + gb->lb, gb->buf + gb->rb, n);
    /* terminate string */
    size_t total_len = gb->lb + rsiz;
    size_t term_index = total_len < bufsiz ? total_len : bufsiz;
    buf[term_index] = '\0';
    return term_index;
}

gapbuf_error_t gapbuf_insert(gapbuf_buffer_t *gb,
                             const char *buf,
                             const size_t n)
{
    if (!(gb && buf))
        return GAPBUF_EARG;
    if (_gapbuf_provision_buf_capacity(gb, n) != 0)
        return GAPBUF_ENOMEM;
    memcpy(gb->buf + gb->lb, buf, n);
    gb->lb += n;
    return GAPBUF_SUCCESS;
}

gapbuf_error_t gapbuf_delete(gapbuf_buffer_t *gb, size_t n)
{
    if (n > gb->lb) /* cannot move beyond left boundary */
        return GAPBUF_ERANGE;
    gb->lb -= n;
    return GAPBUF_SUCCESS;
}

gapbuf_error_t gapbuf_fwd(gapbuf_buffer_t *gb, size_t n)
{
    if (n + gb->rb > gb->size)
        return GAPBUF_ERANGE;
    memmove(gb->buf + gb->lb, gb->buf + gb->rb, n);
    gb->lb += n;
    gb->rb += n;
    return GAPBUF_SUCCESS;
}

gapbuf_error_t gapbuf_rwd(gapbuf_buffer_t *gb, size_t n)
{
    if (n > gb->lb)
        return GAPBUF_ERANGE;
    memmove(gb->buf + gb->rb - n, gb->buf + gb->lb - n, n);
    gb->lb -= n;
    gb->rb -= n;
    return GAPBUF_SUCCESS;
}

#define my_assert(test, message) \
    do {                         \
        if (!(test))             \
            return message;      \
    } while (0)
#define my_run_test(test)       \
    do {                        \
        char *message = test(); \
        tests_run++;            \
        if (message)            \
            return message;     \
    } while (0)

int tests_run = 0;

static char *test_gapbuf_alloc_happy_path()
{
    printf("%s... ", __func__);
    char *expected = "0123456789";
    gapbuf_buffer_t *gap;
    my_assert(gapbuf_alloc(&gap, expected, strlen(expected)) == GAPBUF_SUCCESS,
              "Could not allocate buffer");
    my_assert(gap->size == 16, "Unexpected allocation size");
    char data[32];
    size_t n = gapbuf_read(gap, data, 32);
    data[n] = '\0';
    gapbuf_free(gap);
    my_assert(strncmp(expected, data, n) == 0,
              "Initialization string is not consistent");
    printf("OK\n");
    return 0;
}

static char *test_gapbuf_alloc_zero_length()
{
    printf("%s... ", __func__);
    gapbuf_buffer_t *gap;
    my_assert(gapbuf_alloc(&gap, "", 0) == GAPBUF_SUCCESS,
              "Could not allocate empty buffer");
    my_assert(gap, "Empty buffer allocation failed");
    my_assert(gap->size == 0, "Unexpected allocation size");
    my_assert(gapbuf_insert(gap, "asdf", 4) == GAPBUF_SUCCESS,
              "Could not insert into empty buffer");
    char data[8];
    size_t n = gapbuf_read(gap, data, 8);
    my_assert(strncmp("asdf", data, n) == 0, "String inconsistency");
    gapbuf_free(gap);
    printf("OK\n");
    return 0;
}

static char *test_gapbuf_read_into_insufficient_buffer()
{
    printf("%s... ", __func__);
    gapbuf_buffer_t *gap;
    my_assert(gapbuf_alloc(&gap, "0123456789", 10) == GAPBUF_SUCCESS,
              "Could not allocate buffer");
    char too_short[4];
    size_t n = gapbuf_read(gap, too_short, 4);
    my_assert(strncmp("0123456789", too_short, n) == 0,
              "String inconsistency for short buffer");
    printf("OK\n");
    return 0;
}

static char *test_suite()
{
    my_run_test(test_gapbuf_alloc_happy_path);
    my_run_test(test_gapbuf_alloc_zero_length);
    my_run_test(test_gapbuf_read_into_insufficient_buffer);
    return 0;
}

int main()
{
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    } else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}
