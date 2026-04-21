#ifndef LOCKBOX_H
#define LOCKBOX_H

#include "stm32l4xx_hal.h"

#define LOCKBOX_PORT GPIOA
#define LOCKBOX_PIN (GPIO_PIN_5)

#define PASSCODE_LEN 8

void LOCKBOX_GPIO_Init();
void LOCKBOX_Init();
void LOCKBOX_read_input();

#endif