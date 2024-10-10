#include "../Src/Data/Dat.h"

void dat_sort(dat_arr* arr){
    for(int i=0;i<arr->length;i++){
        for(int j=i;j>0;j--){
            int left=j-1;
            int right=j;
            if(arr->data[left]>arr->data[right]){
                dat_swap(arr,left,right);
            }else{
                j=0; // zastaveni swap
            }
        } 
    }
}
