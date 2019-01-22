#pragma once

/*
 * Red LED: PB2
 * Green LED: PA6
 * Input PWM: ~PA0
 * Brake: PA1 -> Low (activate internal pullup)
 * Coast: PA5 -> Low (activate internal pullup)
 * IN1: PA2
 * IN2: PA3
 * NSLEEP: PA4 (pull high to turn on driver)
 */

#define SET_HIGH(register, bit) register |= 1 << bit;
#define SET_LOW(register, bit) register &= ~(1 << bit);
#define REGISTER_BIT(register, bit) (register >> bit) & 1;

// Number of 5 uS counts of no signal to wait before disabling output
// 8000 is two signal periods
#define DEAD_TIME 8000