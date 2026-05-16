#ifndef DATASET_H
#define DATASET_H

#include <stddef.h>

#include "dataset_enum.h"
#include "cli.h"

typedef struct {
    void *data;
    void *aux;

    size_t n;
    size_t elem_size;
    size_t size;

    dataset_type_t data_mode;
    dataset_type_t aux_mode;
} dataset_t;

dataset_t *dataset_create(config_t *cfg);
void dataset_fill(dataset_t *ds);
void dataset_free(dataset_t *ds);

#endif


