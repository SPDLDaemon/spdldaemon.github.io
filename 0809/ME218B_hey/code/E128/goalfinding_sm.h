/***********************************************************************

Module
    goalfinding_sm.h

Description
    Header file for the goal-finding state machine
    
***********************************************************************/

#ifndef GOALFINDING_SM_H
#define GOALFINDING_SM_H

#include "events.h"

// Goalfinding States
typedef enum { FINDING_GOAL,
               HEADING_TO_GOAL,
               ORIENTING_IN_GOAL,
               WAITING } GoalFindingState_t;

// Public Function Prototypes
void StartGoalFindingSM (void);
Event_t RunGoalFindingSM(Event_t CurrentEvent);
GoalFindingState_t QueryGoalFindingSM (void);

#endif


/*-------------------------- End of file -----------------------------*/
