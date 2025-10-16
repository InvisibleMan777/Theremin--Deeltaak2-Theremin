#ifndef CALCULATEMEDIAN_H
#define CALCULATEMEDIAN_H

#include <stdint.h>

//function to calculate median of given uint32_t array and size, and store result in returnPointer
void calculateMedian_uint32(const uint32_t *samples, uint8_t size, uint32_t *returnPointer);

#endif