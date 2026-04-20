#ifndef SORT_H
#define SORT_H

#include <stddef.h>
#define ELEM(ctx, i) ((char*)(ctx->data) + (i) * (ctx->elem_size))
#define ELEM_AUX(ctx, i) ((char*)(ctx->aux) + (i) * (ctx->elem_size))

typedef struct sort_ctx sort_ctx;

typedef int (*cmp_fn)(sort_ctx *ctx, size_t left, size_t right);

struct sort_ctx {
    void *data;
    void *aux;  // pomocne pole
    size_t n;
    size_t elem_size;
    cmp_fn cmp;
    void *swap_buf;
    int print_steps;
};

typedef enum {
    main_to_aux,
    aux_to_main
} copy_direction_t;

typedef void (*sort_fn)(sort_ctx *);
void swap(sort_ctx *ctx, size_t left, size_t right);
void copy(sort_ctx *ctx, size_t src, size_t dst, copy_direction_t dir);

#endif
