#ifndef SONAR_STATE_MACHINE_H
#define SONAR_STATE_MACHINE_H

#include "../src/main.h"
#include "queueType.h"

extern uint32_t timeSinceTriggerStart;

void sonarStateMachineStep(enum SonarState *sonarState, uint32_t time_micros, Queue_uint32 *measurementSamples, uint32_t *distance);

#endif