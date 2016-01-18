/***********************************************************************

Module
    timer.h

Description
    header file for timer module
    
Notes
    Requires Board Connections as noted in the lab writeup
    
***********************************************************************/

#ifndef TIMER_H
#define TIMER_H

#define GAME_TIMER 0
#define FLAG_CAPTURED_TIMER 1
#define LEFT_ENCODER_TIMER 2
#define RIGHT_ENCODER_TIMER 3
//#define BEACON_CONTROL_TIMER 4
#define GREEN_TAPE_TIMER 4
#define WARNING_TIMER 5
#define TOP_LEFT_BEACON_TIMER 6
#define TOP_RIGHT_BEACON_TIMER 7
#define FLAG_LEFT_BEACON_TIMER 8
#define FLAG_RIGHT_BEACON_TIMER 9

#define GAME_TIME 120000
#define WARNING_TIME 90000

void InitPorts( void);
void InitTimer(void);
void StartTimer(char TimerNum, unsigned long ms_duration);
char IsTimerExpired(char TimerNum);
unsigned long GetAbsoluteTime(void);

#endif

/*-------------------------- End of file -----------------------------*/

