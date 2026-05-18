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
#include "helper.h"
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

// Stepper tick timer for LEG B, motor 1
void TIM3_IRQHandler() {
  if (TIM3->SR & TIM_SR_UIF) {
    TIM3->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[2]);
  }
}

// Stepper tick timer for LEG B, motor 2
void TIM4_IRQHandler() {
  if (TIM4->SR & TIM_SR_UIF) {
    TIM4->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[3]);
  }
}

// Stepper tick timer for LEG C, motor 1
void TIM5_IRQHandler() {
  if (TIM5->SR & TIM_SR_UIF) {
    TIM5->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[4]);
  }
}

// Stepper tick timer for LEG C, motor 2
void TIM8_IRQHandler() {
  if (TIM8->SR & TIM_SR_UIF) {
    TIM8->SR &= ~(TIM_SR_UIF);
    stepper_tick(&motors[5]);
  }
};

// GPIO interrupt vectors

// LEG C1 HOME END STOP
void EXTI1_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF1)
    handle_endstop(&motors[4], HOME_ENDSTOP, EXTI_PR1_PIF1); 
}

// LEG C2 HOME END STOP
void EXTI2_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF2)
    handle_endstop(&motors[5], HOME_ENDSTOP, EXTI_PR1_PIF2); 
}

// LEG C2 EXTENSION END STOP
void EXTI3_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF3)
    handle_endstop(&motors[5], EXTENSION_ENDSTOP, EXTI_PR1_PIF3); 
}


// LEG A1 HOME END STOP
void EXTI4_IRQHandler() {
  if (EXTI->PR1 & EXTI_PR1_PIF4)
    handle_endstop(&motors[0], HOME_ENDSTOP, EXTI_PR1_PIF4); 
}

void EXTI9_5_IRQHandler() {
  // LEG A1 EXTENSION END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF5) 
    handle_endstop(&motors[0], EXTENSION_ENDSTOP, EXTI_PR1_PIF5); 

  // LEG A2 HOME END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF6)
    handle_endstop(&motors[1], HOME_ENDSTOP, EXTI_PR1_PIF6); 


  // LEG A2 EXTENSION END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF7)
    handle_endstop(&motors[1], EXTENSION_ENDSTOP, EXTI_PR1_PIF7); 


  // LEG C1 EXTENSION END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF9)
    handle_endstop(&motors[4], EXTENSION_ENDSTOP, EXTI_PR1_PIF9); 

}

void EXTI15_10_IRQHandler() {
  
  // LEG B2 HOME END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF10)
    handle_endstop(&motors[3], HOME_ENDSTOP, EXTI_PR1_PIF10); 


  // LEG B2 EXTENSION END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF11)
    handle_endstop(&motors[3], EXTENSION_ENDSTOP, EXTI_PR1_PIF11); 

  // LEG B1 HOME END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF14)
    handle_endstop(&motors[2], HOME_ENDSTOP, EXTI_PR1_PIF14); 


  // LEG B1 EXTENSION END STOP
  if (EXTI->PR1 & EXTI_PR1_PIF15)
    handle_endstop(&motors[2], EXTENSION_ENDSTOP, EXTI_PR1_PIF15); 

}


int main(void)
{

  HAL_Init();
  SystemClock_Config();

  STEWART_init();
  // Blocking call until all legs have been homed and positioned in starting position
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
