#include <avr/io.h>
#include <avr/interrupt.h>

#include "main.h"
#include "timetracking.h"
#include "constants.h"

volatile uint32_t echoTimeStart = 0; //start time of echo signal from sonar sensor

//pin change interrupt service routine for echo pin of the sonar sensor
ISR(PCINT1_vect) {
    //interupt is triggered on both rising and falling edge of echo pin
    switch (PINC & (1 << PINC2)) {
        //rising edge
        case (1 << PINC2):
            //start timing
            echoTimeStart = micros();
            break;
            
        //falling edge
        case 0:
            //store time
            latestMeasurement = micros() - echoTimeStart;
            //set flag to indicate echo has been received,  used in the sonar state machine in main loop
            echoReceivedFlag = 1;
            break;

        default:
            //should never happen
            break;
    }
}

//pin change interrupt service routine for buttons to change filter size
ISR(PCINT0_vect) {
    //button0 on falling edge
    if (!(PINB & (1 << PINB0)) && filterSize < MAX_SAMPLES) {
        filterSize += 2;
    }
    //button1 on falling edge
    if (!(PINB & (1 << PINB1)) && filterSize > MIN_SAMPLES) {
        //button on PINB1 was pressed, increase filter size
        filterSize -= 2;
    }
}

//timer0 compare interrupt service routine for buzzer
ISR(TIMER0_COMPA_vect) {
    //toggle PD3 (buzzer) by toggling its data direction
    DDRD ^= (1 << DDD3);
}