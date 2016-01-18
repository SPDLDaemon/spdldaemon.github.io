/***********************************************************************

Module
    playing_sm.c

Description
    this is the playing state machine module

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
#include "motors.h"
#include "playing_sm.h"
#include "timer.h"
#include "beacons.h"
#include "goalfinding_sm.h"

/*------------------------ Module Defines ----------------------------*/
//#define PLAYING_SM_TEST
	 
/*------------------------ Private Function Prototypes ---------------*/
static Event_t DuringInitialMoving(Event_t Event);
static Event_t DuringGoingToGoal(Event_t Event);  
static Event_t DuringLookingForFlag(Event_t Event);
static Event_t DuringHeadingForFlag(Event_t Event);
static Event_t DuringCapturingFlag(Event_t Event);

/*------------------------ Module Variables --------------------------*/
static PlayingState_t CurrentState = INITIAL_MOVING;
  static unsigned char NextTurn = RIGHT;

/*-------------------------- Module Code -----------------------------*/

/****************************************************************************
 Function
     StartPlayingSM

 Parameters
     None

 Returns
     None

 Description
     Does any required initialization for this state machine
****************************************************************************/
void StartPlayingSM (void) {
  // call the entry function (if any)
  (void)RunPlayingSM(EV_ENTRY);
}

/****************************************************************************
 Function
    RunPlayingSM

 Parameters
   Event_t: the event to process

 Returns
   Event_t: an event to return

 Description
   
****************************************************************************/
Event_t RunPlayingSM(Event_t CurrentEvent) {
  static unsigned char FlagsCaptured = 0;

  unsigned char MakeTransition = FALSE;
  PlayingState_t NextState = CurrentState;

  switch(CurrentState) {
    case INITIAL_MOVING:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringInitialMoving(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_FLAG_SENSED:
          NextState = HEADING_FOR_FLAG;
          MakeTransition = TRUE;
          break;
      }
      break;

    case HEADING_FOR_FLAG:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringHeadingForFlag(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_GATE_BLOCKED: 
          NextState = CAPTURING_FLAG;
          MakeTransition = TRUE;
          break;
          
        case EV_FLAG_LOST:
          NextState = LOOKING_FOR_FLAG;
          MakeTransition = TRUE;
          break;
        
        case EV_WARNING_TIMER_DONE:
          NextState = GOING_TO_GOAL;
          MakeTransition = TRUE;
          break;
      }
      break;
    
    case CAPTURING_FLAG:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringCapturingFlag(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_CAPTURING_TIMER_DONE:
          FlagsCaptured++;
          if(FlagsCaptured == 1) {
            NextTurn = RIGHT;
            SetPWMOffset(15);
            NextState = LOOKING_FOR_FLAG;
          } else if(FlagsCaptured == 2) {
            NextTurn = LEFT;
            NextState = LOOKING_FOR_FLAG;
          } else if(FlagsCaptured == 3) {
            NextState = GOING_TO_GOAL;
          } else {
            NextState = LOOKING_FOR_FLAG;
          }
          MakeTransition = TRUE;
          break;
      }
      break;

    case LOOKING_FOR_FLAG:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringLookingForFlag(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_GATE_BLOCKED: 
          NextState = CAPTURING_FLAG;
          MakeTransition = TRUE;
          break;

        case EV_FLAG_SENSED:
          NextState = HEADING_FOR_FLAG;
          MakeTransition = TRUE;
          break;
          
        case EV_WARNING_TIMER_DONE:
          NextState = GOING_TO_GOAL;
          MakeTransition = TRUE;
          break;
      }
      break;

    case GOING_TO_GOAL:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringGoingToGoal(CurrentEvent);
      break;
  }
  
  if(MakeTransition == TRUE) {
    //Execute exit function for current state
    (void)RunPlayingSM(EV_EXIT);
    CurrentState = NextState;
    //Execute entry function for new state
    (void)RunPlayingSM(EV_ENTRY);
  }
  return CurrentEvent;
}

/****************************************************************************
 Function
     QueryPlayingSM

 Parameters
     None

 Returns
     PlayingState_t The current state of the playing state machine
****************************************************************************/
PlayingState_t QueryPlayingSM (void) {
  return CurrentState;
}

/* Private Functions */
//Executes code that should run when in the initial moving state
static Event_t DuringInitialMoving(Event_t Event) {
  //process EV_ENTRY and EV_EXIT events
  if(Event == EV_ENTRY) {
    //Go Straight
    SimpleMove(20, 20);
  }	
  return Event;
}

//Executes code that should run when in the heading for flash state
static Event_t DuringHeadingForFlag(Event_t Event) {
  if(Event != EV_ENTRY && Event != EV_EXIT) {//do the 'during' function for this state
    BeaconControlMove(); //Use the control law to move towards the beacon
  }
  return Event;
}

//Executes code that should run when in the capturing flag state
static Event_t DuringCapturingFlag(Event_t Event) {
  if(Event == EV_ENTRY) {
    SimpleMove(20, 20);
  }
  return Event;
}

//Executes code that should run when in the going to goal state
static Event_t DuringGoingToGoal(Event_t Event) {
  //process EV_ENTRY and EV_EXIT events
  if(Event == EV_ENTRY) {
    StartGoalFindingSM();
  } else {
    Event = RunGoalFindingSM(Event);
  }
  return Event;
}

//Executes code that should run when in the looking for flag state
static Event_t DuringLookingForFlag(Event_t Event) {
  //process EV_ENTRY and EV_EXIT events
  if(Event == EV_ENTRY) {
    if(NextTurn == LEFT) {
      //Turn Left
      SimpleMove(-15, 20);
    } else {
      //Turn Right
      SimpleMove(20, -15);
    }
      
  }
  return Event;
}

/*-------------------------- Test Harness ----------------------------*/
#ifdef STRATEGY_TEST

void main(void) {
}

#endif

/*-------------------------- End of file -----------------------------*/
