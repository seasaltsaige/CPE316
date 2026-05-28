/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
#include "main.h"
#include "cmsis_gcc.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_pwr.h"

void SystemClock_Config(void);
void NormalMode();
void SleepMode();
void Stop2_RTC_Config();
void Stop2Mode();

void BusyWork();


void RTC_WKUP_IRQHandler(void) {
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;
  RTC->ISR &= ~(RTC_ISR_WUTF);
  RTC->WPR = 0xFF;
  EXTI->PR1 |= EXTI_PR1_PIF20;
}

// Used to wake up sleep mode every 5 seconds
void TIM2_IRQHandler() {
  TIM2->SR &= ~TIM_SR_UIF;
}

// SLEEP_TYPE = 0 | Normal
// SLEEP_TYPE = 1 | Sleep Mode
// SLEEP_TYPE = 2 | Stop 2
#define SLEEP_TYPE 2

int main(void)
{
  // Default setup
  HAL_Init();
  SystemClock_Config();

  // Ensure peripheral clocks are disabled for testing
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;

  if (SLEEP_TYPE == 0) {
    NormalMode();
  } else if (SLEEP_TYPE == 1) {
    SleepMode();
  } else if (SLEEP_TYPE == 2) {
    Stop2Mode();

  }

}

void NormalMode() {
  while (1) { BusyWork(); }
}

// Wake every 5 seconds using tim2 as an interrupt generator
void SleepMode() {
  TIM2->PSC = 4;
  TIM2->ARR = (32000000) - 1;

  NVIC->ISER[0] |= (1 << TIM2_IRQn);

  TIM2->DIER |= TIM_DIER_UIE;
  TIM2->CR1 |= TIM_CR1_CEN;
  TIM2->SR &= ~(TIM_SR_UIF);

  __enable_irq();

  while (1) {    
    SysTick->CTRL &= ~(SysTick_CTRL_ENABLE_Msk); // disable systick so light sleep isnt woken by tick
    __WFI();
    SysTick->CTRL |= (SysTick_CTRL_ENABLE_Msk); // enable systick so light sleep isnt woken by tick
    BusyWork();
    HAL_Delay(500);
  }
}

void Stop2_RTC_Config() {
  // Unlock backup domain
  RCC->APB1ENR1 |= RCC_APB1ENR1_PWREN;
  PWR->CR1 &= ~(PWR_CR1_LPR);
  PWR->CR1 |= PWR_CR1_DBP;
  while (!(PWR->CR1 & PWR_CR1_DBP)) {};

  // enable LSI 32khz
  RCC->CSR |= RCC_CSR_LSION;
  // Wait for stable
  while (!(RCC->CSR & RCC_CSR_LSIRDY)) {};

  // Select LSI as rtc clk
  RCC->BDCR &= ~(RCC_BDCR_RTCSEL);
  RCC->BDCR |= (RCC_BDCR_RTCSEL_1 | RCC_BDCR_RTCEN);


  // RTC IS USED To WAKE EVERY 5 SECONDS FROM DEEP SLEEP
  // write protection key sequence
  // stm32l47xxx ref manual page 1232 (section 38.3.7)
  // two byte sequence
  RTC->WPR = 0xCA;
  RTC->WPR = 0x53;

  RTC->CR &= ~(RTC_CR_WUTE);
  while (!(RTC->ISR & RTC_ISR_WUTWF)) {};

  // // Set wakeup clk sel to ck_spre
  RTC->CR &= ~(RTC_CR_WUCKSEL);
  RTC->CR |= (RTC_CR_WUCKSEL_2);

  // // Set wake timer to 5 seconds (based on 1hz clk sel above)
  RTC->WUTR = 4; // (WUTR + 1)

  // // re-enable wakeup timer + irq
  RTC->CR |= (RTC_CR_WUTE | RTC_CR_WUTIE);

  // // write to WPR; re-enable write protection
  RTC->WPR = 0xFF;

  // bit 20 for rtc wakeup timer
  // rising edge wakeup
  EXTI->IMR1 |= EXTI_IMR1_IM20;
  EXTI->RTSR1 |= EXTI_RTSR1_RT20;

  // set priority and enable in nvic
  NVIC->IP[RTC_WKUP_IRQn] = 0x00;
  NVIC->ISER[0] |= (1 << RTC_WKUP_IRQn);

  __enable_irq();
}

void Stop2Mode() {
  PWR->CR1 &= ~(PWR_CR1_LPMS);
  PWR->CR1 |= PWR_CR1_LPMS_STOP2;

  SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
  Stop2_RTC_Config();

  while (1) {
    BusyWork();
    PWR->SCR |= PWR_SCR_CWUF;
    __WFI();
  }
}


void BusyWork(void)
{
  volatile uint32_t i;
  volatile uint64_t result = 0;
  for (i = 0; i < 100000; i++)
  {
    result += i * 3;
  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = 1;
  RCC_OscInitStruct.PLL.PLLN = 16;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
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
