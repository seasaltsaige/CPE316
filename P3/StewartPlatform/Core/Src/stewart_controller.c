#include "stewart_controller.h"
#include "stm32l476xx.h"
#include <stdint.h>
#include <stdio.h>

Stepper_t motors[6] = {
    { TIM1, &TIM1->CCR1, LEG_A_PORT, DIR_PIN_A1, HOME_PIN_A1, LIMIT_PIN_A1 },
    { TIM2, &TIM2->CCR1, LEG_A_PORT, DIR_PIN_A2, HOME_PIN_A2, LIMIT_PIN_A2 },
    { TIM3, &TIM3->CCR1, LEG_B_PORT, DIR_PIN_B1, HOME_PIN_B1, LIMIT_PIN_B1 },
    { TIM4, &TIM4->CCR1, LEG_B_PORT, DIR_PIN_B2, HOME_PIN_B2, LIMIT_PIN_B2 },
    { TIM5, &TIM5->CCR2, LEG_Ca_PORT, DIR_PIN_C1, HOME_PIN_C1, LIMIT_PIN_C1 },
    { TIM8, &TIM8->CCR1, LEG_Cb_PORT, DIR_PIN_C2, HOME_PIN_C2, LIMIT_PIN_C2 },
};

void STEWART_init() {
    RCC->AHB2ENR |= RCC_AHB2ENR_GPIOAEN | RCC_AHB2ENR_GPIOBEN | RCC_AHB2ENR_GPIOCEN;
    // so many timers
    // platform has 6 independent steppers, so we need to be able to generate 6 independent PWM waves
    // 1 timer for acceleration/deceleration
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_TIM8EN; 
    RCC->APB1ENR1 |= RCC_APB1ENR1_TIM2EN | RCC_APB1ENR1_TIM3EN | RCC_APB1ENR1_TIM4EN | RCC_APB1ENR1_TIM5EN | RCC_APB1ENR1_TIM6EN;
    // EXTI interrupts
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    // GPIO init, PA0, PA1, PA8, PB4, PB6, PC6
    // set to AF mode
    GPIOA->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1 | GPIO_MODER_MODE8);
    GPIOA->MODER |=  (GPIO_MODER_MODE0_1 | GPIO_MODER_MODE1_1 | GPIO_MODER_MODE8_1);
    GPIOB->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE6);
    GPIOB->MODER |=  (GPIO_MODER_MODE4_1 | GPIO_MODER_MODE6_1);
    GPIOC->MODER &= ~(GPIO_MODER_MODE6);
    GPIOC->MODER |=  (GPIO_MODER_MODE6_1);

    // PA10, PA11, PA12, PB0, PB1, and PC0 are used dir pins
    // for the stepper motors
    // setting to output mode
    GPIOA->MODER &= ~(GPIO_MODER_MODE10 | GPIO_MODER_MODE11 | GPIO_MODER_MODE12);
    GPIOA->MODER |=  (GPIO_MODER_MODE10_0 | GPIO_MODER_MODE11_0 | GPIO_MODER_MODE12_0);
    GPIOB->MODER &= ~(GPIO_MODER_MODE0 | GPIO_MODER_MODE1);
    GPIOB->MODER |=  (GPIO_MODER_MODE0_0 | GPIO_MODER_MODE1_0);
    GPIOC->MODER &= ~(GPIO_MODER_MODE0);
    GPIOC->MODER |=  (GPIO_MODER_MODE0_0);


    // PA4 - EXTI4
    // PA5 - EXTI5
    // PA6 - EXTI6
    // PA7 - EXTI7
    // PA1 - EXTI1
    // PA9 - EXTI9

    // PB14 - EXTI14
    // PB15 - EXTI15
    // PB10 - EXTI10
    // PB11 - EXTI11

    // PC2 - EXTI2
    // PC3 - EXTI3
    // are left in reset state for input mode
    // as they act as the limit switch inputs
    // Set to pull-down mode
    GPIOA->PUPDR &= ~(
        GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 |
        GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD9
    );
    GPIOA->PUPDR |= (
        GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD4_1 | GPIO_PUPDR_PUPD5_1 |
        GPIO_PUPDR_PUPD6_1 | GPIO_PUPDR_PUPD7_1 | GPIO_PUPDR_PUPD9_1
    );
    GPIOB->PUPDR &= ~(
        GPIO_PUPDR_PUPD14 | GPIO_PUPDR_PUPD15 |
        GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11
    );
    GPIOB->PUPDR |= (
        GPIO_PUPDR_PUPD14_1 | GPIO_PUPDR_PUPD15_1 |
        GPIO_PUPDR_PUPD10_1 | GPIO_PUPDR_PUPD11_1
    );
    GPIOC->PUPDR &= ~(      
        GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3
    );
    GPIOC->PUPDR |= (      
        GPIO_PUPDR_PUPD2_1 | GPIO_PUPDR_PUPD3_1
    );

    // Configure EXTI interrupts for each channel

    // EXTI1, 4, 5, 6, 7, and 9 are mapped to Port A
    // EXTI10, 11, 14, and 15 are mapped to Port B
    // EXTI2 and 3 are mapped to Port C

    // Clear EXTICR registers first
    // controls EXTI0-3
    SYSCFG->EXTICR[0] &= ~(
        SYSCFG_EXTICR1_EXTI1 | SYSCFG_EXTICR1_EXTI2 | SYSCFG_EXTICR1_EXTI3
    );
    // Controls EXTI4-7
    SYSCFG->EXTICR[1] &= ~(
        SYSCFG_EXTICR2_EXTI4 | SYSCFG_EXTICR2_EXTI5 | 
        SYSCFG_EXTICR2_EXTI6 | SYSCFG_EXTICR2_EXTI7 
    );
    // Controls EXTI8-11
    SYSCFG->EXTICR[2] &= ~(
        SYSCFG_EXTICR3_EXTI9 | SYSCFG_EXTICR3_EXTI10 | SYSCFG_EXTICR3_EXTI11
    );
    // Controls EXTI12-15
    SYSCFG->EXTICR[3] &= ~(
        SYSCFG_EXTICR4_EXTI14 | SYSCFG_EXTICR4_EXTI15
    );

    // Set to ports as described above
    SYSCFG->EXTICR[0] |= (
        SYSCFG_EXTICR1_EXTI1_PA | SYSCFG_EXTICR1_EXTI2_PC | SYSCFG_EXTICR1_EXTI3_PC
    );
    SYSCFG->EXTICR[1] |= (
        SYSCFG_EXTICR2_EXTI4_PA | SYSCFG_EXTICR2_EXTI5_PA | 
        SYSCFG_EXTICR2_EXTI6_PA | SYSCFG_EXTICR2_EXTI7_PA 
    );
    SYSCFG->EXTICR[2] |= (
        SYSCFG_EXTICR3_EXTI9_PA | SYSCFG_EXTICR3_EXTI10_PB | SYSCFG_EXTICR3_EXTI11_PB
    );
    SYSCFG->EXTICR[3] |= (
        SYSCFG_EXTICR4_EXTI14_PB | SYSCFG_EXTICR4_EXTI15_PB
    );

    // Unmask interrupts in IMR reg
    EXTI->IMR1 |= (
        EXTI_IMR1_IM1 | EXTI_IMR1_IM2 | EXTI_IMR1_IM3 |
        EXTI_IMR1_IM4 | EXTI_IMR1_IM5 | EXTI_IMR1_IM6 | 
        EXTI_IMR1_IM7 | EXTI_IMR1_IM9 | EXTI_IMR1_IM10 | 
        EXTI_IMR1_IM11 | EXTI_IMR1_IM14 | EXTI_IMR1_IM15
    );

    // Enable rising edge trigger (end stops active high)
    EXTI->RTSR1 |= (
        EXTI_RTSR1_RT1 | EXTI_RTSR1_RT2 | EXTI_RTSR1_RT3 |
        EXTI_RTSR1_RT4 | EXTI_RTSR1_RT5 | EXTI_RTSR1_RT6 | 
        EXTI_RTSR1_RT7 | EXTI_RTSR1_RT9 | EXTI_RTSR1_RT10 |
        EXTI_RTSR1_RT11 | EXTI_RTSR1_RT14 | EXTI_RTSR1_RT15
    );

    // Disable falling edge trigger (should already be done, but doesnt hurt)
    EXTI->FTSR1 &= ~(
        EXTI_FTSR1_FT1 | EXTI_FTSR1_FT2 | EXTI_FTSR1_FT3 |
        EXTI_FTSR1_FT4 | EXTI_FTSR1_FT5 | EXTI_FTSR1_FT6 |
        EXTI_FTSR1_FT7 | EXTI_FTSR1_FT9 | EXTI_FTSR1_FT10 |
        EXTI_FTSR1_FT11 | EXTI_FTSR1_FT14 | EXTI_FTSR1_FT15
    );

    // Reset all DIR pins to low state, just in case
    GPIOA->BRR = (GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12);
    GPIOB->BRR = (GPIO_PIN_0  | GPIO_PIN_1);
    GPIOC->BRR = (GPIO_PIN_0);

    // High speed output for pwm, not sure if needed, but doesn't
    // hurt i dont think
    GPIOA->OSPEEDR |= (GPIO_OSPEEDR_OSPEED0 | GPIO_OSPEEDR_OSPEED1 | GPIO_OSPEEDR_OSPEED8);
    GPIOB->OSPEEDR |= (GPIO_OSPEEDR_OSPEED4 | GPIO_OSPEEDR_OSPEED6);
    GPIOC->OSPEEDR |= (GPIO_OSPEEDR_OSPEED6);
    
    // Clear alternate function for pins 0 and 8
    LEG_A_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL0);
    LEG_A_PORT->AFR[1] &= ~(GPIO_AFRH_AFRH0);
    // Set pins into PWM AF1
    LEG_A_PORT->AFR[0] |= (GPIO_AFRL_AFSEL0_0);
    LEG_A_PORT->AFR[1] |= (GPIO_AFRH_AFSEL8_0);


    // Clear alternate functions for pins 4 and 6
    LEG_B_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL4 | GPIO_AFRL_AFRL6);
    // Set pins into PWM AF2
    LEG_B_PORT->AFR[0] |= (GPIO_AFRL_AFSEL4_1 | GPIO_AFRL_AFSEL6_1);

    LEG_Ca_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL1);
    LEG_Cb_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL6);

    // AF2 select
    LEG_Ca_PORT->AFR[0] |= (GPIO_AFRL_AFSEL1_1);
    // AF3 select
    LEG_Cb_PORT->AFR[0] |= (GPIO_AFRL_AFSEL6_0 | GPIO_AFRL_AFSEL6_1);


    // Set timers into PWM mode
    TIM1->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
    TIM2->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
    TIM3->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
    TIM4->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);
    TIM5->CCMR1 |= (TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1);
    TIM8->CCMR1 |= (TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1);

    // Enable preload so new CCR values are loaded on update
    TIM1->CCMR1 |= (TIM_CCMR1_OC1PE);
    TIM2->CCMR1 |= (TIM_CCMR1_OC1PE);
    TIM3->CCMR1 |= (TIM_CCMR1_OC1PE);
    TIM4->CCMR1 |= (TIM_CCMR1_OC1PE);
    TIM5->CCMR1 |= (TIM_CCMR1_OC2PE);
    TIM8->CCMR1 |= (TIM_CCMR1_OC1PE);

    // Set channel outputs (ch1 for most, ch2 for TIM5)
    TIM1->CCER |= (TIM_CCER_CC1E);
    TIM2->CCER |= (TIM_CCER_CC1E);
    TIM3->CCER |= (TIM_CCER_CC1E);
    TIM4->CCER |= (TIM_CCER_CC1E);
    TIM5->CCER |= (TIM_CCER_CC2E);
    TIM8->CCER |= (TIM_CCER_CC1E);

    // ARR preload like CCR
    TIM1->CR1 |= (TIM_CR1_ARPE);
    TIM2->CR1 |= (TIM_CR1_ARPE);
    TIM3->CR1 |= (TIM_CR1_ARPE);
    TIM4->CR1 |= (TIM_CR1_ARPE);
    TIM5->CR1 |= (TIM_CR1_ARPE);
    TIM8->CR1 |= (TIM_CR1_ARPE);

    TIM6->CR1 |= (TIM_CR1_ARPE);

    // Enable advanced timers
    TIM1->BDTR |= (TIM_BDTR_MOE);
    TIM8->BDTR |= (TIM_BDTR_MOE);

    // PSC of 79 to divide 80MHz clock by 80
    // for a timer speed of 1MHz
    TIM1->PSC = 80 - 1;
    TIM2->PSC = 80 - 1;
    TIM3->PSC = 80 - 1;
    TIM4->PSC = 80 - 1;
    TIM5->PSC = 80 - 1;
    TIM8->PSC = 80 - 1;

    TIM6->PSC = 80 - 1;

    // Defaults (16 bits since some would be truncated otherwise)
    TIM1->ARR = 0xFFFF;
    TIM2->ARR = 0xFFFF;
    TIM3->ARR = 0xFFFF;
    TIM4->ARR = 0xFFFF;
    TIM5->ARR = 0xFFFF;
    TIM8->ARR = 0xFFFF;

    // At 1MHz, an ARR of 1000 yields
    // an acceleration frequency of 1ms
    TIM6->ARR = 1000 - 1;

    TIM1->CCR1 = 0;
    TIM2->CCR1 = 0;
    TIM3->CCR1 = 0;
    TIM4->CCR1 = 0;
    TIM5->CCR2 = 0;
    TIM8->CCR1 = 0;

    TIM1->EGR |= (TIM_EGR_UG);
    TIM2->EGR |= (TIM_EGR_UG);
    TIM3->EGR |= (TIM_EGR_UG);
    TIM4->EGR |= (TIM_EGR_UG);
    TIM5->EGR |= (TIM_EGR_UG);
    TIM8->EGR |= (TIM_EGR_UG);

    // Clear interrupt flags
    TIM1->SR &= ~(TIM_SR_UIF);
    TIM2->SR &= ~(TIM_SR_UIF);
    TIM3->SR &= ~(TIM_SR_UIF);
    TIM4->SR &= ~(TIM_SR_UIF);
    TIM5->SR &= ~(TIM_SR_UIF);
    TIM8->SR &= ~(TIM_SR_UIF);

    TIM6->SR &= ~(TIM_SR_UIF);

    // Enable interrupt in DIER reg
    TIM1->DIER |= (TIM_DIER_UIE);
    TIM2->DIER |= (TIM_DIER_UIE);
    TIM3->DIER |= (TIM_DIER_UIE);
    TIM4->DIER |= (TIM_DIER_UIE);
    TIM5->DIER |= (TIM_DIER_UIE);
    TIM8->DIER |= (TIM_DIER_UIE);

    TIM6->DIER |= (TIM_DIER_UIE);

    // turn on interrupt vectors
    NVIC->ISER[0] |= (
        (1UL << TIM1_UP_TIM16_IRQn) |
        (1UL << TIM2_IRQn) |
        (1UL << TIM3_IRQn) |
        (1UL << TIM4_IRQn) |
        (1UL << EXTI1_IRQn) |
        (1UL << EXTI2_IRQn) |
        (1UL << EXTI3_IRQn) |
        (1UL << EXTI4_IRQn) |
        (1UL << EXTI9_5_IRQn)
    );
    NVIC->ISER[1] |= (
        (1UL << (TIM5_IRQn - 32)) |
        (1UL << (TIM8_UP_IRQn - 32)) |
        (1UL << (TIM6_DAC_IRQn - 32)) |
        (1UL << (EXTI15_10_IRQn - 32)) 
    );
    // TODO: EXTI isrs SHOULD have the highest priority
    // followed by acceleration timer
    // followed by step timers

    // NVIC->IP[TIM1_UP_TIM16_IRQn] = 0x10;
    // NVIC->IP[TIM2_IRQn] = 0x10;
    // NVIC->IP[TIM3_IRQn] = 0x10;
    // NVIC->IP[TIM4_IRQn] = 0x10;
    // NVIC->IP[TIM5_IRQn] = 0x10;
    // NVIC->IP[TIM8_UP_IRQn] = 0x10;
    // NVIC->IP[TIM6_DAC_IRQn] = 0x00;
    
    // enable interrupts in the core
    __enable_irq();

    // Enable timers! yippee
    TIM1->CR1 |= TIM_CR1_CEN;
    // TIM2->CR1 |= TIM_CR1_CEN;
    // TIM3->CR1 |= TIM_CR1_CEN;
    // TIM4->CR1 |= TIM_CR1_CEN;
    // TIM5->CR1 |= TIM_CR1_CEN;
    // TIM8->CR1 |= TIM_CR1_CEN;


    TIM6->CR1 |= TIM_CR1_CEN;
}

