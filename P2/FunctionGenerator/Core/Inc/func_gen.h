#ifndef FUNC_GEN_H
#define FUNC_GEN_H

#include <stdint.h>

// Defines the frequency at which the interrupts should be generated.
// For all but the square wave, this behaves more or less the same
// For the square wave, we can just use the CCR1 and ARR to create a duty cycled wave
// All others can use
// ARR or CCR1 = fclk / (freq * step_count)
// as the reset and general handling is handled by the func_gen
// not the ARR for resetting the dac output
enum WAVE_FREQ : uint16_t  {
    ONE = 100,
    TWO = 200,
    THREE = 300,
    FOUR = 400,
    FIVE = 500
};

// Simple enum to handle 
// for saw, triangle, and sin,
// the ARR will be used to generate the interrupt
// at which the step is increased/set to the next value
// the CCR can be used instead, but the ARR just makes more sense
enum WAVE_TYPE : uint8_t  {
    SQUARE,
    SAW,
    TRIANGLE,
    SIN,
};

// Only applies to the square wave
// Used for selecting duty cycle value in CCR1 and ARR
enum SQUARE_DUTY : uint8_t {
    TEN = 10,
    TWENTY = 20,
    THIRTY = 30,
    FOURTY = 40,
    FIFTY = 50,
    SIXTY = 60,
    SEVENTY = 70,
    EIGHTY = 80,
    NINETY = 90, 
};

// Got this through some ... hard thought
// Given the 80MHz clock, and a settling time of 4.5us on the dac,
// f = 1/4.5us ~= 222.222k samps
// 222,000Hz / 100Hz = 2220 theoretical max sample speed, though this
// wont really give the dac time to settle, and hold the output.
// Went with 1000 samples per period (at 100Hz) to give plenty of room
// for error
#define STEPS_PER_PERIOD_MAX 1000

// Keep track of the current state
extern enum WAVE_FREQ wave_freq;
extern enum WAVE_TYPE wave_type;
extern enum SQUARE_DUTY duty_cycle;

extern volatile uint16_t dac_output_mv;
extern volatile uint16_t step_count;
extern volatile uint8_t triangle_increasing;

// Lookup table used for the SINE wave
extern const uint16_t SIN_LUT[STEPS_PER_PERIOD_MAX];

void FUNC_init();

// Used in the interrupt routine to step the output,
// Square waves are a special case and handled independently
void step_output();

void configure_square();
void configure_other();

void step_saw();
void step_triangle();
void step_sin();

void set_wave(enum WAVE_TYPE wave);
void set_duty(enum SQUARE_DUTY duty);
void set_freq(enum WAVE_FREQ freq);


#endif