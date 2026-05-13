#ifndef FUNC_GEN_H
#define FUNC_GEN_H

#include <stdint.h>

enum WAVE_FREQ {
    ONE,
    TWO,
    THREE,
    FOUR,
    FIVE
} WAVE_FREQ;


enum WAVE_TYPE {
    SQUARE,
    SAW,
    TRIANGLE,
    SIN,
} WAVE_TYPE;

#define STEPS_PER_PERIOD_MAX 60

extern enum WAVE_FREQ wave_freq;
extern enum WAVE_TYPE wave_type;

extern uint16_t dac_output;
extern uint16_t step_count;

extern const uint16_t SIN_LUT[1000];

// void step_output();

void square();
void step_saw();
void step_triangle();
void step_sin();


#endif