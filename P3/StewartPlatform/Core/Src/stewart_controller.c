#include "stewart_controller.h"
#include "stm32l476xx.h"

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

    // PA4, PA5, PA6, PA7, PA1, PA9
    // PB7, PB8, PB9, PB10
    // PC1, PC2
    // are left in reset state for input mode
    // as they act as the limit switch inputs
    // Set to pull-down mode
    GPIOA->PUPDR &= ~(
        GPIO_PUPDR_PUPD4 | GPIO_PUPDR_PUPD5 |
        GPIO_PUPDR_PUPD6 | GPIO_PUPDR_PUPD7 | 
        GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD9
    );
    GPIOA->PUPDR |= (
        GPIO_PUPDR_PUPD4_1 | GPIO_PUPDR_PUPD5_1 |
        GPIO_PUPDR_PUPD6_1 | GPIO_PUPDR_PUPD7_1 | 
        GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD9_1
    );
    GPIOB->PUPDR &= ~(
        GPIO_PUPDR_PUPD7 | GPIO_PUPDR_PUPD8 |
        GPIO_PUPDR_PUPD9 | GPIO_PUPDR_PUPD10
    );
    GPIOB->PUPDR |= (
        GPIO_PUPDR_PUPD7_1 | GPIO_PUPDR_PUPD8_1 |
        GPIO_PUPDR_PUPD9_1 | GPIO_PUPDR_PUPD10_1
    );
    GPIOC->PUPDR &= ~(      
        GPIO_PUPDR_PUPD1 | GPIO_PUPDR_PUPD2
    );
    GPIOC->PUPDR |= (      
        GPIO_PUPDR_PUPD1_1 | GPIO_PUPDR_PUPD2_1
    );


    // Reset to low state, just in case
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
        (1UL << TIM4_IRQn)
    );
    NVIC->ISER[1] |= (
        (1UL << (TIM5_IRQn - 32)) |
        (1UL << (TIM8_UP_IRQn - 32)) |
        (1UL << (TIM6_DAC_IRQn - 32))
    );

    // enable interrupts in the core
    __enable_irq();

    // Enable timers! yippee
    TIM1->CR1 |= TIM_CR1_CEN;
    TIM2->CR1 |= TIM_CR1_CEN;
    TIM3->CR1 |= TIM_CR1_CEN;
    TIM4->CR1 |= TIM_CR1_CEN;
    TIM5->CR1 |= TIM_CR1_CEN;
    TIM8->CR1 |= TIM_CR1_CEN;

    TIM6->CR1 |= TIM_CR1_CEN;
}

void stepper_move(Stepper_t *m, int32_t steps, uint32_t arr_fast, uint32_t arr_slow, uint32_t arr_step) {
    if (steps < 0) {
        m->dir_port->BSRR = m->dir_pin;
        // if going retracting, invert
        steps = -steps;
    } else m->dir_port->BRR = m->dir_pin;

    m->steps_total = steps;
    m->steps_remaining = steps;
    m->accel_time = 0;

    m->arr_current = arr_slow;
    m->arr_fast = arr_fast;
    m->arr_slow = arr_slow;
    m->arr_step = arr_step;
    m->running = 1;

    m->timer->ARR = arr_slow;
    // 50% duty cycle
    *(m->CCR) = arr_slow / 2;
    m->timer->EGR |= TIM_EGR_UG;
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
    // Deceleration 
    if (m->accel_time > ((MOVE_TIME_MS * 2) / 3)) {
        m->arr_current += m->arr_step;
        if (m->arr_current > m->arr_slow) m->arr_current = m->arr_slow;
    } else
    // Acceleration
    if (m->accel_time < (MOVE_TIME_MS / 3)) {
        m->arr_current -= m->arr_step;
        if (m->arr_current < m->arr_fast) m->arr_current = m->arr_fast;

    }
    // Otherwise no need to modify the ARR value
    m->timer->ARR = m->arr_current;
    *(m->CCR) = m->arr_current / 2;
    m->accel_time++;
}

void home_platform() {

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