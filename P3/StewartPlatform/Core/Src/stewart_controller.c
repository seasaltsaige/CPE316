#include "stewart_controller.h"
#include "stm32l476xx.h"
#include "stm32l4xx_hal.h"
#include <stdint.h>
#include <stdio.h>
#include <math.h>

Stepper_t motors[6] = {
    { TIM1, &TIM1->CCR1, LEG_A_PORT, DIR_PIN_A1, HOME_PIN_A1, LIMIT_PIN_A1, EXTI_IMR1_IM4, EXTI_IMR1_IM5 },
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
    SCB->CPACR |= ((3UL << 10*2) | (3UL << 11*2));

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

        // Set all homing/limit pins as input and pulldown
    GPIOA->MODER &= ~(GPIO_MODER_MODE4 | GPIO_MODER_MODE5 | GPIO_MODER_MODE6 | GPIO_MODER_MODE7 | GPIO_MODER_MODE1 | GPIO_MODER_MODE9);
    GPIOB->MODER &= ~(GPIO_MODER_MODE14 | GPIO_MODER_MODE15 | GPIO_MODER_MODE10 | GPIO_MODER_MODE11);
    GPIOC->MODER &= ~(GPIO_MODER_MODE2 | GPIO_MODER_MODE3);

    GPIOA->PUPDR &= ~(GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 | GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD9);
    GPIOA->PUPDR |= (GPIO_PUPDR_PUPD4_1 | GPIO_PUPDR_PUPD5_1 | GPIO_PUPDR_PUPD6_1 | GPIO_PUPDR_PUPD7_1 | GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD9_1);

    GPIOB->PUPDR &= ~(GPIO_PUPDR_PUPD14 | GPIO_PUPDR_PUPD15 | GPIO_PUPDR_PUPD10 | GPIO_PUPDR_PUPD11);
    GPIOB->PUPDR |= (GPIO_PUPDR_PUPD14_1 | GPIO_PUPDR_PUPD15_1 | GPIO_PUPDR_PUPD10_1 | GPIO_PUPDR_PUPD11_1);

    GPIOC->PUPDR &= ~(GPIO_PUPDR_PUPD2 | GPIO_PUPDR_PUPD3);
    GPIOC->PUPDR |= (GPIO_PUPDR_PUPD2_1 | GPIO_PUPDR_PUPD3_1);

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
    EXTI->FTSR1 |= (
        EXTI_FTSR1_FT1 | EXTI_FTSR1_FT2 | EXTI_FTSR1_FT3 |
        EXTI_FTSR1_FT4 | EXTI_FTSR1_FT5 | EXTI_FTSR1_FT6 |
        EXTI_FTSR1_FT7 | EXTI_FTSR1_FT9 | EXTI_FTSR1_FT10 |
        EXTI_FTSR1_FT11 | EXTI_FTSR1_FT14 | EXTI_FTSR1_FT15
    );

    EXTI->PR1 = (
        EXTI_PR1_PIF1  | EXTI_PR1_PIF2  |
        EXTI_PR1_PIF3  | EXTI_PR1_PIF4  |
        EXTI_PR1_PIF5  | EXTI_PR1_PIF6  |
        EXTI_PR1_PIF7  | EXTI_PR1_PIF9  |
        EXTI_PR1_PIF10 | EXTI_PR1_PIF11 |
        EXTI_PR1_PIF14 | EXTI_PR1_PIF15
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

    LEG_Ca_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL2);
    LEG_Cb_PORT->AFR[0] &= ~(GPIO_AFRL_AFRL6);
    // AF2 select
    LEG_Ca_PORT->AFR[0] |= (GPIO_AFRL_AFSEL2_1);
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


    // ALL limit switch interrupts (EXTI)
    // have highest priority of 0, followed
    // by TIM6 which is a time keeper and accel
    // handler, finally followed by the 6 timers
    // for step tracking on each leg
    NVIC->IP[TIM1_UP_TIM16_IRQn] = 0xF0;
    NVIC->IP[TIM2_IRQn] = 0xF0;
    NVIC->IP[TIM3_IRQn] = 0xF0;
    NVIC->IP[TIM4_IRQn] = 0xF0;
    NVIC->IP[TIM5_IRQn] = 0xF0;
    NVIC->IP[TIM8_UP_IRQn] = 0xF0;
    NVIC->IP[TIM6_DAC_IRQn] = 0x10;
    NVIC->IP[EXTI1_IRQn] = 0x00;
    NVIC->IP[EXTI2_IRQn] = 0x00;
    NVIC->IP[EXTI3_IRQn] = 0x00;
    NVIC->IP[EXTI4_IRQn] = 0x00;
    NVIC->IP[EXTI9_5_IRQn] = 0x00;
    NVIC->IP[EXTI15_10_IRQn] = 0x00;


    
    // enable interrupts in the core
    __enable_irq();

    // Enable timers! yippee

    GPIOA->MODER &= ~(GPIO_MODER_MODE3);
    GPIOA->MODER |= (GPIO_MODER_MODE3_0);

    TIM6->CR1 |= TIM_CR1_CEN;
}

