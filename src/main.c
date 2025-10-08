#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <usart.h>
#include <math.h> 
#include <util/delay.h> 
#include <inttypes.h>
#include <avr/interrupt.h>

#define MAX_SAMPLES 10
#define FREQ_BUZZER 440

//only used in interrupt routine
volatile uint32_t echoTimeStart; // time when echo is received
volatile uint8_t sampleIndex = 0; // current index in samples array

uint32_t timeDiffSamples[MAX_SAMPLES] = {0}; // array to hold last 10 samples of distances
uint32_t TimeSinceLastTrigger; // time since last trigger of sonar sensor
uint32_t usartPrintStartTime; // time since last USART print
uint32_t medianTimeDiff = 0; // median of last 10 samples
uint32_t distance = 0; // distance in cm
uint32_t microseconds = 0;
char timer0CompareValueA;

//message buffer used to transmit distance over usart
char message[255] = "";


uint32_t micros() {
    return (microseconds);
}

uint32_t millis() {
    return round(microseconds / 1000);
}

//pin change interrupt service routine for echo pin (PD5)
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

//timer0 compare interrupt service routine for buzzer
ISR(TIMER0_COMPA_vect) {
    // toggle PD3
    PORTD ^= (1 << PORTD3);
}

ISR(TIMER1_COMPA_vect) {
    // increment microseconds
    microseconds += 10;
}

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

    timer0CompareValueA = round(31250 / FREQ_BUZZER) - 1;
    OCR0A = timer0CompareValueA;
    return;
}

void initTimer1() {
    TCCR1B = (1 << CS11 | (1 << WGM12)); //prescaler 8, CTC mode
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
    OCR1A = 20;
    return;
}

int main() {
    USART_Init();
    USART_Transmit_Line("Hello, USART!");
    initSonarSensor();
    initBuzzer();
    initTimer0();
    initTimer1();
    sei(); // enable global interrupts

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