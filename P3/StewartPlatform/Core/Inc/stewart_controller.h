#ifndef __STEWART_CONTROLLER_H
#define __STEWART_CONTROLLER_H

#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>


#define STEPS_PER_REV 200
#define LEAD_SCREW_LEN_MM 300
#define MM_PER_REV 2
#define MOVE_TIME_MS 1000

// Leg A defs
#define LEG_A_PORT GPIOA
#define LEG_A (GPIO_PIN_8 | GPIO_PIN_0)
#define DIR_PIN_A1 (GPIO_PIN_10)
#define DIR_PIN_A2 (GPIO_PIN_11)
#define HOME_PIN_A1 (GPIO_PIN_4)
#define LIMIT_PIN_A1 (GPIO_PIN_5)
#define HOME_PIN_A2 (GPIO_PIN_6)
#define LIMIT_PIN_A2 (GPIO_PIN_7)

// Leg B defs
#define LEG_B_PORT GPIOB
#define LEG_B (GPIO_PIN_4 | GPIO_PIN_6)
#define DIR_PIN_B1 (GPIO_PIN_0)
#define DIR_PIN_B2 (GPIO_PIN_1)
#define HOME_PIN_B1 (GPIO_PIN_7)
#define LIMIT_PIN_B1 (GPIO_PIN_8)
#define HOME_PIN_B2 (GPIO_PIN_9)
#define LIMIT_PIN_B2 (GPIO_PIN_10)

// Leg C, motor 1 defs
#define LEG_Ca_PORT GPIOA
#define LEG_Ca (GPIO_PIN_2)
#define DIR_PIN_C1 (GPIO_PIN_12)
#define HOME_PIN_C1 (GPIO_PIN_1)
#define LIMIT_PIN_C1 (GPIO_PIN_9)

// Leg C, motor 2 defs
#define LEG_Cb_PORT GPIOC
#define LEG_Cb (GPIO_PIN_6)
#define DIR_PIN_C2 (GPIO_PIN_0)
#define HOME_PIN_C2 (GPIO_PIN_1)
#define LIMIT_PIN_C2 (GPIO_PIN_2)

typedef struct {
    TIM_TypeDef *timer;    
    volatile uint32_t *CCR;
    GPIO_TypeDef *dir_port;
    uint16_t dir_pin;
    uint16_t home_pin;
    uint16_t limit_pin;
    
    uint8_t homing;
    uint8_t extending;
    uint8_t running;

    int32_t steps_remaining;
    int32_t steps_total;

    uint32_t accel_time;

    uint32_t arr_current;
    uint32_t arr_fast;    
    uint32_t arr_slow;    
    uint32_t arr_step;   
    int32_t steps_full_travel;   // measured at startup
    int32_t steps_current;       // tracked from home
} Stepper_t;


extern Stepper_t motors[6];

void STEWART_init();
void stepper_move(Stepper_t *m, int32_t steps, uint32_t arr_fast, uint32_t arr_slow, uint32_t arr_step);

void stepper_accel(Stepper_t *m);
void stepper_tick(Stepper_t *m);

void home_platform();
void home_stepper(Stepper_t *m);

#endif