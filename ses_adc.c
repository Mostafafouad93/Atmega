/*
 ***************************************************************************
 ses_adc V1 - Copyright (C) 2018 MOSTAFA HASSAN & HAZEM ABAZA.
 ***************************************************************************
 This file is part of the SES_TUHH library.

 ses_adc is a library converts an analog input voltage to a digital value. The ATmega128RFA1
 features an ADC with 10 bit resolution, yielding 210 distinct digital values which correspond to analog
 voltages2. The ADC is connected via a multiplexer to one of 8 external channels and a few internal
 channels.On the SES board, the following peripherals are connected to the ADC:

 * A temperature sensor at pin 2 of port F
 * A light sensor at pin 4 of port F
 * The joystick at pin 5 of port F
 * A microphone connected differentialy to the pins 0 and and 1 of port F


 ***************************************************************************
 */

/* INCLUDES ******************************************************************/
#include "ses_adc.h"
#include "ses_common.h"
#include "ses_lcd.h"

/* DEFINES & MACROS **********************************************************/
#define TEMP_SENSOR_PORT            PORTF
#define TEMP_SENSOR_PIN		        2

#define LIGHT_SENSOR_PORT           PORTF
#define LIGHT_SENSOR_PIN            4

#define JOYSTICK_PORT               PORTF
#define JOYSTICK_PIN                5

#define MICROPHONE_PORT             PORTF
#define MICROPHONE_PIN_0            0
#define MICROPHONE_PIN_1            1

#define POWER_REDUCTION_MODE        PRR0
#define POWER_REDUCTION_PRADC       0

#define ADMUX_REG                   ADMUX
#define ADLAR_BIT                   5
#define ADC_CHANNEL_SELECT_PIN      0
#define ADC_VREF_SRC                (1<<REFS1)|(1<<REFS0)

#define ADC_CONTROL_STATUS_REG       ADCSRA
#define ADC_Enable_PIN               ADEN
#define ADC_Auto_Trigger_Enable      ADATE
#define ADC_START_CONVERSION         ADSC

#define ADC_PRESCALER                (~(1<ADPS2))&((1<<ADPS1)|(1<ADPS0))

#define ADC_CHANNEL_SELECT_PIN       0

#define RIGHT_ADC                    200
#define UP_ADC                       400
#define LEFT_ADC                     600
#define DOWN_ADC                     800
#define NO_DIRECTION_ADC             1000
#define TOLRENCE                     50
#define ADMUX_CLEAR                  (0xf0)

#define ADC_TEMP_MAX                 40
#define ADC_TEMP_MIN                 20
#define ADC_TEMP_RAW_MAX             257
#define ADC_TEMP_RAW_MIN             482
#define ADC_TEMP_FACTOR              100

/* FUNCTION DEFINITION *******************************************************/

void adc_init(void) {

	DDR_REGISTER(PORTF) &= ~(1 << TEMP_SENSOR_PIN); /* configure data register for temperature sensor*/
	TEMP_SENSOR_PORT &= ~(1 << TEMP_SENSOR_PIN); /* Deactivate pull up resistor for temperature*/

	DDR_REGISTER(PORTF) &= ~(LIGHT_SENSOR_PIN); /* configure data register for light sensor*/
	LIGHT_SENSOR_PORT &= ~(LIGHT_SENSOR_PIN); /* Deactivate pull up resistor for light*/

	DDR_REGISTER(PORTF) &= ~(1 << JOYSTICK_PIN); /* configure data register for joystick sensor*/
	JOYSTICK_PORT &= ~(1 << JOYSTICK_PIN); /* Deactivate pull up resistor for joystick*/

	DDR_REGISTER(PORTF) &= ~(1 << MICROPHONE_PIN_0); /* configure data register for microphone sensor at pin 0*/
	MICROPHONE_PORT &= ~(1 << MICROPHONE_PIN_0); /* Deactivate pull up resistor for microphone*/

	DDR_REGISTER(PORTF) &= ~(1 << MICROPHONE_PIN_1); /* configure data register for microphone sensor at pin 1*/
	MICROPHONE_PORT &= ~(1 << MICROPHONE_PIN_1); /* Deactivate pull up resistor for microphone*/

	POWER_REDUCTION_MODE &= ~(1 << POWER_REDUCTION_PRADC);

	ADMUX_REG &= ~(1 << ADLAR_BIT); /* make the ADC result right adjusted*/
	ADMUX_REG |= ADC_VREF_SRC; /* set Ref.Volatage to Internal 1.6V Voltage Reference*/

	ADC_CONTROL_STATUS_REG |= ADC_PRESCALER; /* setting the pre scaler to 011 to devide the CPU clock by 8*/
	ADC_CONTROL_STATUS_REG &= ~(1 << ADC_Auto_Trigger_Enable); /* makeing sure that auto triggering is not selected.*/
	ADC_CONTROL_STATUS_REG |= (1 << ADC_Enable_PIN); /* enable the adc convertion.*/

}

