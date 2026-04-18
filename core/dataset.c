#include "dataset.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

dataset_t dataset_generate(size_t n, size_t elem_size) {
    dataset_t ds = {0};

    ds.n = n;
    ds.elem_size = elem_size;
    ds.mode = INPUT_GENERATOR;

    ds.data = malloc(n * elem_size);

    // jen int generator
    for (size_t i = 0; i < n; i++) {
        ((int*)ds.data)[i] = rand();
    }

    return ds;
}

dataset_t dataset_mmap(const char *path, size_t elem_size) {
    dataset_t ds = {0};

    int fd = open(path, O_RDWR);
    size_t size = lseek(fd, 0, SEEK_END);

    void *data = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
    close(fd);

    ds.data = data;
    ds.mapping = data;
    ds.size = size;
    ds.elem_size = elem_size;
    ds.n = size / elem_size;
    ds.mode = INPUT_FILE;

    return ds;
}

void dataset_free(dataset_t *ds) {
    if (ds->mode == INPUT_GENERATOR) {
        free(ds->data);
    } else {
        munmap(ds->mapping, ds->size);
    }
}

