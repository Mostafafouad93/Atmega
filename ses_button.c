/*
 ***************************************************************************
 ses_button V1 - Copyright (C) 2018 MOSTAFA HASSAN & HAZEM ABAZA.
 ***************************************************************************
 This file is part of the SES_TUHH library.

 ses_button is a library that allows triggering the button pressed to our board (Joystick or Rotary).
 It has functions to allow its configuration as(external interrupt or deboncing), to check which button action is
 captured by the external (I/O) pins,also there is an ISR to fire an event by using the callback variables to
 perform a certain action debending on the button pressed.Fnally for this library file we have check_state method
 for polling the button state within a timer interrupt to avoid any bouncing behavior.


 ***************************************************************************
 */

/* INCLUDES ******************************************************************/
#include "ses_button.h"
#include "ses_common.h"
#include <avr/interrupt.h>
#include "ses_led.h"
#include <stdbool.h>
#include "ses_timer.h"
#include "ses_lcd.h"

/* DEFINES & MACROS **********************************************************/

#define JOYSTICK_PIN         	         7
#define ROTARY_ENCODER_PIN               6

#define PIN_CHANGE_INTERRUPT_REGISTER    PCICR
#define PIN_Change_Interrupt_Enable_0    PCIE0
#define PIN_CHANGE_MASK_REGISTER_0       PCMSK0
#define PIN_CHANGE_ENABLE_MASK_JOYSTICK  7
#define PIN_CHANGE_ENABLE_MASK_ROTARY    6
#define EXTERNAL_INTERRUPT_FLAG_REGISTER EIFR
#define BUTTON_NUM_DEBOUNCE_CHECKS       5

/* GLOBAL VARIABLES *******************************************************/

volatile pButtonCallback myRotaryCallback = NULL; /* pointer to a fuction to be called when the rotaty is pressed */
volatile pButtonCallback myJoystickCallback = NULL; /* pointer to a fuction to be called when the joystick is pressed */

bool externalInterrupt = false; /* a flag to give the user the choise the usege of the external interrupt or not */

/* FUNCTION DEFINITION *******************************************************/

void button_init(bool debouncing) {

	DDR_REGISTER(PORTB) &= ~(1 << JOYSTICK_PIN); /* declare PINB7 is an input*/
	PORTB |= (1 << JOYSTICK_PIN); /* set the pull up resistor of the Joystick*/
	DDR_REGISTER(PORTB) &= ~(1 << ROTARY_ENCODER_PIN); /* declare PINB6 is an input*/
	PORTB |= (1 << ROTARY_ENCODER_PIN); /* set the pull up resistor of the Rotary*/

	if (debouncing) {
		timer1_start();
		pTimerCallback callback = &button_checkState;
		timer1_setCallback(callback);
	} else {

		/*
		 *  disable the mask first to prevent accidentall calls during setups
		 */

		PIN_CHANGE_MASK_REGISTER_0 &= ~(1 << PIN_CHANGE_ENABLE_MASK_JOYSTICK);
		PIN_CHANGE_MASK_REGISTER_0 &= ~(1 << PIN_CHANGE_ENABLE_MASK_ROTARY);

		EXTERNAL_INTERRUPT_FLAG_REGISTER |= (1
				<< PIN_CHANGE_ENABLE_MASK_JOYSTICK); /* clear pending interrupt for joystick*/
		EXTERNAL_INTERRUPT_FLAG_REGISTER |=
				(1 << PIN_CHANGE_ENABLE_MASK_ROTARY); /* clear pending interrupt for rotaty*/

		if (externalInterrupt) {

			PIN_CHANGE_INTERRUPT_REGISTER |=
					(1 << PIN_Change_Interrupt_Enable_0);
			PIN_CHANGE_MASK_REGISTER_0 |=
					(1 << PIN_CHANGE_ENABLE_MASK_JOYSTICK);
			PIN_CHANGE_MASK_REGISTER_0 |= (1 << PIN_CHANGE_ENABLE_MASK_ROTARY);

		}

	}
	/*
	 * set the glopal interrupt if the button is initialized
	 */
	sei();
}

bool button_isJoystickPressed(void) {

	/* check that the joystick pin is grounded and
	 *  then the input can be read as logic low level via PINB
	 */

	if ((PIN_REGISTER(PORTB) & 1 << JOYSTICK_PIN) == 0) {
		return true;
	} else {
		return false;
	}
}
bool button_isRotaryPressed(void) {

	/* check that the rotary pin is grounded and
	 * then the input can be read as logic low level via PINB
	 */

	if (!(PIN_REGISTER(PORTB) & 1 << ROTARY_ENCODER_PIN) == 0) {
		return true;
	} else {
		return false;
	}
}

ISR(PCINT0_vect) {

	/* the if conditions is to make sure that a button callback is only
	 *  executed if a valid callback was set,
	 * the mask register contains a 1 and to check if one of the button values cheanged
	 */

	if (button_isJoystickPressed() && (myJoystickCallback != NULL)
			&& (PIN_CHANGE_MASK_REGISTER_0
					& 1 << PIN_CHANGE_ENABLE_MASK_JOYSTICK) != 0) {
		myJoystickCallback(NULL);

	} else if (button_isRotaryPressed() && (myRotaryCallback != NULL)
			&& (PIN_CHANGE_MASK_REGISTER_0 & 1 << PIN_CHANGE_ENABLE_MASK_ROTARY)
					!= 0) {
		myRotaryCallback(NULL);
	}

}

void button_setRotaryButtonCallback(pButtonCallback callback) {
	myRotaryCallback = callback;
}

void button_setJoystickButtonCallback(pButtonCallback callback) {
	myJoystickCallback = callback;

}

void button_checkState(void* checkP) {

	static uint8_t state[BUTTON_NUM_DEBOUNCE_CHECKS] = { };
	static uint8_t index = 0;
	static uint8_t debouncedState = 0;
	uint8_t lastDebouncedState = debouncedState;

	/*
	 * each bit in every state byte represents one button
	 */
	state[index] = 0;

	if (button_isJoystickPressed()) {
		state[index] |= 1;
	}

	if (button_isRotaryPressed()) {
		state[index] |= 2;
	}

	index++;

	if (index == BUTTON_NUM_DEBOUNCE_CHECKS) {
		index = 0;
	}

	/* init compare value and compare with ALL reads, only if
	 * we read BUTTON_NUM_DEBOUNCE_CHECKS consistent "1" in the state
	 * array, the button at this position is considered pressed
	 */

	uint8_t j = 0xFF;

	for (uint8_t i = 0; i < BUTTON_NUM_DEBOUNCE_CHECKS; i++) {
		j = j & state[i];
	}
	debouncedState = j;

	/* to make sure that the last state
	 * is different from the current state and that
	 *  the array started to count again form the first to trigger the new state
	 */
	if (lastDebouncedState != debouncedState) {

		if (debouncedState == 1) {
			myJoystickCallback(NULL);

		}
		if (debouncedState == 2) {
			myRotaryCallback(NULL);

		}

		if (debouncedState == 3) {
			myJoystickCallback(NULL);
			myRotaryCallback(NULL);
		}

		lastDebouncedState = debouncedState;
	}
}
