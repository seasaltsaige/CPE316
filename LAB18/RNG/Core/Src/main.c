#include "main.h"
#include "lcd.h"
#include "stm32l476xx.h"

void SystemClock_Config(void);

volatile uint32_t ran_number = 0;
char data[10] = {0};

void parse_int_to_data(void)
{
    for (uint8_t x = 0; x < 10; x++)
        data[x] = ' ';
    // data[10] = '\0';

    uint32_t n = ran_number;
    for (int8_t i = 9; i >= 0 && n > 0; i--)
    {
        data[9-i] = (n % 10) + '0';
        n /= 10;
    }
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  
  RCC->CR &= ~(RCC_CR_MSIRANGE); // clear range
  RCC->CR |= (RCC_CR_MSIRANGE_11 | RCC_CR_MSIRGSEL | RCC_CR_MSION); // set range 11

  while (!(RCC->CR & RCC_CR_MSIRDY));

  RCC->CCIPR &= ~(RCC_CCIPR_CLK48SEL); // route MSU to CLK48 (3)
  RCC->CCIPR |= (RCC_CCIPR_CLK48SEL_0 | RCC_CCIPR_CLK48SEL_1);

  RNG->CR |= RNG_CR_RNGEN;

  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_RNGEN);

  GPIOA->MODER &= ~(GPIO_MODER_MODE5);
  GPIOA->MODER |= (GPIO_MODER_MODE5_0);

  LCD_GPIO_Init();
  LCD_startup();

  RNG->CR |= RNG_CR_RNGEN;

  while (1) {
    // Handle seed error
    if (RNG->SR & RNG_SR_SECS)
    {
      RNG->SR &= ~RNG_SR_SEIS;
      RNG->CR &= ~RNG_CR_RNGEN;
      RNG->CR |=  RNG_CR_RNGEN;
      continue;
    }
    
    // wait for ready flag
    while (!(RNG->SR & RNG_SR_DRDY));

    // set random number, parse, and display on LCD
    ran_number = RNG->DR;
    
    parse_int_to_data();
    LCD_write_string(data, "");
    HAL_Delay(1500);
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 2;
  RCC_OscInitStruct.PLL.PLLN = 24;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV7;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV4;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
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
