#ifndef MAIN_H
#define MAIN_H

#include <stdint.h>

//states for sonar sensor state machine
enum SonarState {
    READY_FOR_TRIGGER,
    SENDING_TRIGGER,
    WAITING_FOR_ECHO,
    ECHO_RECEIVED
};

//export for use in InteruptServiceRoutines.c
extern uint8_t filterSize;
extern char echoReceivedFlag;
extern uint32_t latestMeasurement;

#endif