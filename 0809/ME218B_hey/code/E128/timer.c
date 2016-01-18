/***********************************************************************

Module
    timer.c

Description
    a module used for keeping track of absolute time
    
Notes
    Requires Board Connections as noted in the lab writeup

***********************************************************************/


/*------------------------- Include Files ----------------------------*/

#include <termio.h>
#include <stdio.h>
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"
#include "ADS12.h"
#include "timer.h"


/*------------------------ Module Defines ----------------------------*/

//#define TIMER_TEST

#define TICKS_PER_MS   187.5
#define HUNDREDMS      1875

#define NUM_TIMERS     10

typedef union {
  struct {
    unsigned int overflows;
    unsigned int thetime;
  } ByInts;
  unsigned long AsLong;
} LongByInts;

/*------------------------ Module Variables --------------------------*/
static LongByInts AbsoluteTime;
static LongByInts Timers[NUM_TIMERS];
static unsigned char TimerStarted[NUM_TIMERS];

/*------------------------ Private Function Prototypes ---------------*/

/*-------------------------- Module Code -----------------------------*/

/***********************************************************************
Function
    void InitPorts(void)
    
Description
    Initializes the AD Ports.    
***********************************************************************/

void InitPorts (void) {
  (void)ADS12_Init("IIIAAAAA");
}


/***********************************************************************
Function
    void InitTimer(void)
    
Description
    Initializes the Absolute Timer
    
***********************************************************************/

void InitTimer (void) {
   int i;
   
   // First, Timer 0 which is for the encoders
   TIM0_TSCR1 = _S12_TEN;				                  // turn the timer system on
   TIM0_TSCR2 = (_S12_PR2 | _S12_PR1 | _S12_PR0); // set timer prescale to divide by 128
   TIM0_TIE = (_S12_C4I);// | _S12_C5I | _S12_C6I);	// enable interrupts for IC4, IC5, OC6
   TIM0_TCTL3 = (_S12_EDG4A | _S12_EDG4B) | _S12_EDG5A | _S12_EDG7A; // capture rising edges on 4,5,7
   TIM0_TIOS = _S12_IOS6;												// set timer 0-6 as output compare
   TIM0_TC6 = TIM0_TCNT + HUNDREDMS;              // schedule the first OC6 event
   TIM0_TFLG1 = _S12_C4F | _S12_C5F | _S12_C6F | _S12_C7F; // clear IC4, IC5, OC6, IC7 flag
   TIM0_TFLG2 = _S12_TOF;													// clear overflow flag
   TIM0_TSCR2 |= _S12_TOI;												// turn on overflow interrupts
   
   // Second, Timer 1 which is for the beacons, and the absolute timer
	 
   TIM1_TSCR1 = _S12_TEN;							               // turn the timer system on
   TIM1_TSCR2 = (_S12_PR2 | _S12_PR1 | _S12_PR0);    // set timer prescale to divide by 128
   TIM1_TIE  = (_S12_C4I | _S12_C5I | _S12_C6I | _S12_C7I);	 // enable interrupts for IC4-7               
   //set up Timers for capturing beacon edges, and clear flags
   TIM1_TCTL3 |= (_S12_EDG4A | _S12_EDG4B); 	     // capture all edges on pin 4
   TIM1_TCTL3 |= (_S12_EDG5A | _S12_EDG5B); 	     // capture all edges on pin 5 
   TIM1_TCTL3 |= (_S12_EDG6A | _S12_EDG6B); 	     // capture all edges on pin 6
   TIM1_TCTL3 |= (_S12_EDG7A | _S12_EDG7B); 	     // capture all edges on pin 7
   TIM1_TFLG1 = _S12_C4F | _S12_C5F | _S12_C6F |  _S12_C7F;		// clear IC4-IC7 flags
   TIM1_TFLG2 = _S12_TOF;													// clear overflow flag
   TIM1_TSCR2 |= _S12_TOI;												// turn on overflow interrupts

	//initialize absolute timer
	 AbsoluteTime.ByInts.overflows = 0;
	 AbsoluteTime.ByInts.thetime = TIM1_TCNT;
	 
	 //set all timers to not started
   for(i = 0; i < NUM_TIMERS; i++) {
	 TimerStarted[i] = FALSE;
   }
	 
   //Start Flag Captured Timer
   StartTimer(FLAG_CAPTURED_TIMER, 1);
	  
}


/***********************************************************************
Function
    void StartTimer(char TimerNum, unsigned long ms_duration)
    
Description
    Starts a Timer
    
    Timer 1 is Left Encoder Stall Test
    Timer 2 is Right Encoder Stall Test
    Timer 3 is ControlLaw Update
    
***********************************************************************/

void StartTimer(char TimerNum, unsigned long ms_duration) {
  unsigned long ticks;

  //Calculate ms value in clock ticks. May be truncated.
  ticks = (unsigned long)(long)((float)ms_duration * TICKS_PER_MS);

  //Calculate end time and store in timer
  Timers[TimerNum].AsLong = GetAbsoluteTime() + ticks;
  TimerStarted[TimerNum] = TRUE;
}

/***********************************************************************
Function
    char IsTimerExpired(char TimerNum)
    
Description
    If a Timer has been started and has expired, returns true and sets
    timer to not started, else false.
    
***********************************************************************/

char IsTimerExpired(char TimerNum) {
  if((TimerStarted[TimerNum] == TRUE) && (Timers[TimerNum].AsLong <= GetAbsoluteTime())) {
    TimerStarted[TimerNum] = FALSE;
    return TRUE;
  }
  
  return FALSE;
}

/***********************************************************************
Function
    unsigned long GetAbsoluteTime(void)
    
Description
    Returns an unsigned long, which is the absolute value in ticks
    
***********************************************************************/

unsigned long GetAbsoluteTime(void) {
  unsigned long return_time;

  // disable overflow interrupts
  TIM1_TSCR2 &= (~_S12_TOI);
  AbsoluteTime.ByInts.thetime = TIM1_TCNT;
  return_time = AbsoluteTime.AsLong;
  //enable overflow interrupts
  TIM1_TSCR2 |= _S12_TOI;
  
  return return_time;
}

/***********************************************************************
Function
    void interrupt _Vec_tim1ovf Tim1OverflowInterrupt(void)
    
Description
    Increments the overflow counter for timer 1, for absolute timing purposes
    
***********************************************************************/

void interrupt _Vec_tim1ovf Tim1OverflowInterrupt(void) {
  AbsoluteTime.ByInts.overflows++;		
  TIM1_TFLG2 = _S12_TOF;
}

/*-------------------------- Test Harness ----------------------------*/

#ifdef TIMER_TEST
/* test Harness for testing this module */

void main(void) {
  
  (void)puts("\r\nIn test harness for timer.c \r\n");
  InitTimer();
  EnableInterrupts;
    
  StartTimer(1, 2000);
  while(1){
    if(IsTimerExpired(1)) {
      (void)printf("Egg is Cooked!\r\n");
    }
  }
}
#endif


/*-------------------------- End of file -----------------------------*/