void stepper_move(Stepper_t *m, uint64_t step_number, uint32_t total_time_ms) {
    if (m->steps_current == step_number) return;

    // calculate direction and step distance to travel
    uint64_t distance = 0;
    if (step_number < m->steps_current) {
        m->dir_port->BSRR = m->dir_pin;
        m->current_dir = -1;
        distance = m->steps_current - step_number;
    } else {
        m->dir_port->BRR = m->dir_pin;
        m->current_dir = 1;
        distance = step_number - m->steps_current;
    }

    uint32_t arr_peak = (uint32_t)(((uint64_t)total_time_ms * 500UL) / distance);
    if (arr_peak < 1)      arr_peak = 1;
    if (arr_peak > 0xFFFF) arr_peak = 0xFFFF;

    // 667 arr is ~like 20mm/s i think...
    // also just kinda felt good so...
    if (arr_peak >= 667) {
        uint32_t arr_flat = (uint32_t)(((uint64_t)total_time_ms * 1000UL) / distance);
        if (arr_flat < 1)      arr_flat = 1;
        if (arr_flat > 0xFFFF) arr_flat = 0xFFFF;

        m->ticks_elapsed = 0;
        m->total_time_ms = total_time_ms;
        m->steps_accel = 0;
        m->steps_decel = distance;
        m->arr_step = 0;
        m->steps_total = distance;
        m->steps_remaining = distance;
        m->arr_current = arr_flat;
        m->arr_fast = arr_flat;
        m->arr_slow = arr_flat;
        m->motor_state = NORMAL_RUNNING;

        m->timer->ARR = arr_flat;
        *(m->CCR)     = arr_flat / 2;
        m->timer->EGR |= TIM_EGR_UG;
        m->timer->CR1 |= TIM_CR1_CEN;
        return;
    }

    uint64_t half_steps = (distance / 2);
    if (half_steps == 0) half_steps = 1;

    m->ticks_elapsed = 0;
    m->total_time_ms = total_time_ms;
    m->steps_accel = half_steps;
    m->steps_decel = half_steps;
    m->steps_total = distance;
    m->steps_remaining = distance;
    m->arr_fast = arr_peak;
    m->arr_slow = 0xFFFF;
    m->arr_current = 0xFFFF;
    m->arr_step = 1;
    m->motor_state = NORMAL_RUNNING;

    m->timer->ARR = 0xFFFF;
    *(m->CCR) = 0xFFFF / 2;
    m->timer->EGR |= TIM_EGR_UG;
    m->timer->CR1 |= TIM_CR1_CEN;
}


void stepper_tick(Stepper_t *m) {
    if (m->motor_state == IDLE) return;
    m->steps_remaining--;

    m->steps_current += m->current_dir;

    // If either finished steps, or hit software limits, exit movement
    if (m->steps_remaining <= 0 || 
        (m->motor_state == NORMAL_RUNNING && 
            ((m->steps_current <= SOFTWARE_STEP_LIMIT && m->current_dir == -1) || 
             (m->steps_current >= (m->MAX_STEPS - SOFTWARE_STEP_LIMIT) && m->current_dir == 1))
        )
    ) {

        m->steps_remaining = 0;
        *(m->CCR) = 0;
        m->timer->CR1 &= ~(TIM_CR1_CEN);

        if (m->motor_state == HOMING_FAST_BACKOFF)
            m->motor_state = HOMING_FAST_BACKOFF_DONE;

        else if (m->motor_state == EXTENSION_FAST_BACKOFF)
            m->motor_state = EXTENSION_FAST_BACKOFF_DONE;

        else m->motor_state = IDLE;
        

        return;
    }

}

void stepper_accel(Stepper_t *m) {
    if (m->motor_state != NORMAL_RUNNING) return;

    if (m->arr_step != 0) {

        float t_ticks = (float)m->ticks_elapsed / (float)m->total_time_ms;

        // in theory this should never be the case, but just in case
        if (t_ticks > 1.0f) t_ticks = 1.0f;

        // Calculate velocity based on cosine wave
        // in range from 0 to 2pi (in terms of ticks calculate above)
        float v_norm = (1.0f - cosf(2 * M_PI * t_ticks)) / 2;

        uint32_t arr;
        if (v_norm < 0.001f) {
            // if the speed is too small, just set it 
            // at our max arr value (16 bit limit)
            arr = 0xFFFF;
        } else {
            // otherwise, calculate the arr value based on the 
            // fastest calculated arr value
            arr = (uint32_t)((float)m->arr_fast / v_norm);
            if (arr > 0xFFFF) arr = 0xFFFF;
            if (arr < m->arr_fast) arr = m->arr_fast;
        }

        m->arr_current = arr;
        m->ticks_elapsed++;
    }

    m->timer->ARR = m->arr_current;
    *(m->CCR) = m->arr_current / 2;
}


