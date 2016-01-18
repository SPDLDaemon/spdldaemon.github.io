/***********************************************************************

Module
    goalfinding_sm.c

Description
    this is the goalfinding state machine module
    
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
#include "motors.h"
#include "goalfinding_sm.h"
#include "timer.h"
#include "beacons.h"
#include "events.h"
#include "tape.h"

/*------------------------ Module Defines ----------------------------*/
//#define GOALFINDING_SM_TEST
	 
/*------------------------ Private Function Prototypes ---------------*/
static Event_t DuringFindingGoal(Event_t Event);
static Event_t DuringHeadingToGoal(Event_t Event);  
static Event_t DuringOrientingInGoal(Event_t Event);
static Event_t DuringWaiting(Event_t Event);

/*------------------------ Module Variables --------------------------*/
static GoalFindingState_t CurrentState;

/*-------------------------- Module Code -----------------------------*/

/****************************************************************************
 Function
     StartGoalFindingSM

 Parameters
     None

 Returns
     None

 Description
     Does any required initialization for the goalfinding state machine
****************************************************************************/
void StartGoalFindingSM (void) {
  CurrentState = FINDING_GOAL;

  // call the entry function (if any)
  (void)RunGoalFindingSM(EV_ENTRY);
}

/****************************************************************************
 Function
    RunGoalFindingSM

 Parameters
   Event_t: the event to process

 Returns
   Event_t: an event to return

 Description
   Runs the tasks for the goalfinding state machine.
   
****************************************************************************/
Event_t RunGoalFindingSM(Event_t CurrentEvent) {
  unsigned char MakeTransition = FALSE;
  GoalFindingState_t NextState = CurrentState;

  switch(CurrentState) {
    case FINDING_GOAL:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringFindingGoal(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_AT_GOAL_DIRECTION:
          NextState = HEADING_TO_GOAL;
          MakeTransition = TRUE;
          break;
      }
      break;

    case HEADING_TO_GOAL:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringHeadingToGoal(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_HIT_GREEN_PAINT:
          NextState = ORIENTING_IN_GOAL;
          MakeTransition = TRUE;
          break;
      }
      break;

    case ORIENTING_IN_GOAL:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringOrientingInGoal(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_IN_GOAL:
          NextState = WAITING;
          MakeTransition = TRUE;
          break;
      }
      break;

    case WAITING:
      //Execute during function. EV_ENTRY and EV_EXIT are processed here
      CurrentEvent = DuringWaiting(CurrentEvent);
      //process any events
      switch(CurrentEvent) {
        case EV_MISALIGNED:
          NextState = ORIENTING_IN_GOAL;
          MakeTransition = TRUE;
          break;
      }
      break;
  }
  
  if(MakeTransition == TRUE) {
    //Execute exit function for current state
    (void)RunGoalFindingSM(EV_EXIT);
    CurrentState = NextState;
    //Execute entry function for new state
    (void)RunGoalFindingSM(EV_ENTRY);
  }
  return CurrentEvent;
}

/****************************************************************************
 Function
     QueryGoalFindingSM

 Parameters
     None

 Returns
     GoalFindingState_t The current state of the goalfinding state machine
****************************************************************************/
GoalFindingState_t QueryGoalFindingSM (void) {
  return CurrentState;
}

/* Private Functions */
//Executes code that should run when in the finding goal state
static Event_t DuringFindingGoal(Event_t Event) {

  if(Event == EV_ENTRY) {
    //Turn Left
    SimpleMove(0, 35);
  }
  return Event;
}

//Executes code that should run when in the heading to goal state
static Event_t DuringHeadingToGoal(Event_t Event) {
  if(Event != EV_EXIT && Event != EV_ENTRY) { //do the 'during' function for this state
    if(Event == EV_HIT_BLACK_PAINT) {
      //Go straight more slowly
      SimpleMove(20, 20);
    } else {
      //Go straight
      SimpleMove(30, 30);
    }
  }
  return Event;
}

//Executes code that should run when in the orienting in goal state
static Event_t DuringOrientingInGoal(Event_t Event) {
  char LeftColor, RightColor;

  //process EV_ENTRY and EV_EXIT events
  if(Event != EV_EXIT) {
    //Orienting code
    LeftColor = GetTapeColor(FRONT_LEFT);
    RightColor = GetTapeColor(FRONT_RIGHT);
    if((LeftColor != GREEN && RightColor != GREEN)       //If both front sensors out of goal area
       || (LeftColor == GREEN && RightColor == GREEN)) { //or both front sensors in goal area
      //Move forward
      SimpleMove(20, 20);
    } else if(LeftColor == GREEN && RightColor == BLACK) {//If only left in goal area
      //Turn Left
      SimpleMove(-25, 25);
    } else if(LeftColor == BLACK && RightColor == GREEN) {//If only right in goal area
      //Turn right
      SimpleMove(25, -25);
    }
  }
  return Event;
}

//Executes code that should run when in the waiting state
static Event_t DuringWaiting(Event_t Event) {
  if(Event == EV_ENTRY) {
    //Stand still
    SimpleMove(0, 0);
  }
  return Event;
}

/*-------------------------- Test Harness ----------------------------*/
#ifdef GOALFINDING_SM_TEST

void main(void) {
  (void)printf("In goalfinding_sm.c test harness.\r\n");
  InitPorts();
  InitTimer();
  InitMotors();
  InitSide();
  InitBeacons();
  EnableInterrupts;

  StartGoalFindingSM();

  (void)RunGoalFindingSM(EV_AT_GOAL_DIRECTION);
  while(1) {		   		     // Repeat State Machine Forever
    (void)RunGoalFindingSM(CheckEvents());
  }
}

#endif

/*-------------------------- End of file -----------------------------*/
