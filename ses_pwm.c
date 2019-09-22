/*
 ***************************************************************************
 pwm V1 - Copyright (C) 2018 MOSTAFA HASSAN & HAZEM ABAZA.
 ***************************************************************************
 This file is part of the SES_TUHH library.

 The current delivered to the motor can be controlled by a transistor whose basis is connected to PORTG5.
 This pin is also the OC0B pin, which can be changed by the timer 0 hardware, e.g. on compare match.
 Thus, a PWM signal with a fixed frequency, but adjustable duty cycle can be used to control the motor
 speed.

 ***************************************************************************
 */

/* INCLUDES ******************************************************************/

#include "ses_pwm.h"
#include "ses_timer.h"

/*-----------------------------------------------------------
 * Implementation of functions defined in scheduler.h *
 *----------------------------------------------------------*/

void pwm_init(void) {

	DDR_REGISTER(PORTG) |= (1 << PG5);    //set the pin as O/P

	timer0_start();
}

void pwm_setDutyCycle(uint8_t dutyCycle) {

	OCR0B = dutyCycle;
}
