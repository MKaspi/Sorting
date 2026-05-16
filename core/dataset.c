#include "dataset.h"

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

static size_t dataset_size(config_t *cfg) {
    return cfg->elem_size * cfg->elem_count;
}

static void *dataset_malloc(size_t size) {
    void *data = malloc(size);

    if (data == NULL) {
        perror("malloc");
        exit(1);
    }

    return data;
}

static int dataset_open_file(char *path) {
    int fd = open(path, O_RDWR | O_CREAT, 0644);

    if (fd < 0) {
        perror("open");
        exit(1);
    }

    return fd;
}

static size_t dataset_prepare_file(int fd, size_t required_size, int enlarge) {
    struct stat st;

    if (fstat(fd, &st) == -1) {
        perror("fstat");
        exit(1);
    }

    if ((size_t)st.st_size < required_size) {
        if (!enlarge) {
            fprintf(stderr, "dataset file is smaller than requested size\n");
            exit(1);
        }

        if (ftruncate(fd, required_size) == -1) {
            perror("ftruncate");
            exit(1);
        }
    }

    return required_size;
}

static void *dataset_mmap(char *path, size_t required_size, int enlarge, size_t *mapped_size) {
    int fd = dataset_open_file(path);
    void *data;

    *mapped_size = dataset_prepare_file(fd, required_size, enlarge);

    data = mmap(NULL, *mapped_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    close(fd);

    return data;
}

static void *dataset_alloc(dataset_type_t type, char *path, size_t required_size, int enlarge, size_t *mapped_size) {
    if (type == DATASET_INTERNAL) {
        *mapped_size = required_size;
        return dataset_malloc(required_size);
    }

    if (type == DATASET_FILE) {
        return dataset_mmap(path, required_size, enlarge, mapped_size);
    }

    return NULL;
}

static void dataset_free_memory(dataset_type_t type, void *data, size_t size) {
    if (data == NULL) {
        return;
    }

    if (type == DATASET_INTERNAL) {
        free(data);
        return;
    }

    if (type == DATASET_FILE) {
        munmap(data, size);
        return;
    }
}

void dataset_fill(dataset_t *ds) {
    size_t i;

    for (i = 0; i < ds->n; i++) {
        int value = rand();
        void *target = (char *)ds->data + i * ds->elem_size;

        if (ds->elem_size >= sizeof(int)) {
            *(int *)target = value;
        } else {
            memcpy(target, &value, ds->elem_size);
        }
    }
}

dataset_t *dataset_create(config_t *cfg) {
    dataset_t *ds = dataset_malloc(sizeof(dataset_t));
    size_t required_size = dataset_size(cfg);
    size_t main_size = 0;
    size_t aux_size = 0;

    ds->data = NULL;
    ds->aux = NULL;

    ds->n = cfg->elem_count;
    ds->elem_size = cfg->elem_size;
    ds->data_mode = cfg->dataset;
    ds->aux_mode = cfg->aux_dataset;

    ds->data = dataset_alloc(cfg->dataset, cfg->input_path, required_size, cfg->enlarge_dataset, &main_size);

    ds->size = main_size;

    if (cfg->aux_dataset != DATASET_NONE) {
        ds->aux = dataset_alloc(cfg->aux_dataset, cfg->aux_input_path, ds->size, cfg->enlarge_dataset, &aux_size);
    }

    return ds;
}

void dataset_free(dataset_t *ds) {
    if (ds == NULL) {
        return;
    }

    dataset_free_memory(ds->data_mode, ds->data, ds->size);
    dataset_free_memory(ds->aux_mode, ds->aux, ds->size);

    free(ds);
}

