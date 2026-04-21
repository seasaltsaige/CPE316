#include "keypad.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

const char keypad_map[NUM_ROWS][NUM_COLS] = {
    { '1', '2', '3', 'A', },
    { '4', '5', '6', 'B', },
    { '7', '8', '9', 'C', },
    { '*', '0', '#', 'D', },
};

const uint16_t ROW_MAP[NUM_ROWS] = {GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2, GPIO_PIN_3};
const uint16_t COL_MAP[NUM_COLS] = {GPIO_PIN_4, GPIO_PIN_5, GPIO_PIN_6, GPIO_PIN_7};

void KEYPAD_GPIO_Init() {
    
  // LCD Configure
  KEYPAD_PORT->MODER &= ~(
    // Row pins (inputs/pulldown)
    GPIO_MODER_MODE0 | GPIO_MODER_MODE1 |
    GPIO_MODER_MODE2 | GPIO_MODER_MODE3 |
    // Col pins (outputs)
    GPIO_MODER_MODE4 | GPIO_MODER_MODE5 |
    GPIO_MODER_MODE6 | GPIO_MODER_MODE7
  );
  // Output mode for columns
  KEYPAD_PORT->MODER |= (
    GPIO_MODER_MODE4_0 | GPIO_MODER_MODE5_0 |
    GPIO_MODER_MODE6_0 | GPIO_MODER_MODE7_0
  );

  // Default output types
  KEYPAD_PORT->OTYPER &= ~(
    GPIO_OTYPER_OT4 | GPIO_OTYPER_OT5 |
    GPIO_OTYPER_OT6 | GPIO_OTYPER_OT7
  );
  KEYPAD_PORT->OSPEEDR &= ~(
    GPIO_OSPEEDER_OSPEEDR4 | GPIO_OSPEEDER_OSPEEDR5 |
    GPIO_OSPEEDER_OSPEEDR6 | GPIO_OSPEEDER_OSPEEDR7
  );

  // Pulldown for rows
  KEYPAD_PORT->PUPDR &= ~(
    GPIO_PUPDR_PUPD0 | GPIO_PUPDR_PUPD1 |
    GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3
  );
  KEYPAD_PORT->PUPDR |= (
    GPIO_PUPDR_PUPD0_1 | GPIO_PUPDR_PUPD1_1 |
    GPIO_PUPDR_PUPD2_1 | GPIO_PUPDR_PUPD3_1
  );
}

char read_keypad() {
    char key = KEYPAD_READ_ERR;

    for (uint8_t col = 0; col < NUM_COLS; col++) {

        // Reset all column outputs
        KEYPAD_PORT->BRR = COL_PINS;
        // Set current column high
        KEYPAD_PORT->BSRR = COL_MAP[col];

        HAL_Delay(1);

        for (uint8_t row = 0; row < NUM_ROWS; row++) {
            if (KEYPAD_PORT->IDR & ROW_MAP[row])
                return keypad_map[row][col];
        }
    }
    
    return key;
}