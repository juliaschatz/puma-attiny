#include "main.h"

#ifndef __AVR_ATtiny20__
#define __AVR_ATtiny20__
#endif

#include <avr/interrupt.h>
#include <avr/io.h>

volatile unsigned short in_low_counts = 0;
volatile char input_off_latch = 1;  // Records whether no signal has been received for more than DEAD_TIME

int main() {
    // Configure system clock prescaler (0001 is factor of 2 for 4 MHz operation)
    CCP = 0xD8;
    CLKPSR |= 0b0001;

    // Set timer
    SET_HIGH(TCCR1A, CS10) // 16 bit timer, prescale of 1
    // Set output compare register
    OCR1A = 19; // 5 uS resolution


    // Configure pin change interrupt on PA0
    SET_HIGH(PCMSK0, PCINT0)
    SET_HIGH(GIMSK, PCIE0)
    // Configure A0 as input (logic low)
    SET_LOW(DDRA, DDA0)
    // Disable pullup resistor (set high-Z)
    SET_LOW(PUEA, PUEA0)

    sei(); // Enable interrupts

    // Configure PA1 and PA5 as internal-pullup logic inputs
    SET_LOW(DDRA, DDA1)
    SET_HIGH(PUEA, PUEA1)
    SET_LOW(DDRA, DDA5)
    SET_HIGH(PUEA, PUEA5)

    // Configure PB2 and PA6 as outputs and disable pullups
    SET_HIGH(DDRA, DDA6)
    SET_LOW(PUEA, PUEA6)
    SET_HIGH(DDRB, DDB2)
    SET_LOW(PUEB, PUEB2)

    // Configure PA2,3,4 as outputs
    SET_HIGH(DDRA, DDA2)
    SET_LOW(PUEA, PUEA2)
    SET_HIGH(DDRA, DDA3)
    SET_LOW(PUEA, PUEA3)
    SET_HIGH(DDRA, DDA4)
    SET_LOW(PUEA, PUEA4)

    return 0;
}

/* 
 * Calculates input pulse length.
 */
ISR(PCINT0_vect) {
    char pin_state = REGISTER_BIT(PINA, PINA0)
    // If pin moving LOW, new pulse beginning
    if (!pin_state) {
        in_low_counts = 0;
    }
    // If pin moving HIGH, pulse ending
    if (!pin_state) {
        // Disable watchdog latch
        input_off_latch = 0;
        unsigned short pulse_length = 5 * in_low_counts;
    }

}

/*
 * Keeps track of time to 5 uS resolution
 */
ISR(TIM1_COMPA_vect) {
    in_low_counts++;
    if (in_low_counts >= DEAD_TIME) {
        input_off_latch = 1;
    }
}