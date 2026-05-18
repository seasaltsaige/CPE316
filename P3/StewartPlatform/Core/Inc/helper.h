#ifndef __HELPER_H
#define __HELPER_H
#include "stewart_controller.h"
#include <stdint.h>

typedef enum {
  HOME_ENDSTOP,
  EXTENSION_ENDSTOP,
} ENDSTOP_TYPE;

void handle_endstop(Stepper_t *m, ENDSTOP_TYPE ext_type, uint64_t pending_irq_flag);
void delay_stepper_ms(Stepper_t *m, uint32_t ms);


#endif