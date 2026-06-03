#include "main.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

void SystemClock_Config(void);
int main(void)
{
  HAL_Init();
  SystemClock_Config();

  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN);
  RCC->APB2ENR |= (RCC_APB2ENR_SYSCFGEN);

  // PB2 = analog, PA5 = output
  GPIOA->MODER &= ~(GPIO_MODER_MODE5);
  GPIOA->MODER |= (GPIO_MODER_MODE5_0);

  GPIOB->MODER &= ~(GPIO_MODER_MODE2);
  GPIOB->MODER |= (GPIO_MODER_MODE2_0 | GPIO_MODER_MODE2_1);

  // enable adc vref
  ADC123_COMMON->CCR |= (ADC_CCR_VREFEN);
  // wait for it to turn on, probably WAY overkill time wise
  // but also 5ms is still like nothing to a human
  HAL_Delay(5);

  // inmsel = 010 = 3/4 vref, PB2, inverted (below 3/4), enable dividers (vref), enable
  COMP1->CSR |= (COMP_CSR_INMSEL_1 | COMP_CSR_INPSEL | COMP_CSR_POLARITY | COMP_CSR_BRGEN | COMP_CSR_SCALEN | COMP_CSR_EN); 
  // inmsel = 001 = 1/2 vref, non-inverted (above 1/2), window mode w/ comp1, enable dividers (vref), enable
  COMP2->CSR |= (COMP_CSR_INMSEL_0 | COMP_CSR_WINMODE | COMP_CSR_BRGEN | COMP_CSR_SCALEN | COMP_CSR_EN); 
  while (1)
  {
    // if in window, enable gpio PA5
    if ((COMP1->CSR & COMP_CSR_VALUE) && (COMP2->CSR & COMP_CSR_VALUE))
      GPIOA->ODR |= (GPIO_PIN_5);
    // otherwise turn it off
    else 
      GPIOA->ODR &= ~(GPIO_PIN_5);

  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
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
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif
