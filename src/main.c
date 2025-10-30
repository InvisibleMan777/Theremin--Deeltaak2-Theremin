//standard libraries
#include <avr/io.h>
#include <stdio.h>
#include <math.h> 
#include <avr/interrupt.h>

//internal libraries
#include "timetracking.h"
#include "medianCalculator.h"
#include "queueType.h"
#include "distanceToFrequency.h"

//local files
#include "constants.h"
#include "initIO.h"
#include "sonarStateMachine.h"

#include "main.h"

//global variables
char echoReceivedFlag = 0; // flag to indicate if echo has been fully received by the sonar sensor
uint32_t latestMeasurement = 0; //latest measurement from sonar sensor
uint8_t filterSize = MAX_SAMPLES; //current filter size, can be changed with the buttons, every cycle the queue size is ajusted to this

int main() {
    //local variables
    enum SonarState sonarState = READY_FOR_TRIGGER; // current state of sonar state machine
    Queue_uint32 *measurementSamples; //queue containing the last <filterSize> amount of measurements from sonar sensor
    uint32_t distance = 0; // distance based on sonarsensor input in mm

    //initialize time tracking so we can use millis() and micros()
    timerTrackingInit();
    //initialize I/O, functions found in initIO.c
    initVolumeControl();
    initFilterSizeControl();
    initSonarSensor();
    initBuzzer();
    //enable global interrupts
    sei(); 

    //initialize queue to store measurement samples of the sonar sensor
    measurementSamples = createQueue_uint32(filterSize);

    //initialize measurement samples with zeros
    for (int i = 0; i < filterSize; i++) {
        enqueue_uint32(measurementSamples, 0);
    }

    //main loop
    for(;;) {
        //step sonar state machine (will trigger sonar, wait for echo, and calculate distance when echo is received)
        sonarStateMachineStep(&sonarState, micros(), measurementSamples, &distance);

        //convert distance to frequency for buzzer
        double frequencyBuzzer = DistanceToFrequency(distance, MIN_DISTANCE_MM, MAX_DISTANCE_MM, MIN_FREQ_HZ, MAX_FREQ_HZ);
        //set frequency of buzzer by setting timer0 compare value
        OCR0A = round(31250 / frequencyBuzzer) - 1;
        
        //set volume of buzzer by setting timer2 compare value based on ADC value (potmeter), both values are between 0 and 255 so direct mapping is possible
        //NOTE: ADCH is used so we leave out the 2 least significant bits of the ADC register, which are garbage due to noise
        OCR2B = ADCH;

        //check if filter size has changed
        int8_t samplesAmountInaccuracy = filterSize - measurementSamples->size;

        switch (samplesAmountInaccuracy == 0) {
            case 1:
                //filter size is correct, do nothing
                break;
            case 0:
                if (samplesAmountInaccuracy > 0) {
                    //filter size increased, add zeros to queue
                    for (uint8_t i = 0; i < samplesAmountInaccuracy; i++) {
                        enqueue_uint32(measurementSamples, 0);
                    }
                } else {
                    //filter size decreased, remove oldest samples from queue
                    for (uint8_t i = 0; i < -samplesAmountInaccuracy; i++) {
                        dequeue_uint32(measurementSamples);
                    }
                }
                break;
        }
    }

    //should never reach this
    return 0;
}