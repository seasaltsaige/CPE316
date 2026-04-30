#ifndef DAC_H
#define DAC_H


#include <stdint.h>

void DAC_init();
void DAC_write();

unt16_t DAC_volt_conv();

#endif