#include "lockbox.h"
#include "keypad.h"
#include "lcd.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

static const char passcode[PASSCODE_LEN] = {'3', '1', 'A', 'D', '9', '3', '1', '1'};
static uint8_t user_entry_pos = 0;
static char user_entry[LCD_COLS] = {0};
static uint8_t is_locked = 1;

void LOCKBOX_GPIO_Init() {
    LOCKBOX_PORT->MODER &= ~(GPIO_MODER_MODE5);
    LOCKBOX_PORT->MODER |= (GPIO_MODER_MODE5_0);
    LOCKBOX_PORT->OTYPER &= ~(GPIO_OTYPER_OT5);
    LOCKBOX_PORT->OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR5);
}


static uint8_t compare_passcodes() {
    // If the user has entered more or fewer characters, exit early
    if (user_entry_pos != PASSCODE_LEN) return 0;
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
}

void LOCKBOX_read_input() {
    char key = read_keypad();
    if (key != KEYPAD_READ_ERR) {
        HAL_Delay(40);
        if (read_keypad() != key) return;

        // Passcode 'enter'
        if (key == '#') {
            uint8_t compared = compare_passcodes();
            if (compared) {
                
                reset_passcode();
                if (is_locked) {
                    unlock();
                    is_locked = 0;
                } else {
                    lock();
                    is_locked = 1;
                }
            } else {
                reset_passcode();
                lockbox_incorrect_seq();
            }
        // Passcode 'reset'
        } else if (key == '*')
            reset_passcode();
        // Regular char entry
        else {
            // hmmm maybe just write *...
            // maybe display passcode idk
            if (user_entry_pos == 15) {
                // Maybe do something? Display? idk
                // maybe just compare if another key is pressed?
                return;
            }
            LCD_write('*', W_DAT);
            user_entry[user_entry_pos] = key;
            user_entry_pos++;
        }
        // LCD_write(key, W_DAT);
        // Don't read multiple of the same key...
        while (read_keypad() == key) {};
    }
}