void home_platform() {
    const uint8_t NUM_MOTORS = 1; 
    // Loop through motors
    // todo: add rest of motors
    for (int i = 0; i < NUM_MOTORS; i++) {
        // move down at 20mm/s
        // begin in homing fast state
        stepper_move_const_vel(&motors[i], -999999, HOMING_FAST_MM_S, HOMING_FAST);
    }

    // while all motors have not re-entered idle
    while (
        (motors[0].motor_state != IDLE && motors[0].motor_state != NORMAL_RUNNING) ||
        (motors[1].motor_state != IDLE && motors[1].motor_state != NORMAL_RUNNING )

    ) {
        for (int i = 0; i < NUM_MOTORS; i++) {
            switch (motors[i].motor_state) {

                case HOMING_FAST_LIMIT_TRANSITION:
                    stepper_move_const_vel(&motors[i], HOMING_BACKOFF_STEPS, HOMING_SLOW_MM_S, HOMING_FAST_BACKOFF);
                    break;
                case HOMING_FAST_BACKOFF_TRANSITION:
                    stepper_move_const_vel(&motors[i], -999999, HOMING_SLOW_MM_S, HOMING_SLOW);
                    break;
                case HOMING_SLOW_LIMIT_TRANSITION:
                    // arbitrary step count. halted once limit switch is cleared
                    stepper_move_const_vel(&motors[i], 999999, HOMING_SLOW_MM_S, HOMING_SLOW_BACKOFF);
                   break;

                case HOMING_SLOW_BACKOFF_TRANSITION:
                    motors[i].steps_current = 0;
                    stepper_move_const_vel(&motors[i], 999999, HOMING_FAST_MM_S, EXTENSION_FAST);
                    break;

                case EXTENSION_FAST_LIMIT_TRANSITION:
                    stepper_move_const_vel(&motors[i], -HOMING_BACKOFF_STEPS, HOMING_SLOW_MM_S, EXTENSION_FAST_BACKOFF);
                    break;
                case EXTENSION_FAST_BACKOFF_TRANSITION:
                    stepper_move_const_vel(&motors[i], 999999, HOMING_SLOW_MM_S, EXTENSION_SLOW);
                    break;
                case EXTENSION_SLOW_LIMIT_TRANSITION:
                    stepper_move_const_vel(&motors[i], -999999, HOMING_SLOW_MM_S, EXTENSION_SLOW_BACKOFF);
                    break;

                case EXTENSION_SLOW_BACKOFF_TRANSITION:
                // TODO: 
                    motors[i].MAX_STEPS = motors[i].steps_current;
                    // stepper_goto_step(&motors[i], 0, HOMING_FAST_MM_S);
                    stepper_move(&motors[i], 0, MOVE_TIME_MS);
                    break;

                default:
                    break;
            }
        }
    }

}

void stepper_move_const_vel(Stepper_t *m, int32_t steps, uint32_t vel_mm_s, MOTOR_STATE type) {

    m->current_dir     = steps > 0 ? 1 : -1;
    if (steps < 0) {
        m->dir_port->BSRR = m->dir_pin;
        steps = -steps;
    } else {
        m->dir_port->BRR = m->dir_pin;
    }

    uint32_t vel_step_s = (vel_mm_s * STEPS_PER_REV) / MM_PER_REV;
    uint32_t arr_val = (1000000UL / (vel_step_s));

    m->steps_accel     = 0;
    m->steps_decel     = 0;
    m->arr_step        = 0;
    m->steps_total     = steps;
    m->steps_remaining = steps;
    m->arr_current     = arr_val;
    m->arr_fast        = arr_val;
    m->arr_slow        = arr_val;
    m->motor_state     = type;

    m->timer->ARR = arr_val;
    *(m->CCR)     = arr_val / 2;
    m->timer->CNT = 0;
    m->timer->EGR |= TIM_EGR_UG;
    m->timer->CR1 |= TIM_CR1_CEN;
}

void stepper_goto_step(Stepper_t *m, uint64_t step_number, uint32_t vel_mm_s) {
    if (m->steps_current == step_number) return;

    uint64_t distance = 0;
    int8_t dir = 1;
    if (step_number < m->steps_current) {
        // negative dir (towards home)
        m->dir_port->BSRR = m->dir_pin;
        distance = m->steps_current - step_number;
        dir = -1;
    } else {
       // positive dir (towards extension)
       m->dir_port->BRR = m->dir_pin;
       distance = step_number - m->steps_current;
       dir = 1;
    }


    uint32_t vel_step_s = (vel_mm_s * STEPS_PER_REV) / MM_PER_REV;
    uint32_t arr_val = (1000000UL / (vel_step_s));

    m->steps_accel     = 0;
    m->steps_decel     = 0;
    m->arr_step        = 0;
    m->steps_total     = distance;
    m->steps_remaining = distance;
    m->arr_current     = arr_val;
    m->arr_fast        = arr_val;
    m->arr_slow        = arr_val;
    m->motor_state     = NORMAL_RUNNING;
    m->current_dir     = dir;

    m->timer->ARR = arr_val;
    *(m->CCR)     = arr_val / 2;
    m->timer->CNT = 0;
    m->timer->EGR |= TIM_EGR_UG;
    m->timer->CR1 |= TIM_CR1_CEN;



}