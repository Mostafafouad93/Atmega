#ifndef SES_MOTORFREQUENCY_H_
#define SES_MOTORFREQUENCY_H_
/*INCLUDES *******************************************************************/
#include <inttypes.h>
#include <avr/io.h>
#include "ses_common.h"

/* FUNCTION PROTOTYPES *******************************************************/
void motorFrequency_init();
uint16_t motorFrequency_getRecent();
uint16_t motorFrequency_getMedian();
void motorSet(bool);

#endif /* SES_MOTORFREQUENCY_H_ */
