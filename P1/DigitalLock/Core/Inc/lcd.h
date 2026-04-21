#ifndef LCD_H
#define LCD_H

#include "stm32l4xx_hal.h"
#include <stdint.h>

#define LCD_DATA_PIN_MASK (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7)
#define LCD_RS (GPIO_PIN_8)
#define LCD_RW (GPIO_PIN_9)
#define LCD_E (GPIO_PIN_10)

#define CNT_BTN (GPIO_PIN_11)

#define LCD_PORT GPIOB

#define LCD_CMD_FS 0x3C
#define LCD_CMD_DISP 0x0F
#define LCD_CMD_CLEAR 0x01
#define LCD_CMD_ENTRY 0x06
#define LCD_CMD_HOME 0x02

#define LCD_DDRAM_SET 0x80

#define LCD_CURS_SHFT 0x10
#define LCD_MOVE_CURS 0x00
#define LCD_MOVE_LEFT 0x00
#define LCD_MOVE_RIGHT 0x04

#define LCD_MAX_CHARS 32
#define LCD_COLS 16
#define LCD_ROWS 2

#define LCD_COL_ONE 1
#define LCD_ROW_TWO 2

#define TYPING_SPEED 100

#define W_CMD 0
#define W_DAT 1

void LCD_GPIO_Init();
void LCD_startup();
void LCD_write_string(char* top, char* bottom);
void LCD_move_cursor(uint8_t col, uint8_t row);
void LCD_backspace(uint8_t count);
void LCD_write(uint8_t data, uint8_t mode);
void LCD_clear();

#endif