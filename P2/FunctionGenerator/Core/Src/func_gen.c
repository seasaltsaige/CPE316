#include "func_gen.h"
#include "DAC.h"
#include "stm32l476xx.h"
#include <stdint.h>

enum WAVE_FREQ wave_freq = ONE;
enum WAVE_TYPE wave_type = SQUARE;
enum SQUARE_DUTY duty_cycle = FIFTY;

uint16_t dac_output = 0;
uint16_t step_count = 0;
// Triangle wave will start as increasing,
// then switch to decreasing (0)
uint8_t triange_increasing = 1;


// dont look at this
// This was generated using nodejs in the terminal
// with the following sequence of entries
/**
 * const sin_100 = [];
 * for (let i = 0; i < 300; i++) {
 *     sin_100.push(Math.round((4095/2)*(Math.sin(2*Math.PI * i / 300)+1)));
 * }
 * sin_100.join(", ");
 */
// Which prints the LUT to the console, which I copied over here
const uint16_t SIN_LUT[STEPS_PER_PERIOD_MAX] = {
    2048, 2090, 2133, 2176, 2219, 2262, 2304, 2347, 2389, 2431, 2473, 
    2515, 2557, 2598, 2639, 2680, 2721, 2761, 2801, 2841, 2880, 
    2919, 2958, 2996, 3034, 3071, 3108, 3145, 3181, 3216, 3251, 
    3285, 3319, 3353, 3385, 3418, 3449, 3480, 3510, 3540, 3569, 
    3597, 3625, 3652, 3678, 3704, 3729, 3753, 3776, 3799, 3821, 
    3842, 3862, 3881, 3900, 3918, 3935, 3951, 3967, 3981, 3995, 
    4008, 4020, 4031, 4041, 4050, 4059, 4066, 4073, 4079, 4084, 
    4088, 4091, 4093, 4095, 4095, 4095, 4093, 4091, 4088, 4084, 
    4079, 4073, 4066, 4059, 4050, 4041, 4031, 4020, 4008, 3995, 
    3981, 3967, 3951, 3935, 3918, 3900, 3881, 3862, 3842, 3821, 
    3799, 3776, 3753, 3729, 3704, 3678, 3652, 3625, 3597, 3569, 
    3540, 3510, 3480, 3449, 3418, 3385, 3353, 3319, 3285, 3251, 
    3216, 3181, 3145, 3108, 3071, 3034, 2996, 2958, 2919, 2880, 
    2841, 2801, 2761, 2721, 2680, 2639, 2598, 2557, 2515, 2473, 
    2431, 2389, 2347, 2304, 2262, 2219, 2176, 2133, 2090, 2048, 
    2005, 1962, 1919, 1876, 1833, 1791, 1748, 1706, 1664, 1622, 
    1580, 1538, 1497, 1456, 1415, 1374, 1334, 1294, 1254, 1215, 
    1176, 1137, 1099, 1061, 1024, 987, 950, 914, 879, 844, 810, 
    776, 742, 710, 677, 646, 615, 585, 555, 526, 498, 470, 443, 
    417, 391, 366, 342, 319, 296, 274, 253, 233, 214, 195, 177, 
    160, 144, 128, 114, 100, 87, 75, 64, 54, 45, 36, 29, 22, 16, 
    11, 7, 4, 2, 0, 0, 0, 2, 4, 7, 11, 16, 22, 29, 36, 45, 54, 
    64, 75, 87, 100, 114, 128, 144, 160, 177, 195, 214, 233, 253, 
    274, 296, 319, 342, 366, 391, 417, 443, 470, 498, 526, 555, 
    585, 615, 646, 677, 710, 742, 776, 810, 844, 879, 914, 950, 
    987, 1024, 1061, 1099, 1137, 1176, 1215, 1254, 1294, 1334, 
    1374, 1415, 1456, 1497, 1538, 1580, 1622, 1664, 1706, 1748, 
    1791, 1833, 1876, 1919, 1962, 2005
}


