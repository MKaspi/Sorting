#pragma once

#define ELEM_TYPE int

typedef struct {
    ELEM_TYPE *data;
    int length;
} dat_arr;

void dat_sort(dat_arr*);

void dat_print(dat_arr*);
void dat_swap(dat_arr* arr, int left, int right);
void dat_load(dat_arr* arr);


