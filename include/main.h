#pragma once

/*
 * Red LED: PA3
 * Green LED: PA6
 * Input PWM: ~PA0
 * Brake: PA1 -> Low (activate internal pullup), otherwise Coast
 * IN1: PB2
 * IN2: PA7
 * NSLEEP: PA4 (pull high to turn on driver)
 */

#define SET_HIGH(register, bit) register |= 1 << bit;
#define SET_LOW(register, bit) register &= ~(1 << bit);
#define REGISTER_BIT(register, bit) ((register >> bit) & 1)
#define SET_REGISTER(register, bit, value) register ^= (-value ^ register) & (1 << bit);

// Number of 8.192 ms counts of no signal to wait before disabling output
#define DEAD_TIME 5

// Smallest number of real timer counts to register an input (1000 uS)
#define MIN_PULSE_LEN 8000

// Largest number of counts to register an input (2000 uS)
#define MAX_PULSE_LEN 16000

#define DEADZONE_LOW 11800
#define DEADZONE_CENTER 12000
#define DEADZONE_HIGH 12200

#define IN_RANGE 3800