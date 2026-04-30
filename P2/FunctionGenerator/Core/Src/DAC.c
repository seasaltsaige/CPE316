#include <stm32l4xx_hal.h>
#include "DAC.h"
#include "stm32l476xx.h"


void DAC_init() {
    GPIOA->AFR[0] &= ;
    GPIOA->AFR[0] |= ;
    GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7);
    GPIOA->MODER |= (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE5_1 | GPIO_MODER_MODE6_1 | GPIO_MODER_MODE7_1);

    // SPI1->CR1 = ;
    // SPI1->CR2 = ;
    
}


void DAC_write(uint16_t voltage) {

}