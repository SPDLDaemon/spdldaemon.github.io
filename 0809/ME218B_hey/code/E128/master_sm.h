/***********************************************************************

Module
    master_sm.h

Description
    Header file for the master state machine
    
***********************************************************************/

#ifndef MASTER_SM_H
#define MASTER_SM_H

#include "events.h"

// States for the master state machine
typedef enum { WAITING_FOR_FLASH,
               PLAYING_GAME,
               GAME_DONE } MasterState_t;

// Public Function Prototypes
void StartMasterSM (void);
Event_t RunMasterSM(Event_t CurrentEvent);
MasterState_t QueryMasterSM (void);

#endif


/*-------------------------- End of file -----------------------------*/
