#include "lcd.h"
#include "stm32l4xx_hal.h"
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TYPING_VARIABILITY 300

void LCD_startup() {
    
    // Wait for more than 30ms on init
    HAL_Delay(100);

    // 0x38
    // 0011 1100 = Function set, 
    LCD_write(LCD_CMD_FS, W_CMD);
    
    // At least 40us
    HAL_Delay(1);
    // 0000 1111 = Disp on/off, disp=ON, curs=ON, blink=ON
    LCD_write(LCD_CMD_DISP, W_CMD);

    // At least 40us
    HAL_Delay(1);
    // 0000 0001 = Clear display
    LCD_write(LCD_CMD_CLEAR, W_CMD);

    // At least 1.64ms
    HAL_Delay(10);
    // 0000 0110 = Entry mode, increment mode, shift off
    LCD_write(LCD_CMD_ENTRY, W_CMD);

    return;
}

static void typing_delay() {
    HAL_Delay(TYPING_SPEED + (rand() % TYPING_VARIABILITY));
}

void LCD_write_string(char* top, char* bottom) {
    // Clear screen and home cursor
    LCD_clear();

    uint8_t i;

    // Write top half
    for (i = 0; (i < strlen(top)) && (i < LCD_COLS); i++) {
        LCD_write(top[i], W_DAT);
        typing_delay();
    }

    // Move cursor to first column of second row
    LCD_move_cursor(LCD_COL_ONE, LCD_ROW_TWO);
    
    for (i = 0; (i < strlen(bottom)) && (i < LCD_COLS); i++) {
        LCD_write(bottom[i], W_DAT);
        typing_delay();
    }

    return;
}

void LCD_move_cursor(uint8_t col, uint8_t row) {
    // Row 1 ddram len = 0x3F, so setting DDRAM address to
    // 0x40 will offset cursor to second row. 
    uint8_t move_cursor_cmd = LCD_DDRAM_SET | ((col - 1) + (0x40 * (row - 1)));
    LCD_write(move_cursor_cmd, W_CMD);
}

void LCD_backspace(uint8_t count) {
    for (uint8_t i = 0; i < count; i++)
        LCD_write(LCD_CURS_SHFT | LCD_MOVE_CURS | LCD_MOVE_LEFT , W_CMD);
    for (uint8_t i = 0; i < count; i++) 
        LCD_write(' ', W_DAT);
    for (uint8_t i = 0; i < count; i++)
        LCD_write(LCD_CURS_SHFT | LCD_MOVE_CURS | LCD_MOVE_LEFT , W_CMD);
}

void LCD_write(uint8_t data, uint8_t mode) {
    // Disable everything
    LCD_PORT->BRR |= (LCD_DATA_PIN_MASK | LCD_RW | LCD_RS | LCD_E);
    HAL_Delay(1);

    LCD_PORT->BSRR |= (data);
    HAL_Delay(1);
    // Set RS to 0 if sending command/instruction
    if (mode == W_CMD)
        LCD_PORT->BRR |= (LCD_RS);
    // Otherwise set to 1 for data write
    else 
        LCD_PORT->BSRR |= (LCD_RS);
    HAL_Delay(1);
    // Write opperation
    LCD_PORT->BRR |= (LCD_RW);
    
    HAL_Delay(1);

    // Set enable pin high
    LCD_PORT->BSRR |= (LCD_E);
    HAL_Delay(1);
    // Reset
    LCD_PORT->BRR |= (LCD_E);
    return;
}

void LCD_clear() {
    LCD_write(LCD_CMD_CLEAR, W_CMD);
    LCD_write(LCD_CMD_HOME, W_CMD);
}