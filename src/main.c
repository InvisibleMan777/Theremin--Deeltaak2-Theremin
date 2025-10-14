//standard libraries
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <math.h> 
#include <util/delay.h>

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

//only used in interrupt routine
volatile uint32_t echoTimeStart; // time when echo is received
volatile uint8_t sampleIndex = 0; // current index in samples array

//global variables
uint32_t timeDiffSamples[MAX_SAMPLES] = {0}; // array to hold last MAX samples of distances
uint32_t TimeSinceLastTrigger; // time since last trigger of sonar sensor
uint32_t usartPrintStartTime; // time since last USART print
uint32_t medianTimeDiff = 0; // median of last MAX samples
uint32_t distance = 0; // distance in cm
uint16_t FREQ_BUZZER = 440; //frequency of buzzer in Hz

//buffers
char timer0CompareValueA; //buffer for OCR0A to set buzzer frequency
char message[255] = ""; //message buffer used to transmit distance over usart

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
        //trigger sonar every x ms
        if (millis() - TimeSinceLastTrigger > 1) {
            //enable trigger pin for 10 microseconds
            PORTD |= (1 << PORTD4);
            _delay_us(10);
            PORTD &= ~(1 << PORTD4);
            //reset timer
            TimeSinceLastTrigger = millis();
        }

        // calculate median of last 10 samples
        medianTimeDiff = calculateMedian_uint32(timeDiffSamples, MAX_SAMPLES);

        //calculate distance in mm: distance = (timeDiff * speed of sound) / 2
        distance = round((medianTimeDiff * 0.343) / 2);

        //cap distance to MAX_DISTANCE_MM and MIN_DISTANCE_MM
        if (distance < MIN_DISTANCE_MM) {
            distance = MIN_DISTANCE_MM;
        } else if (distance > MAX_DISTANCE_MM) {
            distance = MAX_DISTANCE_MM;
        }

        //mapping distance to frequency negatively linearly: frequenty = ((dmax - distance + dmin) / (dmax - dmin)) * (fmax - fmin) + fmin
        //casting to double to prevent integer division (which would result in 0 for distances < dmax)
        FREQ_BUZZER = round(((MAX_DISTANCE_MM - distance + MIN_DISTANCE_MM) / (double)(MAX_DISTANCE_MM - MIN_DISTANCE_MM)) * (MAX_FREQ_HZ - MIN_FREQ_HZ) + MIN_FREQ_HZ);

        //update timer0 compare value for buzzer frequency
        timer0CompareValueA = round(31250 / FREQ_BUZZER) - 1;
        OCR0A = timer0CompareValueA;

        // print distance every x ms
        if (millis() - usartPrintStartTime > 100) {
            //load distance into message buffer
            sprintf(message, "distance: %lu", distance);
            //trasmit message buffer and reset timer
            USART_Transmit_Line(message);
            usartPrintStartTime = millis();
        }
    }

    return 0;
}