#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

//export for use in InteruptServiceRoutines.c
extern uint8_t filterSize;
extern volatile uint32_t echoTimeStart;
extern char echoReceivedFlag;
extern uint32_t latestMeasurement;

#endif