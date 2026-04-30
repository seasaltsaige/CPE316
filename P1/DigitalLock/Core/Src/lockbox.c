#include "lockbox.h"
#include "keypad.h"
#include "lcd.h"
#include "stm32l4xx_hal.h"
#include "stm32l4xx_hal_flash.h"
#include <stdint.h>

static char passcode[LCD_COLS] = {0};
static uint8_t user_entry_pos = 0;
static char user_entry[LCD_COLS] = {0};
static uint8_t is_locked = 1;
uint8_t PASSCODE_LEN = 8;

void LOCKBOX_GPIO_Init() {
  LOCKBOX_PORT->MODER &= ~(GPIO_MODER_MODE5);
  LOCKBOX_PORT->MODER |= (GPIO_MODER_MODE5_0);
  LOCKBOX_PORT->OTYPER &= ~(GPIO_OTYPER_OT5);
  LOCKBOX_PORT->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR5);
}

static uint8_t compare_passcodes() {
  // If the user has entered more or fewer characters, exit early
  if (user_entry_pos != PASSCODE_LEN)
    return 0;
  uint8_t correct = 1;
  for (uint8_t i = 0; i < PASSCODE_LEN; i++) {
    if (passcode[i] != user_entry[i]) {
      correct = 0;
      break;
    }
  }

  return correct;
}

static void reset_passcode() {
  for (uint8_t i = 0; i < LCD_COLS; i++)
    user_entry[i] = 0;

  LCD_backspace(user_entry_pos);
  user_entry_pos = 0;
}

static void lock() {
  LCD_write_string("Key to Unlock:", "");
  LOCKBOX_PORT->BSRR = LOCKBOX_PIN;
}

static void unlock() {
  LCD_write_string("Key to Lock:", "");
  LOCKBOX_PORT->BRR = LOCKBOX_PIN;
}

static void lockbox_incorrect_seq() {
  if (is_locked)
    LCD_write_string("Key to Unlock:", "WRONG PASSWORD");
  else
    LCD_write_string("Key to Lock:", "WRONG PASSWORD");

  HAL_Delay(500);
  LCD_backspace(14);
  HAL_Delay(500);

  if (is_locked)
    LCD_write_string("Key to Unlock:", "WRONG PASSWORD");
  else
    LCD_write_string("Key to Lock:", "WRONG PASSWORD");
  HAL_Delay(500);
  LCD_backspace(14);
}

void LOCKBOX_Init() {
  reset_passcode();
  is_locked = 1;
  LCD_startup();
  lock();
  char start_passcode[8] = {'3', '1', '1', 'A', 'B', 'C', '9', '5'};
  for (uint8_t i = 0; i < PASSCODE_LEN; i++) {
    passcode[i] = start_passcode[i];
  }
}

void LOCKBOX_read_input() {
  char key = read_keypad();
  if (key != KEYPAD_READ_ERR) {
    HAL_Delay(40);
    if (read_keypad() != key)
      return;

    // Passcode 'enter'
    if (key == '#') {

      if (is_locked) {
        // Normal passcode handling
        uint8_t compared = compare_passcodes();
        if (compared) {
          unlock();
          is_locked = 0;
          user_entry_pos = 0;
        } else {
          reset_passcode();
          lockbox_incorrect_seq();
        }
      } else {
        // if not locked
        if (user_entry_pos == 0) {
          // If no passcode has been entered, lockbox can be re-locked
          lock();
          is_locked = 1;
          
        } else {
          // Set new passcode and lock box
          // passcode = user_entry;
          for (int i = 0; i < user_entry_pos; i++) {
            passcode[i] = user_entry[i];
          }
          // Set new passcode length
          PASSCODE_LEN = user_entry_pos;
          lock();
          is_locked = 1;
          user_entry_pos = 0;
        }
      }
    } else if (key == '*')
      reset_passcode();
    // Regular char entry
    else {
      // hmmm maybe just write *...
      // maybe display passcode idk
      if (user_entry_pos == COL_PINS) {
        // Maybe do something? Display? idk
        // maybe just compare if another key is pressed?
        return;
      }
      LCD_write(key, W_DAT);
      user_entry[user_entry_pos] = key;
      user_entry_pos++;
    }
    // LCD_write(key, W_DAT);
    // Don't read multiple of the same key...
    while (read_keypad() == key) {
    };
  }
}