void FUNC_init() {
    set_freq(ONE);
    set_wave(SQUARE);
    set_duty(FIFTY);
    step_count = 0;
    configure_square();
}


void step_output() {
    // Extra gaurd clause just to make sure nothing weird happens
    if (wave_type == SQUARE) return;

    // Otherwise, step based on function type
    if (wave_type == SAW) step_saw();
    else if (wave_type == SIN) step_sin();
    else if (wave_type == TRIANGLE) step_triangle();
}

// Saw is simple with a step size of
// (3300) / (STEPS_PER_PERIOD_MAX / (wave_freq / 100))
// meaning, say, for 100Hz, 3300 / (1000 / (100 / 100)) = 3300 / 1000
// for 500Hz, 3300 / (1000 / (500 / 100)) = 3300 / 200
void step_saw() {

}

void step_sin() {
    // Check to see if step has overflowed
    // Since the max period step count is divisible by
    // 1, 2, 3, 4, and 5, we can simply reset step count to 0
    // since all (should) overflow to 1000
    if (step_count >= STEPS_PER_PERIOD_MAX) step_count = 0;

    // Write current LUT position to dac output
    DAC_write(SIN_LUT[step_count]);

    // Index step size will be based on the frequency
    // 100Hz will have a step size of 1, and get all 1000 values in one period
    // 500Hz will have a step size of 5, and get only 200 values in one period
    uint8_t step_size = wave_freq / 100;
    // Increment steps for next iteration
    step_count += step_size;
}

void step_triangle() {

}

// Configures the ARR and 
void configure_square() {
    // Disabled counter while configuration takes place
    TIM2->CR1 &= ~(TIM_CR1_CEN);
    TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);

    // Reset counter to ensure normal behavior on restart
    TIM2->CNT = 0;
    // (80MHz / freq) - 1 ;  yields ARR value
    TIM2->ARR = (80000000 / wave_freq) - 1;
    // Duty cycle setup
    TIM2->CCR1 = ((TIM2->ARR + 1) * duty_cycle) / 100;

    // Clear interrupt flags
    TIM2->SR &= ~(TIM_SR_UIF | TIM_SR_CC1IF);

    step_count = 0;
    
    // Re-enable in interrupt enable reg
    TIM2->DIER |= (TIM_DIER_CC1IE | TIM_DIER_UIE);
    TIM2->CR1 |= TIM_CR1_CEN;
}

void configure_other() {
    // Disabled counter while configuration takes place
    TIM2->CR1 &= ~(TIM_CR1_CEN);
    TIM2->DIER &= ~(TIM_DIER_CC1IE | TIM_DIER_UIE);

    // Reset counter to ensure normal behavior on restart
    TIM2->CNT = 0;

    uint16_t step_count = (STEPS_PER_PERIOD_MAX / (wave_freq / 100));

    // (80MHz / freq*step_count) - 1 ;  yields ARR value
    TIM2->ARR = (80000000 / (wave_freq * step_count)) - 1;
    // No CCR for wave types that are not square waves

    // Clear interrupt flags
    TIM2->SR &= ~(TIM_SR_UIF);

    step_count = 0;

    // Re-enable in interrupt enable reg
    TIM2->DIER |= (TIM_DIER_UIE);
    TIM2->CR1 |= TIM_CR1_CEN;
}


void set_wave(enum WAVE_TYPE wave) {
    switch (wave) {
        case SIN:
        case SAW:
        case TRIANGLE:
            configure_other();
            break;
        case SQUARE:
            configure_square();
            break;
    }

    wave_type = wave;
}

void set_freq(enum WAVE_FREQ freq) {
    wave_freq = freq;
}

void set_duty(enum SQUARE_DUTY duty) {
    // Not NECESSARY, but if it isn't a square wave, 
    // duty cycle does nothing, so just return
    if (wave_type != SQUARE) return;

    duty_cycle = duty;
}