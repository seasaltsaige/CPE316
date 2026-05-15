#ifndef DAC_H
#define DAC_H

#include <stdint.h>

// 0111 => Write to dac, buffered, 1x gain, active mode
#define DAC_HEADER (0x7 << 12)

#define DAC_MAX 4096
// 3Vp-p
#define VOLT_MAX 3000

void DAC_init();
void DAC_write(uint16_t value);
uint16_t DAC_volt_conv(uint16_t voltage);


#endif