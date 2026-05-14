#include "func_gen.h"
#include "DAC.h"
#include "stm32l476xx.h"
#include <stdint.h>

enum WAVE_FREQ wave_freq = ONE;
enum WAVE_TYPE wave_type = SQUARE;
enum SQUARE_DUTY duty_cycle = FIFTY;

volatile uint16_t dac_output_mv = 0;
volatile uint16_t step_count = 0;
// Triangle wave will start as increasing,
// then switch to decreasing (0)
volatile uint8_t triangle_increasing = 1;


// dont look at this
// This was generated using nodejs in the terminal
// with the following sequence of entries
/**
 * const sin_100 = [];
 * for (let i = 0; i < 1000; i++) {
 *     sin_100.push(Math.round((3000/2)*(Math.sin(2*Math.PI * i / 1000)+1)));
 * }
 * sin_100.join(", ");
 */
// Which prints the LUT to the console, which I copied over here
const uint16_t SIN_LUT[STEPS_PER_PERIOD_MAX] = {
1500, 1509, 1519, 1528, 1538, 1547, 1557, 1566, 1575, 1585, 1594, 1604, 1613, 1622, 1632, 1641, 1651, 1660, 1669, 1679, 1688, 1697, 1707, 1716, 1725, 1735, 1744, 1753, 1763, 1772, 1781, 1790, 1800, 1809, 1818, 1827, 1836, 1846, 1855, 1864, 1873, 1882, 1891, 1900, 1909, 1918, 1928, 1937, 1946, 1955, 1964, 1972, 1981, 1990, 1999, 2008, 2017, 2026, 2035, 2043, 2052, 2061, 2070, 2078, 2087, 2096, 2104, 2113, 2122, 2130, 2139, 2147, 2156, 2164, 2173, 2181, 2189, 2198, 2206, 2214, 2223, 2231, 2239, 2247, 2255, 2264, 2272, 2280, 2288, 2296, 2304, 2312, 2320, 2327, 2335, 2343, 2351, 2359, 2366, 2374, 2382, 2389, 2397, 2404, 2412, 2419, 2427, 2434, 2442, 2449, 2456, 2463, 2471, 2478, 2485, 2492, 2499, 2506, 2513, 2520, 2527, 2534, 2540, 2547, 2554, 2561, 2567, 2574, 2580, 2587, 2593, 2600, 2606, 2613, 2619, 2625, 2631, 2638, 2644, 2650, 2656, 2662, 2668, 2674, 2679, 2685, 2691, 2697, 2702, 2708, 2714, 2719, 2725, 2730, 2735, 2741, 2746, 2751, 2756, 2761, 2766, 2772, 2776, 2781, 2786, 2791, 2796, 2801, 2805, 2810, 2814, 2819, 2823, 2828, 2832, 2837, 2841, 2845, 2849, 2853, 2857, 2861, 2865, 2869, 2873, 2877, 2880, 2884, 2888, 2891, 2895, 2898, 2901, 2905, 2908, 2911, 2914, 2918, 2921, 2924, 2927, 2929, 2932, 2935, 2938, 2940, 2943, 2946, 2948, 2951, 2953, 2955, 2957, 2960, 2962, 2964, 2966, 2968, 2970, 2972, 2973, 2975, 2977, 2978, 2980, 2982, 2983, 2984, 2986, 2987, 2988, 2989, 2990, 2991, 2992, 2993, 2994, 2995, 2996, 2996, 2997, 2998, 2998, 2999, 2999, 2999, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 3000, 2999, 2999, 2999, 2998, 2998, 2997, 2996, 2996, 2995, 2994, 2993, 2992, 2991, 2990, 2989, 2988, 2987, 2986, 2984, 2983, 2982, 2980, 2978, 2977, 2975, 2973, 2972, 2970, 2968, 2966, 2964, 2962, 2960, 2957, 2955, 2953, 2951, 2948, 2946, 2943, 2940, 2938, 2935, 2932, 2929, 2927, 2924, 2921, 2918, 2914, 2911, 2908, 2905, 2901, 2898, 2895, 2891, 2888, 2884, 2880, 2877, 2873, 2869, 2865, 2861, 2857, 2853, 2849, 2845, 2841, 2837, 2832, 2828, 2823, 2819, 2814, 2810, 2805, 2801, 2796, 2791, 2786, 2781, 2776, 2772, 2766, 2761, 2756, 2751, 2746, 2741, 2735, 2730, 2725, 2719, 2714, 2708, 2702, 2697, 2691, 2685, 2679, 2674, 2668, 2662, 2656, 2650, 2644, 2638, 2631, 2625, 2619, 2613, 2606, 2600, 2593, 2587, 2580, 2574, 2567, 2561, 2554, 2547, 2540, 2534, 2527, 2520, 2513, 2506, 2499, 2492, 2485, 2478, 2471, 2463, 2456, 2449, 2442, 2434, 2427, 2419, 2412, 2404, 2397, 2389, 2382, 2374, 2366, 2359, 2351, 2343, 2335, 2327, 2320, 2312, 2304, 2296, 2288, 2280, 2272, 2264, 2255, 2247, 2239, 2231, 2223, 2214, 2206, 2198, 2189, 2181, 2173, 2164, 2156, 2147, 2139, 2130, 2122, 2113, 2104, 2096, 2087, 2078, 2070, 2061, 2052, 2043, 2035, 2026, 2017, 2008, 1999, 1990, 1981, 1972, 1964, 1955, 1946, 1937, 1928, 1918, 1909, 1900, 1891, 1882, 1873, 1864, 1855, 1846, 1836, 1827, 1818, 1809, 1800, 1790, 1781, 1772, 1763, 1753, 1744, 1735, 1725, 1716, 1707, 1697, 1688, 1679, 1669, 1660, 1651, 1641, 1632, 1622, 1613, 1604, 1594, 1585, 1575, 1566, 1557, 1547, 1538, 1528, 1519, 1509, 1500, 1491, 1481, 1472, 1462, 1453, 1443, 1434, 1425, 1415, 1406, 1396, 1387, 1378, 1368, 1359, 1349, 1340, 1331, 1321, 1312, 1303, 1293, 1284, 1275, 1265, 1256, 1247, 1237, 1228, 1219, 1210, 1200, 1191, 1182, 1173, 1164, 1154, 1145, 1136, 1127, 1118, 1109, 1100, 1091, 1082, 1072, 1063, 1054, 1045, 1036, 1028, 1019, 1010, 1001, 992, 983, 974, 965, 957, 948, 939, 930, 922, 913, 904, 896, 887, 878, 870, 861, 853, 844, 836, 827, 819, 811, 802, 794, 786, 777, 769, 761, 753, 745, 736, 728, 720, 712, 704, 696, 688, 680, 673, 665, 657, 649, 641, 634, 626, 618, 611, 603, 596, 588, 581, 573, 566, 558, 551, 544, 537, 529, 522, 515, 508, 501, 494, 487, 480, 473, 466, 460, 453, 446, 439, 433, 426, 420, 413, 407, 400, 394, 387, 381, 375, 369, 362, 356, 350, 344, 338, 332, 326, 321, 315, 309, 303, 298, 292, 286, 281, 275, 270, 265, 259, 254, 249, 244, 239, 234, 228, 224, 219, 214, 209, 204, 199, 195, 190, 186, 181, 177, 172, 168, 163, 159, 155, 151, 147, 143, 139, 135, 131, 127, 123, 120, 116, 112, 109, 105, 102, 99, 95, 92, 89, 86, 82, 79, 76, 73, 71, 68, 65, 62, 60, 57, 54, 52, 49, 47, 45, 43, 40, 38, 36, 34, 32, 30, 28, 27, 25, 23, 22, 20, 18, 17, 16, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4, 4, 3, 2, 2, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 2, 2, 3, 4, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 18, 20, 22, 23, 25, 27, 28, 30, 32, 34, 36, 38, 40, 43, 45, 47, 49, 52, 54, 57, 60, 62, 65, 68, 71, 73, 76, 79, 82, 86, 89, 92, 95, 99, 102, 105, 109, 112, 116, 120, 123, 127, 131, 135, 139, 143, 147, 151, 155, 159, 163, 168, 172, 177, 181, 186, 190, 195, 199, 204, 209, 214, 219, 224, 228, 234, 239, 244, 249, 254, 259, 265, 270, 275, 281, 286, 292, 298, 303, 309, 315, 321, 326, 332, 338, 344, 350, 356, 362, 369, 375, 381, 387, 394, 400, 407, 413, 420, 426, 433, 439, 446, 453, 460, 466, 473, 480, 487, 494, 501, 508, 515, 522, 529, 537, 544, 551, 558, 566, 573, 581, 588, 596, 603, 611, 618, 626, 634, 641, 649, 657, 665, 673, 680, 688, 696, 704, 712, 720, 728, 736, 745, 753, 761, 769, 777, 786, 794, 802, 811, 819, 827, 836, 844, 853, 861, 870, 878, 887, 896, 904, 913, 922, 930, 939, 948, 957, 965, 974, 983, 992, 1001, 1010, 1019, 1028, 1036, 1045, 1054, 1063, 1072, 1082, 1091, 1100, 1109, 1118, 1127, 1136, 1145, 1154, 1164, 1173, 1182, 1191, 1200, 1210, 1219, 1228, 1237, 1247, 1256, 1265, 1275, 1284, 1293, 1303, 1312, 1321, 1331, 1340, 1349, 1359, 1368, 1378, 1387, 1396, 1406, 1415, 1425, 1434, 1443, 1453, 1462, 1472, 1481, 1491
};


