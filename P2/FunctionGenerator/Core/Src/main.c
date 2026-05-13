#include "main.h"
#include "func_gen.h"
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
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOCEN);
  RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
  RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN;
  DAC_init();
  TIM_init();
  KEYPAD_GPIO_Init();

  while (1) {
    char read = read_keypad();
    HAL_Delay(10);
    if (read == read_keypad()) {

      // -- begin frequency handling --
      if (read == '1') set_freq(ONE);
      else if (read == '2') set_freq(TWO);
      else if (read == '3') set_freq(THREE);
      else if (read == '4') set_freq(FOUR);
      else if (read == '5') set_freq(FIVE);
      // -- end frequency handling --

      // -- begin wave type handling --
      else if (read == '6') set_wave(SIN);
      else if (read == '7') set_wave(TRIANGLE);
      else if (read == '8') set_wave(SAW);
      else if (read == '9') set_wave(SQUARE);
      // -- end wave type handling --

      // -- begin square wave duty cycle handling --
      else if (read == '0') set_duty(FIFTY);
      else if (read == '*' && duty_cycle != TEN) set_duty((SQUARE_DUTY)(duty_cycle - 10));
      else if (read == '#' && duty_cycle != NINETY) set_duty((SQUARE_DUTY)(duty_cycle + 10));
      // -- end square wave duty cycle handling --

      while (read_keypad() == read) {} // dont continue until button is released
    }
  }
}


void TIM_init() {
  __enable_irq();
  NVIC->ISER[0] = (1 << (TIM2_IRQn));
  TIM2->SR &= ~(TIM_SR_CC1IF | TIM_SR_UIF);
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
