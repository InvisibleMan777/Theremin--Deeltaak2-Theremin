//standard libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h> 

//internal libraries
#include "usart.h"
#include "timetracking.h"
#include "medianCalculator.h"

//constants
#define MAX_SAMPLES 10 // number of samples to take for median filtering
#define MAX_DISTANCE_MM 650 // maximum distance measurable by the sensor in mm
#define MIN_DISTANCE_MM 20 // minimum distance measurable by the sensor in mm
#define MAX_FREQ_HZ 1400 // maximum frequency of the buzzer in Hz
#define MIN_FREQ_HZ 230 // minimum frequency of the buzzer in Hz

//states for sonar state machine
enum SonarState {
    READY_FOR_TRIGGER,
    SENDING_TRIGGER,
    WAITING_FOR_ECHO,
    ECHO_RECEIVED
};

//only used in interrupt routine
volatile uint32_t echoTimeStart; // time when echo is received
volatile uint8_t sampleIndex = 0; // current index of oldest sample in samples array

//global variables
uint32_t timeDiffSamples[MAX_SAMPLES] = {0}; // array to hold last MAX samples of distances
char echoReceivedFlag = 0; // flag to indicate if echo has been received by the sonar sensor

//pin change interrupt service routine for echo pin of the sonar sensor
ISR(PCINT2_vect) {
    //start timing on rising edge
    if (PIND & (1 << PIND5)) {
            echoTimeStart = micros();

    //stop timing on falling edge, save time difference in samples array
    } else if (!(PIND & (1 << PIND5))) {
            //replace oldest sample with new sample
            timeDiffSamples[sampleIndex] = micros() - echoTimeStart;
            //next oldest sample is one index higher, wrap around at last index using modulo (max+1 % max = 0)
            sampleIndex = (sampleIndex + 1) % MAX_SAMPLES;
            //set flag to indicate echo has been received
            echoReceivedFlag = 1;
    }
}

//timer0 compare interrupt service routine for buzzer
ISR(TIMER0_COMPA_vect) {
    //toggle PD3
    PORTD ^= (1 << PORTD3);
}

//initialize regestries for sonar sensor
void initSonarSensor() {
    DDRD |= (1 << DDD4); //initalize trigger pin (PD4) as output

    //enable interrupt
    PCICR |= (1 << PCIE2); // enable pin change interrupt for PORTD
    PCMSK2 |= (1 << PCINT21); // enable interrupt for PIND5
}

//initialize regestries for buzzer
void initBuzzer() {
    DDRD |= (1 << DDD3); // set PD3 (connected to buzzer) as output

    //init timer0
    TCCR0A = (1 << WGM01); // set CTC mode
    TCCR0B = (1 << CS02); // set prescaler to 256
    TIMSK0 |= (1 << OCIE0A); // enable timer compare interrupt for match A
}

int main() {
    enum SonarState sonarState = READY_FOR_TRIGGER; // current state of sonar state machine

    uint32_t timeSinceTriggerStart; // time since last trigger of sonar sensor
    uint32_t timeSinceLastUsartPrint; // time since last USART print
    uint32_t medianTimeDiff = 0; // median of last MAX samples
    uint32_t distance = 0; // distance in cm
    uint16_t frequencyBuzzer = 440; //frequency of buzzer in Hz, initialized at 440Hz (A4) but will be updated every cycle based on distance
    
    char message[255] = ""; //message buffer used to transmit distance over usart

    //initialize usart communication for debugging
    USART_Init();
    USART_Transmit_Line("Hello, USART!");
    //initialize time tracking so we can use millis() and micros()
    timerTrackingInit();
    //initialize sonar sensor and buzzer
    initSonarSensor();
    initBuzzer();
    //enable global interrupts
    sei(); 

    //main loop
    for(;;) {
        //state machine for sonar sensor
        switch (sonarState) {
            case READY_FOR_TRIGGER:
                //enable trigger pin
                PORTD |= (1 << PORTD4);
                timeSinceTriggerStart = micros();
                sonarState = SENDING_TRIGGER;
                break;

            case SENDING_TRIGGER:
                //disable trigger pin after 10 microseconds
                if (micros() - timeSinceTriggerStart > 10) {
                    PORTD &= ~(1 << PORTD4);
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
                //calculate median of last 10 samples
                medianTimeDiff = calculateMedian_uint32(timeDiffSamples, MAX_SAMPLES);
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

        //mapping distance to frequency negatively linearly: frequenty = ((dmax - distance + dmin) / (dmax - dmin)) * (fmax - fmin) + fmin
        //casting to double to prevent integer division (which would result in 0 for distances < dmax)
        frequencyBuzzer = round(((MAX_DISTANCE_MM - distance + MIN_DISTANCE_MM) / (double) (MAX_DISTANCE_MM - MIN_DISTANCE_MM)) * (MAX_FREQ_HZ - MIN_FREQ_HZ) + MIN_FREQ_HZ);

        //set frequency of buzzer by setting timer0 compare value, cast to uint8_t to make sure it fits in the register
        OCR0A = (uint8_t) round(31250 / frequencyBuzzer) - 1;

        // print distance every x ms (debug)
        if (millis() - timeSinceLastUsartPrint > 100) {
            //load distance into message buffer
            sprintf(message, "distance: %lu", distance);
            //trasmit message buffer and reset timer
            USART_Transmit_Line(message);
            timeSinceLastUsartPrint = millis();
        }
    }

    return 0;
}