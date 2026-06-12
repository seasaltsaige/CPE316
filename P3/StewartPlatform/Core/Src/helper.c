#include "helper.h"
#include "stewart_controller.h"
#include <stdint.h>


void handle_endstop(Stepper_t *m, ENDSTOP_TYPE ext_type, uint64_t pending_irq_flag) {
  // Clear interrupt flag
  uint64_t exti_flag = 0;
  if (ext_type == HOME_ENDSTOP)
    exti_flag = m->EXTI_home_flag;
  else
    exti_flag = m->EXTI_limit_flag;
  
  EXTI->IMR1 &= ~(exti_flag);

  // Clear pending flag
  EXTI->PR1 = pending_irq_flag;


  if (ext_type == HOME_ENDSTOP) {
    switch (m->motor_state) {
      case NORMAL_RUNNING:
        // TODO: If endstops are hit while normal operation is happening
        // we will likely need to re-home
        // back off from endstop (depending on which one)
        // then switch state, then call the home function agian
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = IDLE;
        m->steps_current = 0;
      
        break;

      case HOMING_FAST:
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = HOMING_FAST_LIMIT;
        m->steps_current = 0;
        break;

      case HOMING_SLOW:
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = HOMING_SLOW_LIMIT;
        m->steps_current = 0;
        break;

      case HOMING_SLOW_BACKOFF:
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = HOMING_SLOW_BACKOFF_DONE;
        m->steps_current = 0;
        break;

      default:
        // If none of the above cases happened, re-enable the interrupt
        EXTI->IMR1 |= (exti_flag);
        break;
    }
  } else {
    switch (m->motor_state) {
      case NORMAL_RUNNING:  

        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = IDLE;
        m->steps_current = 0;

        break;

      case EXTENSION_FAST:
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = EXTENSION_FAST_LIMIT;
        break;

      case EXTENSION_SLOW:
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = EXTENSION_SLOW_LIMIT;
        break;

      case EXTENSION_SLOW_BACKOFF:
        *(m->CCR) = 0;
        m->timer->CR1 &= ~TIM_CR1_CEN;
        m->timer->CNT = 0;
        m->motor_state = EXTENSION_SLOW_BACKOFF_DONE;
        m->MAX_STEPS = m->steps_current;
        break;

      default:
        // If none of the above cases happened, re-enable the interrupt
        EXTI->IMR1 |= (exti_flag);
        break;
    }
  }

}

void delay_stepper_ms(Stepper_t *m, uint32_t ms) {
  m->delay_time_ms = ms;
  m->motor_state = DELAY;
}

void read_endstops_pwr_up(Stepper_t *m) {
  // END STOP PORTS SAME AS MAIN PORT
  uint8_t home_active = (m->dir_port->IDR & m->home_pin ? 1 : 0);
  uint8_t limit_active = (m->dir_port->IDR & m->limit_pin ? 1 : 0);

  // If one or the other is active
  // (hopefully not both ever)
  if (home_active) {
    // extend by backoff distance
    stepper_move_const_vel(m, 2 * HOMING_BACKOFF_STEPS, 5, NORMAL_RUNNING);
    
  } else if (limit_active) {
    // retract by backoff distance
    stepper_move_const_vel(m, -2 * HOMING_BACKOFF_STEPS, 5, NORMAL_RUNNING);
  }
}


uint8_t poll_motors_busy() {
  return (motors[0].motor_state != IDLE || motors[1].motor_state != IDLE || 
          motors[2].motor_state != IDLE || motors[3].motor_state != IDLE ||
          motors[4].motor_state != IDLE || motors[5].motor_state != IDLE) ? 1 : 0;
}