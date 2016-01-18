/***********************************************************************

Module
    events.c

Description
    This is the events module. It checks events for the state chart.

***********************************************************************/


/*------------------------- Include Files ----------------------------*/

#include <termio.h>
#include <stdio.h>
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12E128bits.h>
#include <bitdefs.h>
#include "ADS12.h"
#include "events.h"
#include "timer.h"
#include "beacons.h"
#include "tape.h"
#include "goalfinding_sm.h"
#include "playing_sm.h"

/*------------------------ Module Variables --------------------------*/



/*------------------------ Module Functions --------------------------*/



/*------------------------ Module Defines ----------------------------*/
//#define EVENTS_TEST

/*-------------------------- Module Code -----------------------------*/
/****************************************************************************
 Function
     CheckEvents

 Parameters
     None

 Returns
     Event_t the detected event

 Description
     Checks and returns events.
****************************************************************************/

Event_t CheckEvents(void) {
  static char FlagSensed = FALSE;
  static char Capturing = FALSE;
  static char RightSawOpposing = FALSE;
  static char HitGreenTape = FALSE;

  GoalFindingState_t GFState;
  PlayingState_t PState;
  char TapeFL, TapeFR, TapeR;
  CornerBeacons_t corners;
  
  //Get the side information
  corners = GetSide();
  //Get the state of the playing state machine
  PState = QueryPlayingSM();
  //Get the state of the goalfinding state machine
  GFState = QueryGoalFindingSM();
  //Get the tape sensor values
  TapeFL = GetTapeColor(FRONT_LEFT);
  TapeFR = GetTapeColor(FRONT_RIGHT);
  TapeR = GetTapeColor(REAR);
  
  if(DetectFlash()) {
   //Flash detected
   return EV_FLASH;
  }
  
  if(IsTimerExpired(GAME_TIMER)) {
    //Game is over
    return EV_GAME_TIMER_DONE;
  }
  
  if(PState == GOING_TO_GOAL) {
    //In GOING_TO_GOAL state of playing state machine

    if(GFState == FINDING_GOAL && BeaconDuty(RIGHT_TOP_BEACON) == corners.opposing) {
      //Note that the right top beacon has seen the duty cycle of the opposing goal
      RightSawOpposing = TRUE;
    }
    
    if(IsTimerExpired(GREEN_TAPE_TIMER) == TRUE) {
      //Note that the robot has passed onto and out of the green tape
      HitGreenTape = TRUE;
    }
    
    if(GFState == HEADING_TO_GOAL
       && (GetTapeColor(FRONT_LEFT) == GREEN_TAPE || GetTapeColor(FRONT_RIGHT) == GREEN_TAPE)) {
      //Start a timer when we hit green tape, and restart as long as on tape.
      //Prevents firing a green tape event multiple times
      StartTimer(GREEN_TAPE_TIMER, 100);
    }
    
    if(GFState == FINDING_GOAL && BeaconDuty(RIGHT_TOP_BEACON) == corners.own && RightSawOpposing == TRUE) {
      //When right beacon sees own beacon after seeing opposing beacon, we must be facing our goal
      TIM1_TIE &= ~(_S12_C4I | _S12_C5I); //Disable beacon interrupts
      return EV_AT_GOAL_DIRECTION;
    }
    
    if(GFState == HEADING_TO_GOAL
       && (TapeFL == GREEN || TapeFR == GREEN)
       && HitGreenTape == TRUE) {
      //Front hit green paint
      return EV_HIT_GREEN_PAINT;
    }
    
    if(GFState == HEADING_TO_GOAL && TapeFL == BLACK && TapeFR == BLACK) {
      //Front hit black paint
      return EV_HIT_BLACK_PAINT;
    }
    
    if((TapeFL == GREEN || TapeFL == WHITE)
       && (TapeFR == GREEN || TapeFR == WHITE)
       && TapeR == GREEN) {
      //In goal when front sensors are on green or white and rear is in green.
      return EV_IN_GOAL;
    } else {
      //If not in goal, must be misaligned.
      return EV_MISALIGNED;
    }
  }
  
  if(IsTimerExpired(FLAG_CAPTURED_TIMER)) {
    //Done capturing a flag
    return EV_CAPTURING_TIMER_DONE;
  }
  
  if(IsGateBlocked() == TRUE) {
    //Flag is entering the catcher.
    //Timer prevents thinking multiple flags have been captured.
    StartTimer(FLAG_CAPTURED_TIMER, 500);
    return EV_GATE_BLOCKED;
  }
  
  if(GetBeaconLevel(LEFT_FLAG_LEVEL) > 400
     || GetBeaconLevel(RIGHT_FLAG_LEVEL) > 400) {
    //Flag levels are above a threshold
    FlagSensed = TRUE;
    return EV_FLAG_SENSED;
  } else if(FlagSensed == TRUE) {
    //Flag lost when it was sensed, but now it is not
    FlagSensed = FALSE;
    return EV_FLAG_LOST;
  }
  
  return EV_NO_EVENT;
}


#ifdef EVENTS_TEST

void main(void) {
	
}

#endif

/*-------------------------- End of file -----------------------------*/
