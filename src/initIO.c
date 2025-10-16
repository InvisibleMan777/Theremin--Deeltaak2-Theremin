#include <avr/io.h>

#include "initIO.h"

//initialize regestries for sonar sensor
void initSonarSensor() {
    DDRC |= (1 << DDC1); //initalize trigger pin (PC1) as output

    //enable interrupt
    PCICR |= (1 << PCIE1); // enable pin change interrupt for PORTC
    PCMSK1 |= (1 << PCINT10); // enable interrupt for PC2
}

//initialize regestries for buzzer
void initBuzzer() {
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

//initialize regestries for volume control (potmeter / ADC)
void initVolumeControl() {
    //init ADC, reads potmeter value and converts to digital value used for volume control
    ADMUX = (1 << ADLAR | 1 << REFS0); // set reference voltage to AVcc, select ADC0 (connected to potmeter) as input, and left adjust result to allow easy 8 bit reading
    ADCSRA = (1 << ADEN | 1 << ADATE | 1 << ADSC |1 << ADPS2 | 1 << ADPS1 | 1 << ADPS0); // enable ADC, enable auto trigger, start initial conversion, and set prescaler to 128
}

//initialize regestries for filter size control (buttons)
void initFilterSizeControl() {
   PORTB |= (1 << PORTB0 | 1 << PORTB1); //enable pullup resistors on both buttons
   PCICR |= (1 << PCIE0); // enable pin change interrupt for PORTB
   PCMSK0 |= (1 << PCINT0 | 1 << PCINT1); // enable interrupt for PINB0 and PINB1
}