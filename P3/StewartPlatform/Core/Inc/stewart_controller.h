#ifndef __STEWART_CONTROLLER_H
#define __STEWART_CONTROLLER_H

#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

#define STEPS_PER_REV        200
#define LEAD_SCREW_LEN_MM    300
#define MM_PER_REV           2
#define MOVE_TIME_MS         4000

#define HOMING_FAST_MM_S     17
#define HOMING_SLOW_MM_S     2
#define HOMING_BACKOFF_STEPS 100
#define HOMING_LIMIT_HOLD_MS 300

#define SOFTWARE_STEP_LIMIT 100

#define MIN_TOP_SPEED_MM_S  15

// --- Leg A ---
#define LEG_A_PORT      GPIOA
#define LEG_A           (GPIO_PIN_8 | GPIO_PIN_0) // PA8, PA0 (TIM1_CH1, TIM2_CH2)
#define DIR_PIN_A1      (GPIO_PIN_10) // PA10
#define DIR_PIN_A2      (GPIO_PIN_11) // PA11
// Leg A EXTI interrupts
#define HOME_PIN_A1     (GPIO_PIN_4) // PA4 - EXTI4
#define LIMIT_PIN_A1    (GPIO_PIN_5) // PA5 - EXTI5
// -----
#define HOME_PIN_A2     (GPIO_PIN_6) // PA6 - EXTI6
#define LIMIT_PIN_A2    (GPIO_PIN_7) // PA7 - EXTI7

// --- Leg B ---
#define LEG_B_PORT      GPIOB
#define LEG_B           (GPIO_PIN_4 | GPIO_PIN_6) // PB4, PB6 (TIM3_CH1, TIM4_CH1)
#define DIR_PIN_B1      (GPIO_PIN_0) // PB0
#define DIR_PIN_B2      (GPIO_PIN_1) // PB1
// Leg B EXTI interrupts
#define HOME_PIN_B1     (GPIO_PIN_14) // PB14 - EXTI14
#define LIMIT_PIN_B1    (GPIO_PIN_15) // PB15 - EXTI15
// -----
#define HOME_PIN_B2     (GPIO_PIN_10) // PB10 - EXTI10
#define LIMIT_PIN_B2    (GPIO_PIN_11) // PB11 - EXTI11

// --- Leg C motor 1 ---
#define LEG_Ca_PORT     GPIOA
#define LEG_Ca          (GPIO_PIN_2) // PA2 (TIM5_CH3)
#define DIR_PIN_C1      (GPIO_PIN_12) // PA12
// LEG C1 EXTI interrupts
#define HOME_PIN_C1     (GPIO_PIN_1) // PA1 - EXTI1
#define LIMIT_PIN_C1    (GPIO_PIN_9) // PA9 - EXTI9

// --- Leg C motor 2 ---
#define LEG_Cb_PORT     GPIOC
#define LEG_Cb          (GPIO_PIN_6) // PC6 (TIM8_CH1)
#define DIR_PIN_C2      (GPIO_PIN_0) // PC0
// Leg C2 EXTI interrupts
#define HOME_PIN_C2     (GPIO_PIN_2) // PC2 - EXTI2
#define LIMIT_PIN_C2    (GPIO_PIN_3) // PC3 - EXTI3

// 2 pins per leg for pwm (2 * 3 = 6)
// 2 pins per leg for DIR (2 * 3 = 6) 
// 2 pins per leg for homing (2 * 3 = 6)
// 2 pins per leg for extension (2 * 3 = 6)
// 24 gpio total

typedef enum {
    IDLE, // no motion
    HOMING_FAST, // retract motors quickly until limit switch is hit
    HOMING_FAST_LIMIT,
    HOMING_FAST_LIMIT_TRANSITION,
    HOMING_FAST_BACKOFF, // extend motors slowly until sure limit switches are not pressed (~4mm)
    HOMING_FAST_BACKOFF_DONE, // state transition once steps from previous state finish
    HOMING_FAST_BACKOFF_TRANSITION, 
    HOMING_SLOW, // retract motors slowwwly until limit switch is hit
    HOMING_SLOW_LIMIT,
    HOMING_SLOW_LIMIT_TRANSITION,
    HOMING_SLOW_BACKOFF, // extend motors slowly until instant limit switch is released
    HOMING_SLOW_BACKOFF_DONE,
    HOMING_SLOW_BACKOFF_TRANSITION,
    // TODO: repeat for extension


    EXTENSION_FAST, // retract motors quickly until limit switch is hit
    EXTENSION_FAST_LIMIT,
    EXTENSION_FAST_LIMIT_TRANSITION,
    EXTENSION_FAST_BACKOFF, // extend motors slowly until sure limit switches are not pressed (~4mm)
    EXTENSION_FAST_BACKOFF_DONE, // state transition once steps from previous state finish
    EXTENSION_FAST_BACKOFF_TRANSITION, 
    EXTENSION_SLOW, // retract motors slowwwly until limit switch is hit
    EXTENSION_SLOW_LIMIT,
    EXTENSION_SLOW_LIMIT_TRANSITION,
    EXTENSION_SLOW_BACKOFF, // extend motors slowly until instant limit switch is released
    EXTENSION_SLOW_BACKOFF_DONE,
    EXTENSION_SLOW_BACKOFF_TRANSITION,

    DELAY,
    NORMAL_RUNNING,
} MOTOR_STATE;


typedef struct {
    volatile TIM_TypeDef *timer;
    volatile uint32_t *CCR;
    volatile GPIO_TypeDef *dir_port;
    volatile uint16_t dir_pin;
    volatile uint16_t home_pin;
    volatile uint16_t limit_pin;
    volatile uint64_t EXTI_home_flag;
    volatile uint64_t EXTI_limit_flag;

    volatile MOTOR_STATE motor_state;
    volatile uint32_t delay_time_ms;
    volatile uint32_t ticks_ms;

    volatile uint64_t MAX_STEPS; // MAX EXTENSION RANGE FROM HOME 0 = HOME | MAX_STEPS = FULL EXTENSION
    volatile uint64_t steps_current; // current positioning

    // Step counters
    volatile int8_t current_dir;
    volatile int32_t steps_remaining;
    volatile int32_t steps_total;
    volatile int32_t steps_accel;
    volatile int32_t steps_decel; 

    volatile uint32_t ticks_elapsed;
    volatile uint32_t total_time_ms;

    volatile uint32_t arr_current;
    volatile uint32_t arr_fast;         
    volatile uint32_t arr_slow;         
    volatile uint32_t arr_step;         
} Stepper_t;

extern Stepper_t motors[6];

void STEWART_init(void);
void stepper_move_const_vel(Stepper_t *m, int32_t steps, uint32_t vel_mm_s, MOTOR_STATE type);
void stepper_move(Stepper_t *m, uint64_t step_number, uint32_t total_time_ms);
void stepper_tick(Stepper_t *m);
void stepper_accel(Stepper_t *m);
void home_platform(void);
void stepper_goto_step(Stepper_t *m, uint64_t step_number, uint32_t vel_mm_s);

#endif