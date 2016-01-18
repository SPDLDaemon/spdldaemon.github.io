/***********************************************************************

Module
    playing_sm.h

Description
    Header file for the playing state machine
    
***********************************************************************/

#ifndef PLAYING_SM_H
#define PLAYING_SM_H

#include "events.h"

// States for playing state machine
typedef enum { INITIAL_MOVING,
               LOOKING_FOR_FLAG,
               HEADING_FOR_FLAG,
               CAPTURING_FLAG,
               GOING_TO_GOAL } PlayingState_t;

// Public Function Prototypes
void StartPlayingSM (void);
Event_t RunPlayingSM(Event_t CurrentEvent);
PlayingState_t QueryPlayingSM (void);

#endif


/*-------------------------- End of file -----------------------------*/
