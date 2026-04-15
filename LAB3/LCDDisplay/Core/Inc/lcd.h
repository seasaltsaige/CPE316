#ifndef LCD_H
#define LCD_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define LCD_DATA_PIN_MASK (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
extern const 
#define LCD_RS (GPIO_PIN_8)
#define LCD_RW (GPIO_PIN_9)
#define LCD_E (GPIO_PIN_10)

#define LCD_PORT GPIOB

#define LCD_CMD_FS 0x38
#define LCD_CMD_DISP 0x0F
#define LCD_CMD_CLEAR 0x01
#define LCD_CMD_ENTRY 0x06
#define LCD_CMD_HOME 0x02

#define LCD_DDRAM_SET 0x80


#define LCD_MAX_CHARS 32

#define TYPING_SPEED 250


// R/W pin defs

// RS pin defs

// E pin defs


#define W_CMD 0
#define W_DAT 1

void LCD_startup();
void LCD_write_string(char* data, uint8_t length);
void LCD_write(uint8_t data, uint8_t mode);

#endif