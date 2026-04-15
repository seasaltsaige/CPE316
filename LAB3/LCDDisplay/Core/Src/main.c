#include "main.h"
#include "lcd.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <string.h>

void SystemClock_Config(void);
void GPIO_Init();
void write_presses();

static uint8_t press_counter = 0;

int main(void)
{
  SystemClock_Config();
  HAL_Init();

  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN);
  // Init GPIO pins
  GPIO_Init();
  // LCD Startup sequence as defined by datasheet
  LCD_startup();

  char *top_string = "Hello, world!";
  char *bottom_string = "Assignment 3";
  LCD_write_string(top_string, bottom_string);
  
  for (int i = 0; i < 4; i++)
    LCD_write(' ', W_DAT);

  write_presses();
  // Monitor reset button
  for (;;) {
    if (GPIOB->IDR & CNT_BTN) {
      HAL_Delay(50);
      if (GPIOB->IDR & CNT_BTN) {
        // Valid button press
        press_counter++;
        write_presses();
        HAL_Delay(10);
      }
    }
  }
}

void write_presses() {
  LCD_backspace(3);
  uint8_t curr_cnt = press_counter;

  uint8_t ones = curr_cnt % 10;
  curr_cnt /= 10;
  uint8_t tens = curr_cnt % 10;
  curr_cnt /= 10;
  uint8_t hund = curr_cnt % 10;
  curr_cnt /= 10;

  LCD_write(hund != 0 ? hund + 48 : ' ', W_DAT);
  LCD_write(tens != 0 || hund != 0 ? tens + 48 : ' ', W_DAT);
  LCD_write(ones + 48, W_DAT);  
}

void GPIO_Init() {
  // Configure data pins as outputs
  // PB0-7 correspond to D0-7
  // PB8 = RS, PB9 = RW, PB10 = E
  // PB11 = reset button, pulldown
  LCD_PORT->MODER &= ~(
    GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | 
    GPIO_MODER_MODE2 | GPIO_MODER_MODE3 | 
    GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | 
    GPIO_MODER_MODE6 | GPIO_MODER_MODE7 | 
    GPIO_MODER_MODE8 | GPIO_MODER_MODE9 | 
    GPIO_MODER_MODE10 | GPIO_MODER_MODE11
  );

  // PB0-10 = output, PB11 = input
  LCD_PORT->MODER |= (
    GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0 | 
    GPIO_MODER_MODE2_0 | GPIO_MODER_MODE3_0 | 
    GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 | 
    GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0 | 
    GPIO_MODER_MODE8_0 | GPIO_MODER_MODE9_0 | 
    GPIO_MODER_MODE10_0
  );

  // Pull down on PB11
  LCD_PORT->PUPDR &= ~GPIO_PUPDR_PUPD11;
  LCD_PORT->PUPDR |= GPIO_PUPDR_PUPD11_1;


  // Push pull
  LCD_PORT->OTYPER &= ~(
    GPIO_OTYPER_OT0 | GPIO_OTYPER_OT1 | 
    GPIO_OTYPER_OT2 | GPIO_OTYPER_OT3 | 
    GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5 | 
    GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7 | 
    GPIO_OTYPER_OT8 | GPIO_OTYPER_OT9 | 
    GPIO_OTYPER_OT10
  );

  // Default speed
  LCD_PORT->OSPEEDR &= ~(
    GPIO_OSPEEDER_OSPEEDR0 | GPIO_OSPEEDER_OSPEEDR1 | 
    GPIO_OSPEEDER_OSPEEDR2 | GPIO_OSPEEDER_OSPEEDR3 | 
    GPIO_OSPEEDER_OSPEEDR4 | GPIO_OSPEEDER_OSPEEDR5 | 
    GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7 | 
    GPIO_OSPEEDER_OSPEEDR8 | GPIO_OSPEEDER_OSPEEDR9 | 
    GPIO_OSPEEDER_OSPEEDR10
  );

  // Set all output pins low to start
  LCD_PORT->BRR |= (LCD_DATA_PIN_MASK | LCD_RS | LCD_RW | LCD_E);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    // Error_Handler();
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
    // Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    // Error_Handler();
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
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