void stepper_move(Stepper_t *m, int32_t steps, uint32_t total_time_ms) {
    if (steps == 0) return;

    if (steps < 0) {
        m->dir_port->BSRR = m->dir_pin;
        steps = -steps;
    } else {
        m->dir_port->BRR = m->dir_pin;
    }

    uint32_t v_cruise = (2 * (uint32_t)steps * 1000) / total_time_ms;

    uint32_t arr_cruise = 1000000UL / v_cruise;

    if (v_cruise < 3000) {

        uint32_t v_flat  = ((uint32_t)steps * 1000) / total_time_ms;
        if (v_flat == 0) v_flat = 1;
        uint32_t arr_flat = 1000000UL / v_flat;
        if (arr_flat > 0xFFFF) arr_flat = 0xFFFF;

        m->steps_accel     = 0;
        m->steps_decel     = steps;
        m->arr_step        = 0;
        m->steps_total     = steps;
        m->steps_remaining = steps;
        m->arr_current     = arr_flat;
        m->arr_fast        = arr_flat;
        m->arr_slow        = arr_flat;
        m->running         = 1;

        m->timer->ARR = arr_flat;
        *(m->CCR)     = arr_flat / 2;
        m->timer->EGR |= TIM_EGR_UG;
        m->timer->CR1 |= TIM_CR1_CEN;
        return;
    }

    // At start/end of ramp, velocity = 0 — but we need a practical slow speed
    // Use 10% of cruise speed as the starting speed
    uint32_t arr_slow = arr_cruise * 10;
    if (arr_slow > 0xFFFF) arr_slow = 0xFFFF;  // clamp to timer max

    // Phase boundaries by step count
    // accel: first 1/4 of steps, cruise: middle 1/2, decel: last 1/4
    m->steps_accel  = steps / 4;
    m->steps_decel  = steps - (steps / 4);  // decel starts here

    uint32_t accel_ticks = total_time_ms / 3;
    if (accel_ticks == 0) accel_ticks = 1;
    m->arr_step = (arr_slow - arr_cruise) / accel_ticks;
    if (m->arr_step == 0) m->arr_step = 1;

    m->steps_total     = steps;
    m->steps_remaining = steps;
    m->arr_current     = arr_slow;
    m->arr_fast        = arr_cruise;
    m->arr_slow        = arr_slow;
    m->running         = 1;

    m->timer->ARR = arr_slow;
    *(m->CCR)     = arr_slow / 2;
    m->timer->EGR |= TIM_EGR_UG;
    m->timer->CR1 |= TIM_CR1_CEN;
}


