
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>


int cmp(int a, int b) {
    return a - b;
}

typedef int (*sort_cmp_pt)(int a, int b);

void swap(int *a, int *b) {
    int temp = 0;
    temp = *a;
    *a = *b;
    *b = temp;
}

void quick_sort(int *nums, const size_t len, sort_cmp_pt cmp) {
    int pivot = 0, min = 0, max = len - 1;
    int i = min, j = max, mid;

    if (len == 0) {
        return;
    }

    pivot = nums[min];

    for (;;) {
        if (i == j) {
            mid = i;
            break;
        }
        

        for (;;) {
            if (i == j) {
                mid = i;
                break;
            }

            if (cmp(nums[j], pivot) >= 0) {
                j--;
                continue;
            }

            //cmp(nums[j], pivot) < 0
            swap(&nums[j], &nums[i]);
            
            i++;
            //now j is pivot
            break;
        }
        
        for (;;) {
            if (i == j) {
                mid = i;
                break;
            }

            if (cmp(nums[i], pivot) <= 0) {
                i++;
                continue;
            }

             //now j is pivot
            swap(&nums[i], &nums[j]);
            
            //now i is pivot
            j--;
            break;
        }

    }

    if (min == max) {
        return;
    }

    quick_sort(nums, mid, cmp);
    quick_sort(nums + mid + 1, len - (mid + 1), cmp);

}

int	main(int argc, char **argv) {
    
    int arr[] = {30, 24, 5, 58, 18, 36, 12, 42, 39};


    quick_sort(arr, sizeof(arr)/sizeof(arr[0]), cmp);


    for (int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++) {
        printf("%d,", arr[i]);
    }

    printf("\n");
    return 0;
}