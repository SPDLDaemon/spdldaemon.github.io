/***********************************************************************

Module
    master_sm.c

Description
    this is the master state machine module
    
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
#include "master_sm.h"
#include "timer.h"
#include "playing_sm.h"

/*------------------------ Module Defines ----------------------------*/
//#define MASTER_SM_TEST
	 
/*------------------------ Module Prototypes -------------------------*/
static Event_t DuringWaitingForFlash(Event_t Event);
static Event_t DuringPlayingGame(Event_t Event);
static Event_t DuringGameDone(Event_t Event);

/*------------------------ Module Variables --------------------------*/
static MasterState_t CurrentState;

/*-------------------------- Module Code -----------------------------*/

/****************************************************************************
 Function
     StartMasterSM

 Parameters
     None

 Returns
     None

 Description
     Does any required initialization for this state machine
****************************************************************************/
void StartMasterSM () {
  CurrentState = WAITING_FOR_FLASH;
  
  // call the entry function (if any) for the ENTRY_STATE
  (void)RunMasterSM(EV_ENTRY);
}

/****************************************************************************
 Function
    RunMasterSM

 Parameters
   Event_t: the event to process

 Returns
   Event_t: an event to return

 Description
   
****************************************************************************/
Event_t RunMasterSM(Event_t CurrentEvent) {
  unsigned char MakeTransition = FALSE;
  MasterState_t NextState = CurrentState;

  switch(CurrentState) {
   case WAITING_FOR_FLASH:
     //Execute during function. EV_ENTRY and EV_EXIT are processed here
     CurrentEvent = DuringWaitingForFlash(CurrentEvent);
     //process any events
     switch(CurrentEvent) {
       case EV_FLASH:
         NextState = PLAYING_GAME;
         MakeTransition = TRUE;
         break;
     }
     break;

   case PLAYING_GAME:
     //Execute during function. EV_ENTRY and EV_EXIT are processed here
     CurrentEvent = DuringPlayingGame(CurrentEvent);
     //process any events
     switch(CurrentEvent) {
       case EV_GAME_TIMER_DONE:
         NextState = GAME_DONE;
         MakeTransition = TRUE;
         break;
     }
     break;

   case GAME_DONE:
     //Execute during function. EV_ENTRY and EV_EXIT are processed here
     CurrentEvent = DuringGameDone(CurrentEvent);
     break;
  }
  
  if(MakeTransition == TRUE) {
    //Execute exit function for current state
    (void)RunMasterSM(EV_EXIT);
    CurrentState = NextState;
    //Execute entry function for new state
    (void)RunMasterSM(EV_ENTRY);
  }
  return CurrentEvent;
}
/****************************************************************************
 Function
     QueryTemplateSM

 Parameters
     None

 Returns
     MasterState_t The current state of the Template state machine
****************************************************************************/
MasterState_t QueryMasterSM (void) {
  return CurrentState;
}

/* Private Functions */
//Executes code that should run when in the waiting for flash state
static Event_t DuringWaitingForFlash(Event_t Event) {
  //process EV_ENTRY and EV_EXIT events
  if(Event != EV_ENTRY && Event != EV_EXIT) {
    //Don't move
    SimpleMove(0, 0);
  }
  return Event;
}

//Executes code that should run when in the playing game state
static Event_t DuringPlayingGame(Event_t Event) {
  //process EV_ENTRY and EV_EXIT events
  if(Event == EV_ENTRY) {
    StartTimer(GAME_TIMER, GAME_TIME);
    StartTimer(WARNING_TIMER, WARNING_TIME);
    StartPlayingSM();
  } else if(Event == EV_EXIT) {
    // on exit, give the lower levels a chance to clean up first
    RunPlayingSM(Event);
  } else { //do the 'during' function for this state
     // run any lower level state machine
     Event = RunPlayingSM(Event);
  }
  return Event;
}

//Executes code that should run when in the game done state
static Event_t DuringGameDone(Event_t Event) {
  //process EV_ENTRY and EV_EXIT events
  if(Event == EV_ENTRY) {
    //Don't move
    SimpleMove(0, 0);
  }
  return Event;
}

/*-------------------------- Test Harness ----------------------------*/
#ifdef STRATEGY_TEST

void main(void) {
}

#endif

/*-------------------------- End of file -----------------------------*/
