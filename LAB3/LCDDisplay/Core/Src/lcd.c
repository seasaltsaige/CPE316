#include "lcd.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

static uint16_t output_data;



void LCD_startup() {
    // Wakeup data
    output_data = 0x30;

    LCD_write(output_data, W_CMD);
    // Wait 100ms
    HAL_Delay(100);

    // Wait 1ms
    HAL_Delay(10);

    // Wait 1ms
    HAL_Delay(10);
}

void LCD_write(uint8_t data, uint8_t mode) {
    // Disable everything
    GPIOA->BRR |= (LCD_DATA_PIN_MASK | LCD_RW | LCD_RS | LCD_E);

    GPIOA->BSRR |= ();
}