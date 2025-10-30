#include "../src/main.h"
#include "queueType.h"
#include "medianCalculator.h"
#include "../src/constants.h"
#include <math.h>
#include <avr/io.h>

#include "sonarStateMachine.h"

uint32_t timeSinceTriggerStart; // time since last trigger of sonar sensor

//state machine for sonar sensor
void sonarStateMachineStep(enum SonarState *sonarState, uint32_t time_micros, Queue_uint32 *measurementSamples, uint32_t *distance) {
    switch (*sonarState) {
        case READY_FOR_TRIGGER:
            //enable trigger pin
            PORTC |= (1 << PORTC1);

            //start timer for trigger duration
            timeSinceTriggerStart = time_micros;

            *sonarState = SENDING_TRIGGER;
            break;

        case SENDING_TRIGGER:
            //turn trigger pin off again after 10 microseconds
            if (time_micros - timeSinceTriggerStart > 10) {
                PORTC &= ~(1 << PORTC1);
                *sonarState = WAITING_FOR_ECHO;
            }
            break;

        case WAITING_FOR_ECHO:
            //check if echo has been received
            if (echoReceivedFlag) {
                //reset flag
                echoReceivedFlag = 0;
                //move to next state
                *sonarState = ECHO_RECEIVED;
            }
            break;

        case ECHO_RECEIVED:
            //save time difference in samples queue
            enqueue_uint32(measurementSamples, latestMeasurement);
            //remove oldest sample
            dequeue_uint32(measurementSamples);

            uint32_t medianTimeDiff = 0;

            //calculate median of the samples, store in medianTimeDiff
            calculateMedian_uint32(measurementSamples->data, measurementSamples->size, &medianTimeDiff);

            //calculate distance in mm: distance = (timeDiff * speed of sound) / 2
            *distance = round((medianTimeDiff * 0.343) / 2);

            //cap distance to MAX_DISTANCE_MM and MIN_DISTANCE_MM
            if (*distance < MIN_DISTANCE_MM) {
                *distance = MIN_DISTANCE_MM;
            } else if (*distance > MAX_DISTANCE_MM) {
                *distance = MAX_DISTANCE_MM;
            }

            //set ready for next trigger
            *sonarState = READY_FOR_TRIGGER;
            break;

        default:
            //should never happen
            break;
    }
}