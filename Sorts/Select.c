#include "../Src/Data/Dat.h"

void dat_sort(dat_arr* arr){
    for(int i=0;i<arr->length;i++){
        int right=i;
        int left=i;
        for(int j=i+1;j<arr->length;j++){
            if(arr->data[right]>arr->data[j]) right=j;
        }
        dat_swap(arr,left,right);
    }
}
