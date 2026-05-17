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

      // Handle logic for stalls in certain states to allow for the actuators to chill out after stopping
      if (motors[i].motor_state != IDLE && motors[i].motor_state != NORMAL_RUNNING) {
        // if the motor is specifically in any of these states
        if (motors[i].motor_state == HOMING_FAST_LIMIT || motors[i].motor_state == HOMING_FAST_BACKOFF_DONE || 
            motors[i].motor_state == HOMING_SLOW_LIMIT || motors[i].motor_state == HOMING_SLOW_BACKOFF_DONE ||

            motors[i].motor_state == EXTENSION_FAST_LIMIT || motors[i].motor_state == EXTENSION_FAST_BACKOFF_DONE || 
            motors[i].motor_state == EXTENSION_SLOW_LIMIT || motors[i].motor_state == EXTENSION_SLOW_BACKOFF_DONE) {
          // tick time
          motors[i].ticks_ms++;
          
          // if time has passed defined holding period
          if (motors[i].ticks_ms == HOMING_LIMIT_HOLD_MS) {
            // reset counter
            motors[i].ticks_ms = 0;

            // Go to next state accordingly
            if (motors[i].motor_state == HOMING_FAST_LIMIT)
              motors[i].motor_state = HOMING_FAST_LIMIT_TRANSITION;
            else if (motors[i].motor_state == HOMING_FAST_BACKOFF_DONE)
              motors[i].motor_state = HOMING_FAST_BACKOFF_TRANSITION;
            else if (motors[i].motor_state == HOMING_SLOW_LIMIT)
              motors[i].motor_state = HOMING_SLOW_LIMIT_TRANSITION;
            else if (motors[i].motor_state == HOMING_SLOW_BACKOFF_DONE)
              motors[i].motor_state = HOMING_SLOW_BACKOFF_TRANSITION;

            else if (motors[i].motor_state == EXTENSION_FAST_LIMIT)
              motors[i].motor_state = EXTENSION_FAST_LIMIT_TRANSITION;
            else if (motors[i].motor_state == EXTENSION_FAST_BACKOFF_DONE)
              motors[i].motor_state = EXTENSION_FAST_BACKOFF_TRANSITION;
            else if (motors[i].motor_state == EXTENSION_SLOW_LIMIT)
              motors[i].motor_state = EXTENSION_SLOW_LIMIT_TRANSITION;
            else if (motors[i].motor_state == EXTENSION_SLOW_BACKOFF_DONE)
              motors[i].motor_state = EXTENSION_SLOW_BACKOFF_TRANSITION;

            

            // re-enable homing EXTI flag 
            EXTI->IMR1 |= (motors[i].EXTI_home_flag);
            EXTI->IMR1 |= (motors[i].EXTI_limit_flag);

          }
        }
        continue;
      }

      if (motors[i].motor_state == IDLE || motors[i].motor_state != NORMAL_RUNNING) continue; 
      stepper_accel(&motors[i]);
    }
  }
}

// Stepper tick timer for LEG A, motor 1  
void TIM1_UP_TIM16_IRQHandler() {
  if (TIM1->SR & TIM_SR_UIF) {
    TIM1->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[0]);
  }
}

// Stepper tick timer for LEG A, motor 2
void TIM2_IRQHandler() {
  if (TIM2->SR & TIM_SR_UIF) {
    TIM2->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[1]);
  }
}

void TIM3_IRQHandler() {

}

void TIM4_IRQHandler() {

}

void TIM5_IRQHandler() {

}

void TIM8_IRQHandler() {

}


// GPIO interrupt vectors

// TODO: might want to mask interrupt on hit, then unmask after some debounce
void EXTI1_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF1) {
    EXTI->PR1 |= (EXTI_PR1_PIF1);
    // HOME_PIN_C1 LIMIT SWITCH ACTIVE

    // Do stuff
  }
}

