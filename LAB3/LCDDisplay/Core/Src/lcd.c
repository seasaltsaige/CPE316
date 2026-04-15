#include "lcd.h"
#include "stm32l4xx_hal.h"
#include <stddef.h>
#include <stdint.h>


uint8_t row_offsets[4] = {
    0x00,
    0x40,
    0x10,
    0x50
};


void LCD_startup() {
    
    // Wait for more than 30ms on init
    HAL_Delay(100);

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

void LCD_write_string(char* data, uint8_t length) {
    LCD_write(LCD_CMD_CLEAR, W_CMD);
    LCD_write(LCD_CMD_HOME, W_CMD);

    uint8_t i;
    // Write top half
    for (i = 0; i < (LCD_MAX_CHARS / 2) ; i++) {
        if (i == length) return;
        // If space, move right instead of write
        // if (data[i] == ' ')
        //     LCD_write(0x14, W_CMD);
        // else if (data == '\0')
        //     continue;
        // else
        
        if (data[i] <= 2 || data[i] >= 0xFF) continue;
            LCD_write(data[i], W_DAT);

        HAL_Delay(TYPING_SPEED);
    }

    uint8_t cols = 16;
    uint8_t rows = 2;
    uint8_t move_cursor = LCD_DDRAM_SET | (0x40);
    LCD_write(move_cursor, W_CMD);
    
    for (; i < LCD_MAX_CHARS; i++) {
        if (i == length) return;
        // If space, move right instead of write
        // if (data[i] == ' ')
        //     LCD_write(0x14, W_CMD);
        // else if (data == '\0')
        //     continue;
        if (data[i] <= 2 || data[i] >= 0xFF) continue;
        // else
            LCD_write(data[i], W_DAT);
        
        HAL_Delay(TYPING_SPEED);
    }

    return;
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