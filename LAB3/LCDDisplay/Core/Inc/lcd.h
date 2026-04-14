#ifndef LCD_H
#define LCD_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define LCD_DATA_PIN_MASK (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
#define LCD_RS (GPIO_PIN_8)
#define LCD_RW (GPIO_PIN_9)
#define LCD_E (GPIO_PIN_10)

// R/W pin defs
#define LCD_READ 0
#define LCD_WRITE 1

// RS pin defs
#define LCD_RS_INST 0
#define LCD_RS_DATA 1

// E pin defs
#define LCD_E_EN 1
#define LCD_E_DIS 0

#define W_CMD 0
#define W_DAT 1

void LCD_startup();
void LCD_write(uint8_t data, uint8_t mode);

#endif