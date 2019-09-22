#ifndef SES_TIMER_H_
#define SES_TIMER_H_

/*INCLUDES *******************************************************************/

#include "ses_common.h"



/*PROTOTYPES *****************************************************************/


/**type of function pointer used as timer callback
 */
typedef void (*pTimerCallback)(void*);


/**
 * Sets a function to be called when the timer fires. If NULL is
 * passed, no callback is executed when the timer fires.
 *
 * @param cb  valid pointer to callback function
 */
void timer2_setCallback(pTimerCallback);

/**
 * Starts hardware timer 2 of MCU with a period
 * of 1 ms.
 */
void timer2_start();


/**
 * Stops timer 2.
 */
void timer2_stop();


/**
 * Sets a function to be called when the timer fires.
 *
 * @param cb  pointer to the callback function; if NULL, no callback
 *            will be executed.
 */
void timer1_setCallback(pTimerCallback);


/**
 * Start timer 1 of MCU to trigger on compare match every 5ms.
 */
void timer1_start();


/**
 * Stops timer 1 of the MCU if it is no longer needed.
 */
void timer1_stop();

/**
 * Start timer 0
 */

void timer0_start(void);

/**
 * Stops timer 0.
 */
void timer0_stop(void);

/**
 * Start timer 5.
 */
void timer5_start(void);

/**
 * Stops timer 5.
 */
void timer5_stop(void);


/**
 * Sets a function to be called when the timer fires.
 *
 * @param cb  pointer to the callback function; if NULL, no callback
 *            will be executed.
 */

void timer5_setCallback(pTimerCallback);



#endif /* SES_TIMER_H_ */
