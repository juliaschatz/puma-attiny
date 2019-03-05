#include "main.h"

#ifndef __AVR_ATtiny20__
#define __AVR_ATtiny20__
#endif

#include <avr/interrupt.h>
#include <avr/io.h>

volatile unsigned char compare_level = 0;
volatile char direction = 0;
volatile unsigned char outer_time = 0;
int main() {
    // Configure system clock prescaler (0000 is factor of 1 for 8 MHz operation)
    CCP = 0xD8;
    CLKPSR &= 0b0000;

    // Set timer
    SET_HIGH(TCCR1A, CS10); // 16 bit timer, prescale of 1
    SET_HIGH(TIMSK, TOIE1); // Enable overflow interrupt (used for keeping longer time)

    // Input PWM signal
    // Configure pin change interrupt on PA0 
    SET_HIGH(PCMSK0, PCINT0);
    SET_HIGH(GIMSK, PCIE0);
    // Configure A0 as input
    SET_LOW(DDRA, DDA0);
    // Disable pullup resistor (set high-Z)
    SET_LOW(PUEA, PUEA0);

    // Configure output PWM timer (phase correct PWM, output high when timer above level)
    SET_LOW(TCCR0B, WGM02);
    SET_LOW(TCCR0B, WGM01);
    SET_HIGH(TCCR0B, WGM00);

    sei(); // Enable interrupts

    // Brake/coast jumper
    // Configure PA1 as internal-pullup logic inputs
    SET_LOW(DDRA, DDA1);
    SET_HIGH(PUEA, PUEA1);

    // LED outputs
    // Configure PA3 and PA6 as outputs and disable pullups
    SET_HIGH(DDRA, DDA6);
    SET_LOW(PUEA, PUEA6);
    SET_HIGH(DDRA, DDA3);
    SET_LOW(PUEA, PUEA3);

    // IN1, IN2, NSLEEP setup
    // Configure PA4,7, PB2 as outputs
    SET_HIGH(DDRA, DDA4);
    SET_LOW(PUEA, PUEA4);
    SET_HIGH(DDRA, DDA7);
    SET_LOW(PUEA, PUEA7);
    SET_HIGH(DDRB, DDB2);
    SET_LOW(PUEB, PUEB2);

    // Default output state
    SET_LOW(PINA, PINA4); // NSLEEP
    SET_LOW(PINB, PINB2); // IN1
    SET_LOW(PINA, PINA7); // IN2

    for (;;) {
        // If the counter passes the limit, set the sleep
        if (TCNT1 >= DEAD_TIME) {
            // Enable sleep
            SET_LOW(PINA, PINA4);
        }

        // Manage LEDs
        // Check if sleep is enabled
        if (REGISTER_BIT(PINA, PINA4)) {
            // Turn off LEDs
            SET_LOW(PINA, PINA3);
            SET_LOW(PINA, PINA6);
        }
        else if (direction == 0) { // Neutral
            // Turn on both LEDs
            SET_HIGH(PINA, PINA3);
            SET_HIGH(PINA, PINA6);
        }
        else {
            // Choose the right pin
            char led_pin;
            char off_pin;
            if (direction > 0) {
                led_pin = PINA6;
                off_pin = PINA3;
            }
            else {
                led_pin = PINA3;
                off_pin = PINA6;
            }
            SET_LOW(PINA, off_pin);
            // Turn on if timer is below level (longer period for higher duty cycle PWM)
            if (2 * outer_time > compare_level) {
                SET_HIGH(PINA, led_pin);
            }
            else {
                SET_LOW(PINA, led_pin);
            }
        }
    }

    return 0;
}

/* 
 * Calculates input pulse length.
 */
ISR(PCINT0_vect) {
    char pin_state = REGISTER_BIT(PINA, PINA0);
    // Copy to prevent timer changing during calculation
    unsigned short _time = TCNT1;
    // If pin moving LOW, new pulse beginning
    if (!pin_state) {
        TCNT1 = 0; // Reset timer
    } // If pin moving HIGH, pulse ending
    else {
        // Verify this is a valid signal by making sure it's in the valid length
        if (_time >= MIN_PULSE_LEN && _time <= MAX_PULSE_LEN) {
            // Disable sleep
            SET_HIGH(PINA, PINA4);

            // REVERSE is PWM on IN2 (IN1 low)
            // FWD is PWM on IN1 (IN2 low)

            // Deadzone/neutral state
            if (_time > DEADZONE_LOW && _time < DEADZONE_HIGH) {
                // If brake pin is unset, use coast mode
                char brake = (~REGISTER_BIT(PINA, PINA1)) & 1;
                SET_REGISTER(PINB, PINB2, brake);
                SET_REGISTER(PINA, PINA7, brake);

                // Set LEDs both on
                SET_HIGH(PINA, PINA3);
                SET_HIGH(PINA, PINA6);
                direction = 0;
            }
            else {
                // Calculate compare level
                // Set to both OCR0A and OCR0B
                // Enable or disable OC0A and OC0B as necessary
                // Set LEDs
                if (_time > DEADZONE_CENTER) {
                    compare_level = (255 * (_time - DEADZONE_HIGH)) / IN_RANGE;
                    // Disable OC0A and set high for brake in off cycle
                    SET_LOW(TCCR0A, COM0A1);
                    SET_LOW(TCCR0A, COM0A0);
                    SET_HIGH(PINB, PINB2);
                    // Set OC0B on
                    SET_HIGH(TCCR0A, COM0B1);
                    SET_HIGH(TCCR0A, COM0B1);

                    direction = 1;
                }
                else {
                    compare_level = (255 * (DEADZONE_LOW - _time)) / IN_RANGE;
                    // Set OC0A on
                    SET_HIGH(TCCR0A, COM0A1);
                    SET_HIGH(TCCR0A, COM0A0);
                    // Disable OC0B and set high for brake in off cycle
                    SET_LOW(TCCR0A, COM0B1);
                    SET_LOW(TCCR0A, COM0B1);
                    SET_HIGH(PINA, PINA7);

                    direction = -1;
                }
                OCR0A = compare_level;
                OCR0B = compare_level;
            }
        }
    }

}

ISR(TIM1_OVF_vect) {
    outer_time++;
}