uint16_t adc_read(uint8_t adc_channel) {

	uint16_t ADC_RESULT;
	uint8_t sreg; /* Variable to save the value of the SREG Register*/

	adc_init();

	if ((adc_channel > ADC_NUM) | (adc_channel < 0)) {
		return ADC_INVALID_CHANNEL;
	}

	else {

		ADMUX_REG &= ADMUX_CLEAR;

		ADMUX_REG |= (adc_channel << ADC_CHANNEL_SELECT_PIN); /*Select the ADC channels that will be used*/

		ADC_CONTROL_STATUS_REG |= (1 << ADC_START_CONVERSION); /*Starting the Conversion*/

		while ((ADC_CONTROL_STATUS_REG & (1 << ADC_START_CONVERSION)))
			;     // Waiting Until Conversion Terminates

		ADC_CONTROL_STATUS_REG &= ~(1 << ADC_START_CONVERSION);

		/* Reading the ADC
		 * register atomically
		 */

		sreg = SREG; /* Save global interrupt flag */

		cli();
		/* Disable interrupts */

		ADC_RESULT = ADC; /* Read the ADC Values*/

		SREG = sreg; /* Restore global interrupt flag */

		__asm__ __volatile__ ("" ::: "memory");  // to prevent any memory reorder

		return (ADC_RESULT);

	}
}

uint8_t adc_getJoystickDirection(void) {

	uint16_t joyStickValue = adc_read(ADC_JOYSTICK_CH);

	if (joyStickValue < RIGHT_ADC + TOLRENCE) {
		return RIGHT;
	} else if (joyStickValue < UP_ADC + TOLRENCE) {
		return UP;
	} else if (joyStickValue < LEFT_ADC + TOLRENCE) {
		return LEFT;
	} else if (joyStickValue < DOWN_ADC + TOLRENCE) {
		return DOWN;
	} else {
		return NO_DIRECTION;
	}
}

int16_t adc_getTemperature(void) {

	int16_t adc = adc_read(ADC_TEMP_CH);
	int16_t slope = (ADC_TEMP_MAX - ADC_TEMP_MIN)
			/ (ADC_TEMP_RAW_MAX - ADC_TEMP_RAW_MIN);
	int16_t offset = ADC_TEMP_MAX - (ADC_TEMP_RAW_MAX * slope);
	return (adc * slope + offset) / ADC_TEMP_FACTOR;
}

void adc_print(void) {
	lcd_clear();
	lcd_setCursor(0, 0);

	fprintf(lcdout, "*the Temp=%d C*", adc_getTemperature());

	lcd_setCursor(1, 1);

	//fprintf(lcdout, "**the light=%d**", adc_getLight());

	//lcd_setCursor(2, 2);

	fprintf(lcdout, "***position=%d", adc_getJoystickDirection());

	lcd_setCursor(3, 3);

	fprintf(lcdout, "*temp raw=%d", adc_read(ADC_TEMP_CH));



}
