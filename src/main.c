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
    //interupt is triggered on both rising and falling edge of echo pin 
    switch (PIND & (1 << PIND5)) {
        //rising edge
        case (1 << PIND5):
            //start timing
            echoTimeStart = micros();
            break;
            
        //falling edge
        case 0:
            //save time difference in samples array
            //replace oldest sample with new sample
            timeDiffSamples[sampleIndex] = micros() - echoTimeStart;
            //next oldest sample is one index higher, wrap around at last index using modulo (max+1 % max = 0)
            sampleIndex = (sampleIndex + 1) % MAX_SAMPLES;
            //set flag to indicate echo has been received
            echoReceivedFlag = 1;
            break;

        default:
            //should never happen
            break;
    }
}

//timer0 compare interrupt service routine for buzzer
ISR(TIMER0_COMPA_vect) {
    //toggle PD3 (buzzer) by toggling its data direction
    DDRD ^= (1 << DDD3);
}

//initialize regestries for sonar sensor
static void initSonarSensor() {
    DDRD |= (1 << DDD4); //initalize trigger pin (PD4) as output

    //enable interrupt
    PCICR |= (1 << PCIE2); // enable pin change interrupt for PORTD
    PCMSK2 |= (1 << PCINT21); // enable interrupt for PIND5
}

//initialize regestries for buzzer
static void initBuzzer() {
    //init timer0, used to create frequency for buzzer
    TCCR0A = (1 << WGM01); // set CTC mode
    TCCR0B = (1 << CS02); // set prescaler to 256
    TIMSK0 = (1 << OCIE0A); // enable timer compare interrupt for match A
    OCR0A = 77; // this register now controls the frequency of the buzzer, initialized at 400Hz (16MHz / (2 * 256 * 400Hz) - 1 = 77)

    //init timer2, used for volume control and output of buzzer
    TCCR2A = (1 << COM2B1 | 1 << WGM21 | 1 << WGM20); // set fast PWM mode, clear OC2B (connected to buzzer) on compare match, set at BOTTOM
    TCCR2B = (1 << CS20); // set prescaler to 1 (no prescaling)
    OCR2B = 25; // this register now controls the volume of the buzzer, initialized at ~10% duty cycle (25/255)
}

//initialize regestries for volume control (ADC)
static void initVolumeControl() {
    //init ADC, used for volume control
    ADMUX = (1 << ADLAR | 1 << REFS0); // set reference voltage to AVcc and select ADC0 (connected to potmeter) as input
    ADCSRA = (1 << ADEN | 1 << ADATE | 1 << ADSC |1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0); // enable ADC, enable auto trigger, start initial conversion, and set prescaler to 128
}

int main() {
    enum SonarState sonarState = READY_FOR_TRIGGER; // current state of sonar state machine

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
    //initialize sensors and actuators
    initVolumeControl();
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
                //calculate median of last 10 samples, store in medianTimeDiff
                calculateMedian_uint32(timeDiffSamples, MAX_SAMPLES, &medianTimeDiff);
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

        //mapping distance to frequency negatively linearly: frequenty = ((dmax - distance) / (dmax - dmin)) * (fmax - fmin) + fmin
        //casting to double to prevent integer division (which would result in 0 for distances < dmax)
        double frequencyBuzzer = ((MAX_DISTANCE_MM - distance) / (double) (MAX_DISTANCE_MM - MIN_DISTANCE_MM)) * (MAX_FREQ_HZ - MIN_FREQ_HZ) + MIN_FREQ_HZ;

        //set frequency of buzzer by setting timer0 compare value, cast to uint8_t to make sure it fits in the register
        OCR0A = (uint8_t) round(31250 / frequencyBuzzer) - 1;

        //set volume of buzzer by setting timer2 compare value based on ADC value (potmeter), both values are between 0 and 255 so direct mapping is possible
        //NOTE: ADCH is used so we leave out the 2 least significant bits of the ADC register, which are garbage due to noise
        OCR2B = ADCH;

        // print distance every x ms (debug)
        if (millis() - timeSinceLastUsartPrint > 100) {
            //load distance into message buffer, cast to unsigned long to prevent warning from cppcheck
            sprintf(message, "distance: %lu | frequency: %u | adc: %u", (unsigned long) distance, (uint16_t) round(frequencyBuzzer), ADCH);
            //trasmit message buffer and reset timer
            USART_Transmit_Line(message);
            timeSinceLastUsartPrint = millis();
        }
    }

    return 0;
}