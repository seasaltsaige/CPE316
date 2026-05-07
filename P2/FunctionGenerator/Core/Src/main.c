#include "main.h"
#include "keypad.h"
#include "stm32l476xx.h"
#include "DAC.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
void SystemClock_Config(void);
void TIM_init();

uint16_t max = 1000;

void TIM2_IRQHandler(void) {

  if (TIM2->SR & TIM_SR_CC1IF) {
    TIM2->SR &= ~(TIM_SR_CC1IF);

    // 1V low
    DAC_write(DAC_volt_conv(0));
  }
  
  if (TIM2->SR & TIM_SR_UIF) {
    TIM2->SR &= ~(TIM_SR_UIF);
    // 2V high
    DAC_write(DAC_volt_conv(max));
  }
}


int main(void) {
  HAL_Init();
  SystemClock_Config();

  // enable spi clock, gpio A, C, and TIM2 clocks 
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN);
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  DAC_init();
  TIM_init();
  KEYPAD_GPIO_Init();

  while (1) {
    char read = read_keypad();
    HAL_Delay(10);
    if (read == read_keypad()) {

      if (read == '1') {
        max = 1000;
      } else if (read == '2') {
        max = 2000;
      } else if (read == '3') {
        max = 3000;
      }

    }
  }
}


void TIM_init() {
  __enable_irq();

  TIM2->ARR = 80000 - 1; // arr will fire every 1000 cycles
  TIM2->CCR1 = 20000 - 1; // 25% duty cycle (ccr status sets low, arr sets high)

  NVIC->ISER[0] = (1 << (TIM2_IRQn & 0x1F));

  TIM2->SR &= ~(TIM_SR_CC1IF);
  TIM2->DIER |= (TIM_DIER_UIE | TIM_DIER_CC1IE); // enable
  TIM2->CR1 |= TIM_CR1_CEN;
}

void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK) {
    Error_Handler();
  }

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 10;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK) {
    Error_Handler();
  }
}

void Error_Handler(void) {
  __disable_irq();
  while (1) {
  }
}
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */
