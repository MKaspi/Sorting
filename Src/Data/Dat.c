#include "./Dat.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void dat_print(dat_arr* arr){
    for(int i=0;i<arr->length;i++){
        printf("%d ",arr->data[i]);
    }
}

void dat_swap(dat_arr* arr, int left, int right){
    if(left==right) return;
    ELEM_TYPE tmp=arr->data[left];
    arr->data[left]=arr->data[right];
    arr->data[right]=tmp;
}

void dat_load(dat_arr* arr){
    srand(time(NULL));
    arr->data=malloc(sizeof(ELEM_TYPE)*arr->length);
    for(int i=0;i<arr->length;i++){
        arr->data[i]=i;
    }
    for(int i=0;i<arr->length;i++){
        dat_swap(arr,i,rand() % arr->length);
    }
}

