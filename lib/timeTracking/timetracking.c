#include <avr/io.h>
#include <avr/interrupt.h>
#include <math.h>

#include "timetracking.h"

volatile uint32_t microseconds = 0;

ISR(TIMER1_COMPA_vect) {
    // increment microseconds by 10 every 10 microseconds
    // the reason we do this instead of just incrementing by 1 every microsecond is because we dont want a interrupt every microsecond, that would be too much overhead
    microseconds += 10;
}

//returns time in microseconds since program start
uint32_t micros() {
    return (microseconds);
}

//returns time in milliseconds since program start
uint32_t millis() {
    return round(microseconds / 1000);
}

//initializes timer1 to track time
void timerTrackingInit() {
    TCCR1B = (1 << CS11 | (1 << WGM12)); //prescaler 8, CTC mode
    TIMSK1 |= (1 << OCIE1A); // enable timer compare interrupt
    OCR1A = 20; // interrupt every 10 microseconds
}