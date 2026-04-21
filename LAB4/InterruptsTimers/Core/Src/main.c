#include "main.h"
#include "stm32l476xx.h"
void SystemClock_Config(void);
void GPIO_Init();
void TIMER_Init();

void TIM2_IRQHandler(void) {
  if (TIM2->SR & TIM_SR_CC1IF) {
    TIM2->SR &= ~(TIM_SR_CC1IF);
    GPIOB->BRR = GPIO_PIN_0;
  }
  
  if (TIM2->SR & TIM_SR_UIF) {
    TIM2->SR &= ~(TIM_SR_UIF);
    GPIOB->BSRR = GPIO_PIN_0;
  }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);
 
  GPIO_Init();
  TIMER_Init();
  
  for (;;)
  {
  }
}

void GPIO_Init() {
  GPIOB->MODER &= ~(GPIO_MODER_MODE0);
  GPIOB->MODER |= (GPIO_MODER_MODE0_0);
  GPIOB->OTYPER &= ~(GPIO_OTYPER_OT0);
  GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD0);
  GPIOB->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR0);
}

void TIMER_Init() {
  __enable_irq();
  NVIC->ISER[0] = (1 << (TIM2_IRQn & 0x1F));

  RCC->APB1ENR1 |= (RCC_APB1ENR1_TIM2EN);
  RCC->CFGR &= ~(RCC_CFGR_HPRE | RCC_CFGR_PPRE1 | RCC_CFGR_PPRE2);
  // 4 MHz clock
  // Want 5kHz 25% duty cycle square wave
  // 5kHz = 0.0002 period
  // 25% duty cycle implies HIGH for 0.25 * 0.0002, low for 0.75 * 0.0002
  // CCR1 and CCR2 can be used

  // 4,000,000 / 5,000 = 800 = 0x320 --- Full period of 5khz (ARR value)
  // System Clock was defaulted to 80MHz, not 4MHz, was getting a 10us period from this, but after
  // updating the MSI clock, getting the expected 200us period.

  TIM2->ARR = 0x320 - 1;
  TIM2->CCR1 = 0xC8 - 1;
  TIM2->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);
  TIM2->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);
  TIM2->CR1 |= (TIM_CR1_CEN);
}



void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    // Error_Handler();
  }

  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    // Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    // Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}