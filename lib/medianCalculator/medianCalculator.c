#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "medianCalculator.h"

//compare function for qsort to sort uint32_t array in ascending order based on value
int compare_uint32(const void *a, const void *b) {
    //base index
    uint32_t arg1 = *(const uint32_t *)a;
    //compare index
    uint32_t arg2 = *(const uint32_t *)b;

    //if base < compare 
    if (arg1 < arg2) return -1;
    //if base > compare
    if (arg1 > arg2) return 1;
    //if equal
    return 0;
}

//function to calculate median of given uint32_t array and size
uint32_t calculateMedian_uint32(uint32_t *samples, uint8_t size) {
    //Handle empty array case
    if (size == 0) {
        return 0;
    }
    
    //create temporary copy of samples so the original order is not changed
    uint32_t temp_samples[size];
    memcpy(temp_samples, samples, sizeof(temp_samples));

    //use qsort from stdlib to sort the samples
    qsort(temp_samples, size, sizeof(uint32_t), compare_uint32);

    //return the median value
    //check if size is even or odd
    if (size % 2 == 0) {
        // average of two middle values
        return (temp_samples[size / 2 - 1] + temp_samples[size / 2]) / 2;
    } else {
        // exact middle value
        return temp_samples[size / 2];
    }
}