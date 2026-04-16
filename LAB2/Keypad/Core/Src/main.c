#include "main.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_gpio.h"
#include <stdint.h>

#define ERR_KEYPAD -1

#define NUM_COLS 3
#define NUM_ROWS 4

const uint16_t COL_PINS[NUM_COLS] = { GPIO_PIN_3, GPIO_PIN_4, GPIO_PIN_5 };
#define KEYPAD_COLS (GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5)

const uint16_t ROW_PINS[NUM_ROWS] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_9, GPIO_PIN_10 };
#define KEYPAD_ROWS (GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_9 | GPIO_PIN_10)


#define LED_PIN_MASK (GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8)

// Note: [3][0] = '*' (display 14)
// [3][2] = '#' (display 15)
const uint16_t keypad_map[NUM_ROWS][NUM_COLS] = {
  {1, 2, 3},
  {4, 5, 6},
  {7, 8, 9},
  {14, 0, 15}
};

void SystemClock_Config(void);
int16_t read_keypad();
void GPIO_Init();

int main(void) {
  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();
  /* Configure the system clock */
  SystemClock_Config();
  RCC->AHB2ENR |= (RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOAEN);

  GPIO_Init();

  while (1) {
    // Initial keypad read
    int16_t keypad_value = read_keypad();
    // If it isn't an error, continue
    if (keypad_value != ERR_KEYPAD) {
      // 50ms debounce delay
      HAL_Delay(50);
      int16_t debounce_read = read_keypad();
      // If the input has changed, debounce has failed (input differs)
      if (keypad_value != debounce_read) continue;

      // Valid press
      // Shift output into correct position in ODR
      // (bits 5 to 8, not 0 to 3)
      int16_t out = keypad_value << 5;
      GPIOA->ODR &= ~LED_PIN_MASK;
      GPIOA->ODR |= out;
    }

    HAL_Delay(10);
  }
}

void GPIO_Init() {
  // ROWS  input w/ pull-downs
  // PA0, PA1, PA9, PA10
  // Tried using PA2 and PA3,
  // but it seems to cause issues with the uart TX/RX
  // Found why this happened, don't super want to change back even though
  // its not that hard to...
  // It is specifically an issue with STLink, since they use PA2 and PA3
  // even if not configured. They are reserved with STLink
  GPIOA->MODER &= ~(
                    GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | 
                    GPIO_MODER_MODE9 | GPIO_MODER_MODE10 | 
                    GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | 
                    GPIO_MODER_MODE7 | GPIO_MODER_MODE8
                  );
  // Reset PUPDR
  GPIOA->PUPDR &= ~(
                    GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 | 
                    GPIO_PUPDR_PUPD9 | GPIO_PUPDR_PUPD10 | 
                    GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | 
                    GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD8
                  );
  // Set Rows to Pull-down (pushing button sends row high --- cols as inputs)
  GPIOA->PUPDR |= (
                    GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD1_1 | 
                    GPIO_PUPDR_PUPD9_1 | GPIO_PUPDR_PUPD10_1
                  );

  // COLS  output HIGH
  // PB3, PB4, PB5
  GPIOB->MODER &= ~(GPIO_MODER_MODE3 | GPIO_MODER_MODE4 | GPIO_MODER_MODE5);
  GPIOB->MODER |= (GPIO_MODER_MODE3_0 | GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0);
  GPIOB->OTYPER &= (GPIO_OTYPER_OT3 | GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5);
  // set LOW in known state
  GPIOB->ODR &= ~KEYPAD_COLS;


  // Output LEDs
  // PA5, PA6, PA7, PA8
  GPIOA->MODER |= (GPIO_MODER_MODE5_0 | GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0 | GPIO_MODER_MODE8_0);
  GPIOA->ODR &= ~LED_PIN_MASK;
}

int16_t read_keypad() {

  for (uint8_t col = 0; col < NUM_COLS; col++) {
    // Drive current column high, all other columns low
    GPIOB->ODR &= ~KEYPAD_COLS;
    GPIOB->ODR |= COL_PINS[col];
    HAL_Delay(1);
    
    // Check each row to see if specific button is pressed
    for (uint8_t row = 0; row < NUM_ROWS; row++) {
      if (GPIOA->IDR & ROW_PINS[row]) {
        return keypad_map[row][col];
      }
    }
  }

  // Otherwise return unknown value (-1)
  return ERR_KEYPAD;
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

void assert_failed(uint8_t *file, uint32_t line) {
}
#endif
