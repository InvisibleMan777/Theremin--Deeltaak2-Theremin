#include <stdint.h>
#include "distanceToFrequency.h"

//function to map distance to frequency linearly
double DistanceToFrequency(uint32_t distance, uint32_t dmin, uint32_t dmax, uint32_t fmin, uint32_t fmax) {
    //casting to double to prevent integer division (which would result in 0 for distances < dmax)
    double frequency = ((dmax - distance) / (double) (dmax - dmin)) * (fmax - fmin) + fmin;
    return frequency;
}