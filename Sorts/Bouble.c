
#include "../Src/Data/Dat.h"

void dat_sort(dat_arr* arr){
    for(int i=0;i<arr->length;i++){
        for(int j=0;j<arr->length-i-1;j++){
            int left=j;
            int right=j+1;
            if(arr->data[left]>arr->data[right])dat_swap(arr,left,right);
        }
    }
}
