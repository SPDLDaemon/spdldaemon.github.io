

#ifndef TIMER_H
#define TIMER_H

/*
 * All public member functions for two minute timer
 */

#define NOT_EXPIRED 0
#define EXPIRED 1
#define TIMER_DELAY 5000
#define MAX_COUNT 4500



/* public functions used in the state machine */
void startTimer(void);
char getTimer(void);


#endif