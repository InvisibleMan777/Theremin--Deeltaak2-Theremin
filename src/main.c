//standard libraries
#include <avr/io.h>
#include <stdio.h>
#include <math.h> 
#include <avr/interrupt.h>

//internal libraries
#include "usart.h"
#include "timetracking.h"
#include "medianCalculator.h"
#include "queueType.h"
#include "distanceToFrequency.h"

//local files
#include "constants.h"
#include "initIO.h"
#include "main.h"

//states for sonar sensor state machine
enum SonarState {
    READY_FOR_TRIGGER,
    SENDING_TRIGGER,
    WAITING_FOR_ECHO,
    ECHO_RECEIVED
};

//global variables
volatile uint32_t echoTimeStart = 0; //start time of echo signal from sonar sensor
char echoReceivedFlag = 0; // flag to indicate if echo has been fully received by the sonar sensor
uint32_t latestMeasurement = 0; //latest measurement from sonar sensor
uint8_t filterSize = MAX_SAMPLES; //current filter size, can be changed with the buttons, every cycle the queue size is ajusted to this
Queue_uint32 *measurementSamples; //queue containing the last <filterSize> amount of measurements from sonar sensor

int main() {
    enum SonarState sonarState = READY_FOR_TRIGGER; // current state of sonar state machine

    //variables for main loop
    uint32_t timeSinceTriggerStart = 0; // time since last trigger of sonar sensor
    uint32_t timeSinceLastUsartPrint = 0; // time since last USART print
    uint32_t distance = 0; // distance based on sonarsensor input in mm
    uint32_t medianTimeDiff = 0; // median of last MAX_SAMPLES time differences in microseconds
    char message[255] = ""; //message buffer used to transmit distance over usart

    //initialize usart communication for debugging
    USART_Init();
    USART_Transmit_Line("Hello, USART!");
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

    //initialize queue with zeros
    for (int i = 0; i < filterSize; i++) {
        enqueue_uint32(measurementSamples, 0);
    }

    //main loop
    for(;;) {
        //state machine for sonar sensor
        switch (sonarState) {
            case READY_FOR_TRIGGER:
                //enable trigger pin
                PORTC |= (1 << PORTC1);
                timeSinceTriggerStart = micros();
                sonarState = SENDING_TRIGGER;
                break;

            case SENDING_TRIGGER:
                //disable trigger pin after 10 microseconds
                if (micros() - timeSinceTriggerStart > 10) {
                    PORTC &= ~(1 << PORTC1);
                    sonarState = WAITING_FOR_ECHO;
                }
                break;

            case WAITING_FOR_ECHO:
                //check if echo has been received
                if (echoReceivedFlag) {
                    //reset flag
                    echoReceivedFlag = 0;
                    //move to next state
                    sonarState = ECHO_RECEIVED;
                }
                break;

            case ECHO_RECEIVED:
                //save time difference in samples queue
                enqueue_uint32(measurementSamples, latestMeasurement);
                //remove oldest sample
                dequeue_uint32(measurementSamples);

                //calculate median of the samples, store in medianTimeDiff
                calculateMedian_uint32(measurementSamples->data, measurementSamples->size, &medianTimeDiff);
                //calculate distance in mm: distance = (timeDiff * speed of sound) / 2
                distance = round((medianTimeDiff * 0.343) / 2);

                //cap distance to MAX_DISTANCE_MM and MIN_DISTANCE_MM
                if (distance < MIN_DISTANCE_MM) {
                    distance = MIN_DISTANCE_MM;
                } else if (distance > MAX_DISTANCE_MM) {
                    distance = MAX_DISTANCE_MM;
                }

                //set ready for next trigger
                sonarState = READY_FOR_TRIGGER;
                break;

            default:
                //should never happen
                break;
        }

        //convert distance to frequency for buzzer
        double frequencyBuzzer = DistanceToFrequency(distance, MIN_DISTANCE_MM, MAX_DISTANCE_MM, MIN_FREQ_HZ, MAX_FREQ_HZ);

        //set frequency of buzzer by setting timer0 compare value
        OCR0A = round(31250 / frequencyBuzzer) - 1;

        //set volume of buzzer by setting timer2 compare value based on ADC value (potmeter), both values are between 0 and 255 so direct mapping is possible
        //NOTE: ADCH is used so we leave out the 2 least significant bits of the ADC register, which are garbage due to noise
        OCR2B = ADCH;

        //check if filter size has changed
        int8_t samplesAmountInaccuracy = filterSize - measurementSamples->size;

        if (samplesAmountInaccuracy == 0) {
            //all good, do nothing
        } else if (samplesAmountInaccuracy > 0) {
            //fill queue with zeros to ajust to new filter size
            for (int i = 0; i < samplesAmountInaccuracy; i++) {
                enqueue_uint32(measurementSamples, 0);
            }
        } else if (samplesAmountInaccuracy < 0) {
            //remove oldest samples to ajust to new filter size
            for (int i = 0; i < -samplesAmountInaccuracy; i++) {
                dequeue_uint32(measurementSamples);
            }
        }

        // print distance every x ms (debug)
        if (millis() - timeSinceLastUsartPrint > 100) {
            //load distance into message buffer, cast to unsigned long to prevent warning from cppcheck
            sprintf(message, "distance: %lu | frequency: %u | adc: %u | filter size: %u", (unsigned long) distance, (uint16_t) round(frequencyBuzzer), ADCH, measurementSamples->size);
            //trasmit message buffer and reset timer
            USART_Transmit_Line(message);
            timeSinceLastUsartPrint = millis();
        }
    }

    return 0;
}