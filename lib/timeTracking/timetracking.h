#ifndef TIMETRACKING_H
#define TIMETRACKING_H

#include <stdint.h>

//returns time in microseconds since program start
uint32_t micros();

//returns time in milliseconds since program start
uint32_t millis();

//initializes timer1 to track time
void timerTrackingInit();

#endif