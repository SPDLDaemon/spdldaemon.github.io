/***********************************************************************

Module
    events.h

Description
    Header file for events module.
    
    
***********************************************************************/

#ifndef events_H
#define events_H

// Event definitions
//Universal events take up 0,1 & 2. User Events start at 3
typedef enum {  EV_NO_EVENT,
                EV_ENTRY,
                EV_EXIT,
                
                EV_FLASH,
                EV_GAME_TIMER_DONE,
                EV_FLAG_SENSED,
                EV_GATE_BLOCKED,
                EV_CAPTURING_TIMER_DONE,
                EV_FLAG_LOST,
                EV_WARNING_TIMER_DONE,
                EV_HIT_BLACK_PAINT,
                EV_HIT_GREEN_PAINT,
                EV_AT_GOAL_DIRECTION,
                EV_ORIENTING_IN_GOAL,
                EV_MISALIGNED,
                EV_IN_GOAL } Event_t;

Event_t CheckEvents(void);

#endif