// Init function generator 
// at 100Hz, 50% duty cycle
// square wave.
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

// increment through the number of steps based on the frequency
// 1000 steps at 100hz, 500 steps at 200hz, etc
// calculate the voltage based on the current step count
// to avoid compounding errors from voltage math.
void step_saw() {
    uint16_t freq_step_mod = wave_freq / 100;
    if (step_count >= (STEPS_PER_PERIOD_MAX / (freq_step_mod))) step_count = 0;
    uint16_t output_voltage_mv = step_count * (VOLT_MAX / (STEPS_PER_PERIOD_MAX / (freq_step_mod)));
    DAC_write(DAC_volt_conv(output_voltage_mv));
    step_count += 1;
}

void step_sin() {
    // Check to see if step has overflowed
    // Since the max period step count is divisible by
    // 1, 2, 3, 4, and 5, we can simply reset step count to 0
    // since all (should) overflow to 1000
    if (step_count >= STEPS_PER_PERIOD_MAX) step_count = 0;

    // Write current LUT position to dac output
    DAC_write(DAC_volt_conv(SIN_LUT[step_count]));

    // Index step size will be based on the frequency
    // 100Hz will have a step size of 1, and get all 1000 values in one period
    // 500Hz will have a step size of 5, and get only 200 values in one period
    uint8_t step_size = wave_freq / 100;
    // Increment steps for next iteration
    step_count += step_size;
}