void EXTI2_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF2) {
    EXTI->PR1 |= (EXTI_PR1_PIF2);
    // HOME_PIN_C2 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}

void EXTI3_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF3) {
    EXTI->PR1 |= (EXTI_PR1_PIF3);
    // LIMIT_PIN_C2 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}


// HOMING side
void EXTI4_IRQHandler() {

  if (EXTI->PR1 & EXTI_PR1_PIF4) {
    // Mask interrupt so end stop bounce doesn't cause weirdness
    EXTI->IMR1 &= ~(EXTI_IMR1_IM4);
    // Clear pending flag FIRST
    EXTI->PR1 = EXTI_PR1_PIF4;


    switch (motors[0].motor_state) {
      case HOMING_FAST:
        *(motors[0].CCR) = 0;
        motors[0].timer->CR1 &= ~TIM_CR1_CEN;
        motors[0].timer->CNT = 0;
        motors[0].motor_state = HOMING_FAST_LIMIT;
        motors[0].steps_current = 0;
        break;

      case HOMING_SLOW:
        *(motors[0].CCR) = 0;
        motors[0].timer->CR1 &= ~TIM_CR1_CEN;
        motors[0].timer->CNT = 0;
        motors[0].motor_state = HOMING_SLOW_LIMIT;
        motors[0].steps_current = 0;
        break;

      case HOMING_SLOW_BACKOFF:
        // TODO: ZERO POSITION
        *(motors[0].CCR) = 0;
        motors[0].timer->CR1 &= ~TIM_CR1_CEN;
        motors[0].timer->CNT = 0;
        motors[0].motor_state = HOMING_SLOW_BACKOFF_DONE;
        motors[0].steps_current = 0;
        break;

      default:
        // If none of the above cases happened, re-enable the interrupt
        EXTI->IMR1 |= (EXTI_IMR1_IM4);
        break;
    }

  }
}

void EXTI9_5_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF5) {
    // Mask interrupt so end stop bounce doesn't cause weirdness
    EXTI->IMR1 &= ~(EXTI_IMR1_IM5);
    // // Clear pending flag FIRST
    EXTI->PR1 = EXTI_PR1_PIF5;

    switch (motors[0].motor_state) {
      case EXTENSION_FAST:
        *(motors[0].CCR) = 0;
        motors[0].timer->CR1 &= ~TIM_CR1_CEN;
        motors[0].timer->CNT = 0;
        motors[0].motor_state = EXTENSION_FAST_LIMIT;
        break;

      case EXTENSION_SLOW:
        *(motors[0].CCR) = 0;
        motors[0].timer->CR1 &= ~TIM_CR1_CEN;
        motors[0].timer->CNT = 0;
        motors[0].motor_state = EXTENSION_SLOW_LIMIT;
        break;

      case EXTENSION_SLOW_BACKOFF:
        // TODO: SET MAX EXTENSION HERE (MAX STEPS TO FULL EXTENSION)
        *(motors[0].CCR) = 0;
        motors[0].timer->CR1 &= ~TIM_CR1_CEN;
        motors[0].timer->CNT = 0;
        motors[0].motor_state = EXTENSION_SLOW_BACKOFF_DONE;
        motors[0].MAX_STEPS = motors[0].steps_current;
        break;

      default:
        // If none of the above cases happened, re-enable the interrupt
        EXTI->IMR1 |= (EXTI_IMR1_IM5);
        break;
    }
    
  }

  if (EXTI->PR1 & EXTI_PR1_PIF6) {
    EXTI->PR1 = (EXTI_PR1_PIF6);
    GPIOA->ODR ^= DIR_PIN_A2;
    // HOME_PIN_A2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF7) {
    EXTI->PR1 = (EXTI_PR1_PIF7);
    GPIOA->ODR ^= DIR_PIN_A2;
    // LIMIT_PIN_A2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF9) {
    EXTI->PR1 = (EXTI_PR1_PIF9);
    GPIOA->ODR ^= DIR_PIN_A2;
    // LIMIT_PIN_C1 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}

void EXTI15_10_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF10) {
    EXTI->PR1 = (EXTI_PR1_PIF10);
    GPIOA->ODR ^= DIR_PIN_A2;
    // HOME_PIN_B2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF11) {
    EXTI->PR1 = (EXTI_PR1_PIF11);
    GPIOA->ODR ^= DIR_PIN_A2;
    // LIMIT_PIN_B2 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF14) {
    EXTI->PR1 = (EXTI_PR1_PIF14);
    GPIOA->ODR ^= DIR_PIN_A2;
    // HOME_PIN_B1 LIMIT SWITCH ACTIVE
    // Do stuff
  }

  if (EXTI->PR1 & EXTI_PR1_PIF15) {
    EXTI->PR1 = (EXTI_PR1_PIF15);
    GPIOA->ODR ^= DIR_PIN_A2;
    // LIMIT_PIN_B1 LIMIT SWITCH ACTIVE
    // Do stuff
  }
}


int main(void)
{

  HAL_Init();
  SystemClock_Config();

  STEWART_init();

  home_platform();

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
  // __disable_irq();
  // while (1)
  // {
  // }
}
#ifdef USE_FULL_ASSERT

void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif 
