#ifndef __STEWART_CONTROLLER_H
#define __STEWART_CONTROLLER_H

#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>

// --- Motion constants ---
#define STEPS_PER_REV       200
#define LEAD_SCREW_LEN_MM   300
#define MM_PER_REV          2
#define MOVE_TIME_MS        2000

// Minimum cruise speed before we fall back to constant-velocity mode.
// Below this the trapezoidal ramp is too shallow to be useful.
// 300 steps/s = 1.5 rev/s = 3 mm/s on a 2mm-pitch lead screw.
#define MIN_CRUISE_STEPS_S  300

// --- Leg A ---
#define LEG_A_PORT      GPIOA
#define LEG_A           (GPIO_PIN_8 | GPIO_PIN_0) // PA8, PA0
#define DIR_PIN_A1      (GPIO_PIN_10) // PA10
#define DIR_PIN_A2      (GPIO_PIN_11) // PA11
// Leg A EXTI interrupts
#define HOME_PIN_A1     (GPIO_PIN_4) // PA4 - EXTI4
#define LIMIT_PIN_A1    (GPIO_PIN_5) // PA5 - EXTI5
#define HOME_PIN_A2     (GPIO_PIN_6) // PA6 - EXTI6
#define LIMIT_PIN_A2    (GPIO_PIN_7) // PA7 - EXTI7

// --- Leg B ---
#define LEG_B_PORT      GPIOB
#define LEG_B           (GPIO_PIN_4 | GPIO_PIN_6) // PB4, PB6
#define DIR_PIN_B1      (GPIO_PIN_0) // PB0
#define DIR_PIN_B2      (GPIO_PIN_1) // PB1
// Leg B EXTI interrupts
#define HOME_PIN_B1     (GPIO_PIN_14) // PB14 - EXTI14
#define LIMIT_PIN_B1    (GPIO_PIN_15) // PB15 - EXTI15
#define HOME_PIN_B2     (GPIO_PIN_10) // PB10 - EXTI10
#define LIMIT_PIN_B2    (GPIO_PIN_11) // PB11 - EXTI11

// --- Leg C motor 1 ---
#define LEG_Ca_PORT     GPIOA
#define LEG_Ca          (GPIO_PIN_2) // PA2
#define DIR_PIN_C1      (GPIO_PIN_12) // PA12
// LEG C1 EXTI interrupts
#define HOME_PIN_C1     (GPIO_PIN_1) // PA1 - EXTI1
#define LIMIT_PIN_C1    (GPIO_PIN_9) // PA9 - EXTI9

// --- Leg C motor 2 ---
#define LEG_Cb_PORT     GPIOC
#define LEG_Cb          (GPIO_PIN_6) // PC6
#define DIR_PIN_C2      (GPIO_PIN_0) // PC0
// Leg C2 EXTI interrupts
#define HOME_PIN_C2     (GPIO_PIN_2) // PC2 - EXTI2
#define LIMIT_PIN_C2    (GPIO_PIN_3) // PC3 - EXTI3


typedef struct {
    volatile TIM_TypeDef    *timer;
    volatile uint32_t       *CCR;
    volatile GPIO_TypeDef   *dir_port;
    volatile uint16_t        dir_pin;
    volatile uint16_t        home_pin;
    volatile uint16_t        limit_pin;

    volatile uint8_t  homing;
    volatile uint8_t  extending;
    volatile uint8_t  running;

    // Step counters
    volatile int32_t  steps_remaining;
    volatile int32_t  steps_total;
    volatile int32_t  steps_accel;      // step index at which accel phase ends
    volatile int32_t  steps_decel;      // step index at which decel phase begins

    // ARR (period) profile — all in 1MHz timer ticks (µs)
    volatile uint32_t arr_current;
    volatile uint32_t arr_fast;         // cruise period (shortest = fastest)
    volatile uint32_t arr_slow;         // start/end period (longest = slowest)
    volatile uint32_t arr_step;         // ARR change applied on each step tick

    // Persistent position tracking
    volatile int32_t  steps_full_travel; // measured at startup via homing
    volatile int32_t  steps_current;     // tracked from home position
} Stepper_t;

extern Stepper_t motors[6];

void STEWART_init(void);
void stepper_move_const_vel(Stepper_t *m, int32_t steps, uint32_t vel_mm_s);
void stepper_move(Stepper_t *m, int32_t steps, uint32_t total_time_ms);
void stepper_tick(Stepper_t *m);
void stepper_accel(Stepper_t *m);
void home_platform(void);
void home_stepper(Stepper_t *m);

#endif /* __STEWART_CONTROLLER_H */