#include <hidef.h>
#include <stdio.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"
#include "TwoMinTimer.h"



//#define TIMER_TEST

static char TIMER_STATE = NOT_EXPIRED;
static int count = 0;


void startTimer(void){

 // Initialize the TIM2 timer (it may be already initialized in beacon sensors)
 
 // ----- TIMER -----------
 TIM2_TSCR1 = _S12_TEN; /* turn the timer system on */
  
 // set pre-scale to /128 or 187.5 kHz clock 
 TIM2_TSCR2 |= _S12_PR0;
 TIM2_TSCR2 |= _S12_PR1;
 TIM2_TSCR2 |= _S12_PR2; 

 //------Output compare (just for 10ms timing)------
 //----------using TC4 for output compare-----------
  
 // (--- use TIM2_TC4 (fourth timer on tim2, PU0 port on E128) ---------)
  
 //set the fourth TCx on TIM1 to output compare
  TIM2_TIOS |= _S12_IOS5;
 
 // set first event to current time 
  TIM2_TC5 = TIM2_TCNT+TIMER_DELAY;
 
 // clear the flag.
  TIM2_TFLG1 = _S12_C5F;
  TIM2_TIE |= _S12_C5I;

 EnableInterrupts;

  (void)printf("Output compare initialized for two minute timer... \r\n");


}



char getTimer(){
  return TIMER_STATE;
}



void interrupt _Vec_tim2ch5 TimerSignalInterrupt (void) {

  // set up the next compare 
  TIM2_TC5 += TIMER_DELAY;
  // clear the flag for the compare
  TIM2_TFLG1 = _S12_C5F;

  count++; 
  
  if(count == MAX_COUNT){
     TIMER_STATE = EXPIRED; 
    }

}




#ifdef TIMER_TEST

void main() 
{

 startTimer();
 
 while(1){
   (void)printf("Timer is at cnt %d and the state is (1 is exprired): %d \r\n", count, getTimer());
 }

}

#endif

