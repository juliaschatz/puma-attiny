#include "main.h"

#ifndef __AVR_ATtiny20__
#define __AVR_ATtiny20__
#endif

#include <avr/interrupt.h>
#include <avr/io.h>

volatile uint16_t compare_level = 0;
volatile char direction = 0;
volatile uint8_t sleep_on = 1;
int led_on = 0;
int main() {
    // Configure system clock prescaler (0000 is factor of 1 for 8 MHz operation)
    CCP = 0xD8;
    CLKPSR &= 0b0000;

    // Timer 1 setup (Used for counting pulse length)
    // Set timer
    TCCR1A = 0;
    TCCR1B = 0;
    SET_HIGH(TCCR1B, CS11); // Prescaler of 8
    SET_HIGH(TIMSK, TOIE1); // Enable overflow interrupt (used for keeping longer time)

    // Input PWM signal
    // Configure pin change interrupt on PA0 
    SET_HIGH(PCMSK0, PCINT0);
    SET_HIGH(GIMSK, PCIE0);
    // Configure A0 as input
    SET_LOW(DDRA, DDA0);
    // Disable pullup resistor (set high-Z)
    SET_LOW(PUEA, PUEA0);

    // Timer 0 configure
    // Configure output PWM timer (phase correct PWM, output high when timer above level)
    SET_HIGH(TCCR0B, CS00); // Prescaler of 1
    SET_HIGH(TCCR0A, WGM00); // Phase correct PWM, 255 is top

    sei(); // Enable interrupts

    // Brake/coast jumper
    // Configure PA1 as internal-pullup logic input
    SET_LOW(DDRA, DDA1);
    SET_HIGH(PUEA, PUEA1);

    // LED outputs
    // Configure PA3 and PA6 as outputs and disable pullups
    SET_HIGH(DDRA, DDA6);
    SET_LOW(PUEA, PUEA6);
    SET_HIGH(DDRA, DDA3);
    SET_LOW(PUEA, PUEA3);

    // IN1, IN2, NSLEEP setup
    // Configure PA4,7, PB2 as outputs, disable pullups
    SET_HIGH(DDRA, DDA4);
    SET_LOW(PUEA, PUEA4);
    SET_HIGH(DDRA, DDA7);
    SET_LOW(PUEA, PUEA7);
    SET_HIGH(DDRB, DDB2);
    SET_LOW(PUEB, PUEB2);

    // Default output state
    SET_HIGH(PORTA, PINA4); // NSLEEP
    SET_LOW(PORTB, PINB2); // IN1
    SET_LOW(PORTA, PINA7); // IN2
    SET_LOW(PORTA, PINA6); // Green LED
    SET_LOW(PORTA, PINA3); // Red LED

    // Startup flash
    GREEN_ON;
    sync_delay_ms(500);
    RED_ON;
    sync_delay_ms(250);
    GREEN_OFF;
    sync_delay_ms(250);
    RED_OFF;

    while (1) {
        // Manage LEDs
        // Check if sleep is enabled
        if (sleep_on) {
            // Turn off LEDs
            GREEN_OFF;
            RED_OFF;
        }
        else if (direction == 0) { // Neutral
            // Turn on both LEDs
            GREEN_ON;
            RED_ON;
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
            SET_LOW(PORTA, off_pin);
            if (led_on) {
                led_on = 0;
                SET_LOW(PORTA, led_pin);
            }
            else {
                led_on = 1;
                SET_HIGH(PORTA, led_pin);
            }
            sync_delay_ms(255-compare_level);
        }
    }

    return 0;
}

/* 
 * Calculates input pulse length.
 */
ISR(PCINT0_vect) {
    char pin_state = REGISTER_BIT(PINA, PORTA0);
    // Copy to prevent timer changing during calculation
    // Due to (possibly?) opto turnoff time, pulses are measured to be about 30us more than they are
    unsigned short _time = TCNT1 - TIME_OFFSET;  
    // If pin moving LOW, new pulse beginning
    if (!pin_state) {
        TCNT1 = 0; // Reset timer
    } // If pin moving HIGH, pulse ending
    else {
        // Verify this is a valid signal by making sure it's in the valid length
        if (_time >= MIN_PULSE_LEN - FUDGE && _time <= MAX_PULSE_LEN + FUDGE) {
            if (_time < MIN_PULSE_LEN) {
                _time = MIN_PULSE_LEN;
            }
            if (_time > MAX_PULSE_LEN) {
                _time = MAX_PULSE_LEN;
            }
            // Disable sleep
            SLEEP_OFF;
            sleep_on = 0;

            // REVERSE is PWM on IN2 (IN1 low)
            // FWD is PWM on IN1 (IN2 low)

            // Deadzone/neutral state
            if (_time > DEADZONE_LOW && _time < DEADZONE_HIGH) {
                // If brake pin is unset, use coast mode

                // Disable output compare
                OC0A_OFF;
                OC0A_ON;
                // Set high or low for brake/coast
                char brake = (~REGISTER_BIT(PINA, PORTA1)) & 1;
                SET_REGISTER(PORTB, PINB2, brake);
                SET_REGISTER(PORTA, PINA7, brake);

                direction = 0;
            }
            else {
                // Calculate compare level
                // Set to both OCR0A and OCR0B
                // Enable or disable OC0A and OC0B as necessary
                if (_time > DEADZONE_CENTER) {
                    uint32_t temp = (255 * (uint32_t)(_time - DEADZONE_HIGH));
                    compare_level = temp / IN_RANGE;
                    // Disable OC0A and set high for brake in off cycle
                    OC0A_OFF;
                    SET_HIGH(PORTB, PINB2);
                    // Set OC0B on
                    OC0A_ON;

                    direction = 1;
                }
                else {
                    uint32_t temp = (255 * (uint32_t)(DEADZONE_LOW - _time));
                    compare_level = temp / IN_RANGE;
                    // Set OC0A on
                    OC0A_ON;
                    // Disable OC0B and set high for brake in off cycle
                    OC0B_OFF;
                    SET_HIGH(PORTA, PINA7);

                    direction = -1;
                }
                OCR0A = compare_level;
                OCR0B = compare_level;
            }
        }
        else {
            SLEEP_ON; // Disable output because this pulse is too long or too short
        }
    }

}

ISR(TIM1_OVF_vect) {
    // Overflows happen every ~65 ms
    sleep_on = 1;
    SLEEP_ON; 
}

void sync_delay_ms(uint16_t ms) {
    for (uint16_t i = 0; i < ms; ++i) {
        __builtin_avr_delay_cycles (8000);
    }
}