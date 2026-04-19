#include "dataset.h"
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdio.h>

void dataset_fill(dataset_t *ds) {
    // jen int generator
    for (size_t i = 0; i < ds->n; i++) {
        ((int*)ds->data)[i] = rand();
    }
}

int prepare_file(dataset_t *ds, config_t cfg) {
    int fd = open(cfg.input_path, O_RDWR | O_CREAT, 0644);
    if (fd < 0) {
        perror("open");
        return -1;
    }

    size_t desired_size = cfg.elem_size * cfg.elem_count;

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return -1;
    }

    if ((size_t)st.st_size < desired_size) {
        if (ftruncate(fd, desired_size) == -1) {
            perror("ftruncate");
            close(fd);
            return -1;
        }
        ds->size = desired_size; // meni se velikost, takze pozadovana
    } else {
        ds->size = st.st_size; // velikost datasetu urcuje existujici soubor
    }

    return fd;
}

void dataset_mmap(dataset_t *ds, config_t cfg) {
    int fd = prepare_file(ds, cfg);
    if (fd < 0) {
        exit(1);
    }

    void *data = mmap(NULL, ds->size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        close(fd);
        exit(1);
    }

    close(fd);

    ds->data = data;
    ds->mapping = data;
}
void dataset_malloc(dataset_t *ds, config_t cfg) {
    ds->size = ds->elem_size * cfg.elem_count;

    void *data = malloc(ds->size);
    if(data == NULL){printf("fuckoff, malloc fail\n");exit(4);}

    ds->data = data;
    dataset_fill(ds); // pokud je internal, rovnou i plnim
}


dataset_t dataset_create(config_t cfg){
    dataset_t ds = {0};

    ds.elem_size = cfg.elem_size;
    ds.mode = cfg.dataset;
    // velikost alokovanyho prostoru si nastavim pro kazdej zvlast
    // pocet prvku si nastavim potom, protoze se nacte z velikosti souboru

    if (ds.mode == DATASET_INTERNAL) {
        dataset_malloc(&ds,cfg);
    } else if(ds.mode == DATASET_FILE) {
        dataset_mmap(&ds,cfg);
    } else {
        printf("fuck off, unknown dataset mode\n");
    }

    ds.n = ds.size / ds.elem_size;

    return ds;
}


void dataset_free(dataset_t *ds) {
    if (ds->mode == DATASET_INTERNAL) {
        free(ds->data);
    } else if(ds->mode == DATASET_FILE) {
        munmap(ds->mapping, ds->size);
    } else {
        printf("fuck it, uvolni OS\n");
    }
}

