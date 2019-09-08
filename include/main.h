#pragma once

#ifndef __AVR_ATtiny20__
#define __AVR_ATtiny20__
#endif

#include <avr/interrupt.h>
#include <avr/io.h>

/*
 * Red LED: PA3
 * Green LED: PA6
 * Input PWM: ~PA0
 * Brake: PA1 -> Low (activate internal pullup), otherwise Coast
 * IN1: PB2
 * IN2: PA7
 * NSLEEP: PA4 (pull high to turn on driver)
 */

#define SET_HIGH(register, bit) register |= (1 << bit)
#define SET_LOW(register, bit) register &= ~(1 << bit)
#define REGISTER_BIT(register, bit) ((register >> bit) & 1)
#define SET_REGISTER(register, bit, value) (value ? (SET_HIGH(register, bit)) : (SET_LOW(register, bit)))
#define GREEN_ON SET_HIGH(PORTA, PINA6)
#define GREEN_OFF SET_LOW(PORTA, PINA6)
#define RED_ON SET_HIGH(PORTA, PINA3)
#define RED_OFF SET_LOW(PORTA, PINA3)
#define SLEEP_OFF SET_HIGH(PORTA, PINA4);
#define SLEEP_ON SET_LOW(PORTA, PINA4);
#define OC0A_ON \
SET_HIGH(TCCR0A, COM0A0); \
SET_HIGH(TCCR0A, COM0A1);
#define OC0A_OFF \
SET_LOW(TCCR0A, COM0A0); \
SET_LOW(TCCR0A, COM0A1);
#define OC0B_ON \
SET_HIGH(TCCR0A, COM0B0); \
SET_HIGH(TCCR0A, COM0B1);
#define OC0B_OFF \
SET_LOW(TCCR0A, COM0B0); \
SET_LOW(TCCR0A, COM0B1);

// Smallest number of timer counts to register an input (1000 uS)
#define MIN_PULSE_LEN 1000

// Largest number of counts to register an input (2000 uS)
#define MAX_PULSE_LEN 2000

// Fudge factor of timer counts
#define FUDGE 20

#define DEADZONE_LOW 1450
#define DEADZONE_CENTER 1500
#define DEADZONE_HIGH 1550

#define IN_RANGE 450 // Difference between deadzone and end range

#define TIME_OFFSET 30

void sync_delay_ms(uint16_t ms);