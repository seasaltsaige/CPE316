#ifndef KEYPAD_H
#define KEYPAD_H

#include <stdint.h>
#include <stm32l4xx_hal.h>

#define KEYPAD_PORT GPIOC

#define NUM_ROWS 4
#define NUM_COLS 4

#define ROW_PINS (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3)
extern const uint16_t ROW_MAP[NUM_ROWS];
#define COL_PINS (GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
extern const uint16_t COL_MAP[NUM_COLS];

extern const char keypad_map[NUM_ROWS][NUM_COLS];

// Maybe not the best, but -1 doesnt work here
#define KEYPAD_READ_ERR 1

void KEYPAD_GPIO_Init();
char read_keypad();

#endif