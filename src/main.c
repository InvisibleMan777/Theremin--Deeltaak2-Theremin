#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <usart.h>
#include <Arduino.h>
#include <math.h> 
#include <util/delay.h> 
#include <inttypes.h>

// fuck you ardiono and your stupid macros that redefine existing functions
#define round(x) round(x)

#define MAX_SAMPLES 10

//only used in interrupt routine
volatile uint32_t echoTimeStart; // time when echo is received
volatile uint8_t sampleIndex = 0; // current index in samples array

uint32_t timeDiffSamples[MAX_SAMPLES] = {0}; // array to hold last 10 samples of distances
uint32_t TimeSinceLastTrigger; // time since last trigger of sonar sensor
uint32_t usartPrintStartTime; // time since last USART print
uint32_t medianTimeDiff = 0; // median of last 10 samples
uint32_t distance = 0; // distance in cm

//message buffer used to transmit distance over usart
char message[255] = "";

//function to calculate median of given uint32_t array and size
uint32_t calculateMedian_uint32(uint32_t *samples, uint8_t size) {
    // simple bubble sort to sort the samples
    for (uint8_t i = 0; i < size - 1; i++) {
        for (uint8_t j = 0; j < size - i - 1; j++) {
            if (samples[j] > samples[j + 1]) {
                uint32_t temp = samples[j];
                samples[j] = samples[j + 1];
                samples[j + 1] = temp;
            }
        }
    }

    //return the median value
    //check if size is even or odd
    if (size % 2 == 0) {
        // average of two middle values
        return (samples[size / 2 - 1] + samples[size / 2]) / 2;
    } else {
        // exact middle value
        return samples[size / 2];
    }
}

//initialize regestries for sonar sensor
void initSonarSensor() {
    // initalize trigger pin (PD4) as output
    DDRD |= (1 << DDD4);

    // initialize interrupt on echo pin (PD5)
    sei(); // enable global interrupts
    PCICR |= (1 << PCIE2); // enable pin change interrupt for PORTD
    PCMSK2 |= (1 << PCINT21); // enable interrupt for PIND5
    return;
}

//initialize regestries for buzzer
void initBuzzer() {
    
}

//pin change interrupt service routine for echo pin (PD4)
ISR(PCINT2_vect) {
    //start timing on rising edge
    if (PIND & (1 << PIND5)) {
            echoTimeStart = micros();
        //replace oldest sample with new sample on falling edge
        } else if (!(PIND & (1 << PIND5))) {
            timeDiffSamples[sampleIndex] = micros() - echoTimeStart;
            //wrap around last index
            sampleIndex = (sampleIndex + 1) % MAX_SAMPLES;
        }
    }

int main() {
    init();
    USART_Init();
    USART_Transmit_Line("Hello, USART!");
    initSonarSensor();

    // main loop
    for(;;) {
        //trigger trigger pin every 100ms
        if (millis() - TimeSinceLastTrigger > 100) {
            PORTD |= (1 << PORTD4);
            _delay_us(10); // 10 microsecond pulse
            PORTD &= ~(1 << PORTD4);
            TimeSinceLastTrigger = millis();
        }

        // calculate median of last 10 samples
        medianTimeDiff = calculateMedian_uint32(timeDiffSamples, MAX_SAMPLES);

        distance = round((medianTimeDiff * 0.343) / 2);

        // print distance every 500ms
        if (millis() - usartPrintStartTime > 500) {
            //load distance into message buffer
            sprintf(message, "distance: %lu", distance);
            //trasmit message buffer and reset timer
            USART_Transmit_Line(message);
            usartPrintStartTime = millis();
        }
    }

    return 0;
}