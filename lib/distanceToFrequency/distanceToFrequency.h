#ifndef DISTANCE_TO_FREQUENCY_H
#define DISTANCE_TO_FREQUENCY_H

#include <stdint.h>

//function to map distance to frequency linearly
double DistanceToFrequency(uint32_t distance, uint32_t dmin, uint32_t dmax, uint32_t fmin, uint32_t fmax);

#endif