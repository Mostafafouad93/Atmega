/*
 * ses_pwm.h
 *
 *  Created on: 16 Jun 2018
 *      Author: Hassan
 */

#ifndef SES_PWM_H_
#define SES_PWM_H_

/*INCLUDES *******************************************************************/
#include "ses_common.h"

/*PROTOTYPES *****************************************************************/

void pwm_init(void);

void pwm_setDutyCycle(uint8_t dutyCycle);


#endif /* SES_PWM_H_ */
