#ifndef LCD_H
#define LCD_H

// #include "stm32l4xx_hal.h"

#include <stdint.h>

void LCD_startup();
void LCD_write(uint8_t data, uint8_t mode);

#endif