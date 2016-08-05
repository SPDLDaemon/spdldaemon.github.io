/*
Morse Code Sample Pseudo Code Using the Gen2.x Event Framework
Rev 14 10/21/15
*************************************************************************************
Pseudo-code for the Button module (a service that implements a state machine)
*/

//#define TEST
// Test function for low level functions in part 3.1

//----------------------------- Include Files -----------------------------*/
// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include <PIC16F1788.h>
#include "BITDEFS.H"
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"	
#include "ES_Timers.h"
#include "ES_Framework.h"
#include "Button.h"

#define BUTTON_PIN RA7 
#define BUTTON_HI 1

#define ALL_BITS (0xff<<2)
#define DEBOUNCE_TIME 2

// Module variables
static ButtonState_t CurrentState;
static uint8_t MyPriority;
static uint8_t LastButtonState;

// Private function prototypes
static uint8_t GetInputLine(void);

// function GetInputLine 
static uint8_t GetInputLine(void) {
	// returns the state of the input line pin
	return BUTTON_PIN;
}

//Takes a priority number, returns True. 
bool InitializeButtonDebounce(uint8_t Priority) {
	//Initialize the MyPriority variable with the passed in parameter.
	MyPriority = Priority;
	// Initialization of button pin is handled in main.c
	// Sample the button port pin and use it to initialize LastButton state
	LastButtonState = GetInputLine();
	// Set CurrentState to be DEBOUNCING
	CurrentState = Debouncing;
	// Start the debounce timer (time posts to ButtonDebounceSM)
	ES_Timer_InitTimer(BUTTON_DEBOUNCE_TIMER,DEBOUNCE_TIME);
	// Return true
	return true;

} 

// Takes no parameters, returns True if an event was posted (11/4/11 jec)
bool CheckButtonEvents(void) {
	// local RetrunVal = False, CurrentButtonState
	uint8_t CurrentButtonState;
	bool ReturnVal = false;
	// Get the CurrentButtonState from the input line
	CurrentButtonState = GetInputLine();
	// If the state of the Morse input line has changed
	if (CurrentButtonState != LastButtonState) {
		// Set ReturnVal = True
		ReturnVal = true;
		ES_Event ThisEvent;
		// If the current state of the input line down
		if (CurrentButtonState != BUTTON_HI) {    
            // PostEvent ButtonDown to ButtonDebounce queue
			ThisEvent.EventType = DBButtonDown;
			PostButtonDebounceSM(ThisEvent);
		// Else (current input state is high)
		} else {
		// 	PostEvent ButtonUP to ButtonDebounce queue
			ThisEvent.EventType = DBButtonUp;
			PostButtonDebounceSM(ThisEvent);
		}
	}
	// Set LastInputState to CurrentInputState
	LastButtonState = CurrentButtonState;
	// Return ReturnVal
	return ReturnVal;
}

// Posts an event to this state machine's queue
bool PostButtonDebounceSM( ES_Event ThisEvent )
{
	// return posted event
  return ES_PostToService(MyPriority, ThisEvent);
}


// RunButtonDebounceSM (implements the state machine for Button Debounce)
ES_Event RunButtonDebounceSM( ES_Event ThisEvent ) {
	// The EventType field of ThisEvent will be one of: Debouncing, Ready2Sample

	// Based on the state of the CurrentState variable choose one of the following blocks of code:
	switch (CurrentState) {

		// CurrentState is Debouncing
		case Debouncing :
			// If ThisEvent is ES_TIMEOUT
			if (ThisEvent.EventType == ES_TIMEOUT) {
				// Set current state to Ready2Sample
				CurrentState = Ready2Sample;
			}
			break; 

		// CurrentState is Ready2Sample
		case Ready2Sample :
			// If ThisEvent is ButtonUp
			if (ThisEvent.EventType == DBButtonUp) {
				// Start debounce timer
				ES_Timer_InitTimer(BUTTON_DEBOUNCE_TIMER, DEBOUNCE_TIME);
				// Set CurrentState to Debouncing
				CurrentState = Debouncing;
				// Post DBPulseButtonUp to StepService and ADService queues
				ES_Event Event2Post;
				Event2Post.EventType = DBButtonUp;
				ES_PostAll(Event2Post);
			}
			// 	If ThisEvent is ButtonDown
			if (ThisEvent.EventType == DBButtonDown) {
                

				// Start debounce timer
				ES_Timer_InitTimer(BUTTON_DEBOUNCE_TIMER, DEBOUNCE_TIME);
				// Set CurrentState to Debouncing
				CurrentState = Debouncing;
				// Post DBPulseButtonDown to StepService and ADService queues
				ES_Event Event2Post;
				//Event2Post.EventType = DBButtonDown;
                Event2Post.EventType = PAIR_TRIGGER;
				ES_PostAll(Event2Post);
			}
			break;
	}

	ES_Event ReturnEvent;
	ReturnEvent.EventType = ES_NO_EVENT;
	return ReturnEvent;
	// End of RunButtonDebounceSM 
}


#ifdef TEST
// Test harness for this module

int main(void)
{
	// Testing functions for 3.1
	TERMIO_Init();\
	
	
}
#endif