// very similar to the saw waveform, except the step count is cut in
// half due to the need to go up, then back down.
// triangle_increasing keeps track of direction
void step_triangle() {
    uint16_t freq_step_mod = wave_freq / 100;
    if (step_count >= ((STEPS_PER_PERIOD_MAX / (freq_step_mod)) / 2)) triangle_increasing = 0;
    else if (step_count == 0) triangle_increasing = 1;

    uint16_t output_voltage_mv = step_count * (VOLT_MAX / (STEPS_PER_PERIOD_MAX / (freq_step_mod))) * 2;
    DAC_write(DAC_volt_conv(output_voltage_mv));

    if (triangle_increasing)
        step_count += 1;
    else 
        step_count -= 1;
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

    // Reset output stuff while timer is disabled so steps/output dont happen
    // and cause issues
    step_count = 0;
    dac_output_mv = 0;
    triangle_increasing = 1;
    
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

    uint32_t step_cnt = (STEPS_PER_PERIOD_MAX * (100));

    // (80MHz / freq*step_count) - 1 ;  yields ARR value
    TIM2->ARR = (80000000 / (step_cnt)) - 1;
    // No CCR for wave types that are not square waves

    // Clear interrupt flags
    TIM2->SR &= ~(TIM_SR_UIF);

    // Reset output stuff while timer is disabled so steps/output dont happen
    // and cause issues
    step_count = 0;
    dac_output_mv = 0;
    triangle_increasing = 1;

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
    if (wave_type == SQUARE) configure_square();
    else configure_other();
}

void set_duty(enum SQUARE_DUTY duty) {
    // Not NECESSARY, but if it isn't a square wave, 
    // duty cycle does nothing, so just return
    if (wave_type != SQUARE) return;
    duty_cycle = duty;

    configure_square();
}