/****************************************************************************
 
  Header file for Button.c
  based on the Gen 2 Events and Services Framework

 ****************************************************************************/

#ifndef Button_H
#define Button_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"	
#include <PIC16F1788.h>

// typedefs for the states
// State definitions for use with the query function
typedef enum { Debouncing, Ready2Sample} ButtonState_t ;

// Public Function Prototypes

bool InitializeButtonDebounce(uint8_t Priority);
bool PostButtonDebounceSM(ES_Event ThisEvent);
ES_Event RunButtonDebounceSM(ES_Event ThisEvent);


#endif /* Button_H */