void stepper_tick(Stepper_t *m) {
    if (!m->running) return;

    m->steps_remaining--;

    if (m->steps_remaining <= 0) {
        *(m->CCR) = 0;
        m->running = 0;
        return;
    }

}

void stepper_accel(Stepper_t *m) {
    if (!m->running) return;

    int32_t steps_done = m->steps_total - m->steps_remaining;

    if (steps_done < m->steps_accel) {
        // Accelerating: decrease ARR toward cruise
        if (m->arr_current > m->arr_fast + m->arr_step) {
            m->arr_current -= m->arr_step;
        } else {
            m->arr_current = m->arr_fast;
        }
    } else if (m->steps_remaining < m->steps_accel) {
        // Decelerating: increase ARR back toward slow
        if (m->arr_current < m->arr_slow - m->arr_step) {
            m->arr_current += m->arr_step;
        } else {
            m->arr_current = m->arr_slow;
        }
    }
    // else: cruise phase, ARR stays at arr_fast

    m->timer->ARR = m->arr_current;
    *(m->CCR)     = m->arr_current / 2;
}


void home_platform() {

}

void stepper_move_const_vel(Stepper_t *m, int32_t steps, uint32_t vel_mm_s) {

    if (steps < 0) {
        m->dir_port->BSRR = m->dir_pin;
        steps = -steps;
    } else {
        m->dir_port->BRR = m->dir_pin;
    }

    uint32_t vel_step_s = (vel_mm_s * STEPS_PER_REV) / MM_PER_REV;
    uint32_t arr_val = (1000000UL / (vel_step_s));

    m->steps_accel     = 0;
    m->steps_decel     = steps;
    m->arr_step        = 0;
    m->steps_total     = steps;
    m->steps_remaining = steps;
    m->arr_current     = arr_val;
    m->arr_fast        = arr_val;
    m->arr_slow        = arr_val;
    m->running         = 1;

    m->timer->ARR = arr_val;
    *(m->CCR)     = arr_val / 2;
    m->timer->EGR |= TIM_EGR_UG;
    m->timer->CR1 |= TIM_CR1_CEN;
}

void home_stepper(Stepper_t *m) {
    // move to home limit switch quick
    
    // back off
    
    // move to home limit switch slowly
    
    // back off


    // extend to limit- limit switch quick

    // back off

    // move to limit- limit switch slowly

    // set max step count of linear actuator for max extension

    // return to 'home'
}