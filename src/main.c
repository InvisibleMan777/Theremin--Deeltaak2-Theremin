#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h> 
#include <inttypes.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "usart.h"
#include "timetracking.h"
#include "medianCalculator.h"

#define MAX_SAMPLES 10 // number of samples to take for median filtering
#define MAX_DISTANCE_MM 650 // maximum distance measurable by the sensor in mm
#define MIN_DISTANCE_MM 20 // minimum distance measurable by the sensor in mm
#define MAX_FREQ_HZ 1400 // maximum frequency of the buzzer in Hz
#define MIN_FREQ_HZ 230 // minimum frequency of the buzzer in Hz

//only used in interrupt routine
volatile uint32_t echoTimeStart; // time when echo is received
volatile uint8_t sampleIndex = 0; // current index in samples array

uint32_t timeDiffSamples[MAX_SAMPLES] = {0}; // array to hold last 10 samples of distances
uint32_t TimeSinceLastTrigger; // time since last trigger of sonar sensor
uint32_t usartPrintStartTime; // time since last USART print
uint32_t medianTimeDiff = 0; // median of last 10 samples
uint32_t distance = 0; // distance in cm
uint16_t FREQ_BUZZER = 440; //frequency of buzzer in Hz

char timer0CompareValueA; //buffer for OCR0A to set buzzer frequency
char message[255] = ""; //message buffer used to transmit distance over usart

//pin change interrupt service routine for echo pin of the sonar sensor
ISR(PCINT2_vect) {
    //start timing on rising edge
    if (PIND & (1 << PIND5)) {
            echoTimeStart = micros();

    //stop timing on falling edge, save time difference in samples array
    } else if (!(PIND & (1 << PIND5))) {
            //replace oldest sample with new sample on falling edge
            timeDiffSamples[sampleIndex] = micros() - echoTimeStart;
            //wrap around last index
            sampleIndex = (sampleIndex + 1) % MAX_SAMPLES;
        }
    }

//timer0 compare interrupt service routine for buzzer
ISR(TIMER0_COMPA_vect) {
    // toggle PD3
    PORTD ^= (1 << PORTD3);
}

//initialize regestries for sonar sensor
void initSonarSensor() {
    // initalize trigger pin (PD4) as output
    DDRD |= (1 << DDD4);

    // initialize interrupt on echo pin (PD5)
    PCICR |= (1 << PCIE2); // enable pin change interrupt for PORTD
    PCMSK2 |= (1 << PCINT21); // enable interrupt for PIND5
    return;
}

//initialize regestries for buzzer
void initBuzzer() {
    // set PD3 as output for buzzer
    DDRD |= (1 << DDD3);
    return;
}

void initTimer0() {
    TCCR0A = (1 << WGM01); // set CTC mode
    TCCR0B = (1 << CS02); // set prescaler to 256
    TIMSK0 |= (1 << OCIE0A); // enable timer compare interrupt for match A
    return;
}

int main() {
    USART_Init();
    USART_Transmit_Line("Hello, USART!");
    initSonarSensor();
    initBuzzer();
    initTimer0();
    timerTrackingInit();
    sei(); // enable global interrupts

    // main loop
    for(;;) {
        //trigger trigger pin every 100ms
        if (millis() - TimeSinceLastTrigger > 1) {
            PORTD |= (1 << PORTD4);
            _delay_us(10); // 10 microsecond pulse
            PORTD &= ~(1 << PORTD4);
            TimeSinceLastTrigger = millis();
        }

        // calculate median of last 10 samples
        medianTimeDiff = calculateMedian_uint32(timeDiffSamples, MAX_SAMPLES);

        distance = round((medianTimeDiff * 0.343) / 2);

        //set distance to MIN if smaller then MIN, buzzer stops buzzing when distance is greater than max
        if (distance < MIN_DISTANCE_MM) {
            distance = MIN_DISTANCE_MM;
        } else if (distance > MAX_DISTANCE_MM) {
            distance = MAX_DISTANCE_MM;
        }

        //mapping distance to frequency linearly: frequenty = ((dmax - distance + dmin) / (dmax - dmin)) * (fmax - fmin) + fmin
        //casting to double to prevent integer division (which would result in 0 for distances < dmax)
        FREQ_BUZZER = round(((MAX_DISTANCE_MM - distance + MIN_DISTANCE_MM) / (double)(MAX_DISTANCE_MM - MIN_DISTANCE_MM)) * (MAX_FREQ_HZ - MIN_FREQ_HZ) + MIN_FREQ_HZ);

        //update timer0 compare value for buzzer frequency
        timer0CompareValueA = round(31250 / FREQ_BUZZER) - 1;
        OCR0A = timer0CompareValueA;

        // print distance every 500ms
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