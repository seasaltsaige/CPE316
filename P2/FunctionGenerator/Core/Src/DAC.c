#include <stdint.h>
#include <stm32l4xx_hal.h>
#include "DAC.h"
#include "stm32l476xx.h"



void DAC_init() {
    // Clear pin 4, 5, 6, and 7 
    GPIOA->AFR[0] &= ~(GPIO_AFRL_AFSEL4 | GPIO_AFRL_AFSEL5 | GPIO_AFRL_AFSEL6 | GPIO_AFRL_AFSEL7);
    // 
    GPIOA->AFR[0] |= ((GPIO_AFRL_AFSEL4_2 | GPIO_AFRL_AFSEL4_0) | 
                      (GPIO_AFRL_AFSEL5_2 | GPIO_AFRL_AFSEL5_0) | 
                      (GPIO_AFRL_AFSEL6_2 | GPIO_AFRL_AFSEL6_0) | 
                      (GPIO_AFRL_AFSEL7_2 | GPIO_AFRL_AFSEL7_0));

    // Set A4-7 to alternate function mode
    GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOA->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);

    // 'master' () mode, divide clock by 4 (80Mhz / 4 = 20Mhz)
    SPI1->CR1 = (SPI_CR1_MSTR | SPI_CR1_BR_0);
    
    // 16 bit mode, hardware chip sel
    SPI1->CR2 = ((SPI_CR2_DS_3 | SPI_CR2_DS_2 | SPI_CR2_DS_1 | SPI_CR2_DS_0) | SPI_CR2_NSSP | SPI_CR2_SSOE);

    // enable spi peripheral
    SPI1->CR1 |= (SPI_CR1_SPE);
}


void DAC_write(uint16_t value) {
    if (value >= DAC_MAX) value = (DAC_MAX - 1);

    uint16_t output = DAC_HEADER | value;
    // Don't write to data register while spi tx is busy
    while (!(SPI1->SR & SPI_SR_TXE)) {};
    SPI1->DR = output; // output spi
}


uint16_t DAC_volt_conv(uint16_t voltage) {
    if (voltage > VOLT_MAX) voltage = VOLT_MAX;
    return (voltage * DAC_MAX) / VOLT_MAX;
}