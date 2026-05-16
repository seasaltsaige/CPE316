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
#include "stewart_controller.h"
#include "stm32l476xx.h"
#include <stdint.h>

void SystemClock_Config(void);

void TIM6_DAC_IRQHandler() {
  if (TIM6->SR & TIM_SR_UIF) {
    TIM6->SR &= ~(TIM_SR_UIF);
    for (int i = 0; i < 6; i++) {
      // if the motor is either not running
      // homing, or extending (finding max extension)
      // we don't need to accelerate
      if (!motors[i].running || motors[i].homing || motors[i].extending) continue;
      stepper_accel(&motors[i]);
    }
  }
}

void TIM1_UP_TIM16_IRQHandler() {
  if (TIM1->SR & TIM_SR_UIF) {
    TIM1->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[0]);
  }
}


// GPIO interrupt vectors

// TODO: might want to mask interrupt on hit, then unmask after some debounce
void EXTI1_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF1) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF1);
    // HOME_PIN_C1 LIMIT SWITCH ACTIVE

    // Do stuff
  }
}

void EXTI2_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF2) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF2);
    // HOME_PIN_C2 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}

void EXTI3_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF3) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF3);
    // LIMIT_PIN_C2 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}

void EXTI4_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF4) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF4);
    // HOME_PIN_A1 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}

void EXTI9_5_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF5) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF5);
    // LIMIT_PIN_A1 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF6) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF6);
    // HOME_PIN_A2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF7) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF7);
    // LIMIT_PIN_A2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF9) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF9);
    // LIMIT_PIN_C1 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}

void EXTI15_10_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF10) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF10);
    // HOME_PIN_B2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF11) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF11);
    // LIMIT_PIN_B2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF14) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF14);
    // HOME_PIN_B1 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF15) {
    EXTI->PR1 &= ~(EXTI_PR1_PIF15);
    // LIMIT_PIN_B1 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}
// void TIM2_IRQHandler() {

// }

// void TIM3_IRQHandler() {

// }

// void TIM4_IRQHandler() {

// }

// void TIM5_IRQHandler() {

// }

// void TIM8_IRQHandler() {

// }

int main(void)
{

  HAL_Init();
  SystemClock_Config();

  STEWART_init();

  int32_t test_dist_mm = -300;
  int32_t test_dist_steps = (test_dist_mm * STEPS_PER_REV) / MM_PER_REV;

  stepper_move_const_vel(&motors[0], test_dist_steps, 20);
  // stepper_move(&motors[0], test_dist_steps, MOVE_TIME_MS);
  

  while (1)
  {
    
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
