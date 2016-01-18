/* atari.c
 * =======
 */

// library files
#include <stdio.h>
#include "our_c32.h"
#include "atari.h"

//#define DEBUG

//Port constants
#define ATARI_PORT T
#define POWER_PIN 1

//Implementation

/* AtariInit
 * =========
 * See header file for usage.
 */
void AtariInit(void) {
  char i;

//set the Atari power pin to read
  SetDDR(ATARI_PORT, POWER_PIN, READ);

//set each Atari control pin to write 
  for(i = UP; i >= BUTTON; i--) {
    WritePort(ATARI_PORT, i, OFF);
    SetDDR(ATARI_PORT, i, WRITE);
  }
}

/* ToggleButton
 * ============
 * See header file for usage.
 */
void ToggleButton(char button, char state) {
  char currState;
  char newState;
  
  switch(state) {
    case ON:
      newState = ON;
      break;
    case OFF:
      newState = OFF;
      break;
    case TOGGLE:
      currState = (char)ReadPort(ATARI_PORT, button);
      newState = !(currState && ON);
      break;
  }

  WritePort(ATARI_PORT, button, newState);
}

/* GetPowerStatus
 * ==============
 * See header file for usage.
 */
char GetPowerStatus(void) {
  return !((char)ReadPort(ATARI_PORT, POWER_PIN));
}

//Test harness
#ifdef DEBUG

void main(void) {
  char dummy;

  AtariInit();

  //Test all buttons
  ToggleButton(UP, ON);
  printf("UP on\r\n");
  dummy = getchar();
  	 
  ToggleButton(UP, OFF);
  printf("UP off\r\n");
  dummy = getchar(); 
  
  ToggleButton(DOWN, ON);
  printf("DOWN on\r\n");
  dummy = getchar();  
  
  ToggleButton(DOWN, OFF);
  printf("DOWN off\r\n");
  dummy = getchar();  
  
  ToggleButton(RIGHT, ON);
  printf("RIGHT on\r\n");
  dummy = getchar(); 
    
  ToggleButton(RIGHT, OFF);
  printf("RIGHT off\r\n");
  dummy = getchar();
  
  ToggleButton(LEFT, ON);
  printf("LEFT on\r\n");
  dummy = getchar(); 											
  
  ToggleButton(LEFT, OFF);
  printf("LEFT off\r\n");
  dummy = getchar();
  
  ToggleButton(BUTTON, ON);
  printf("BUTTON on\r\n");
  dummy = getchar(); 											
  
  ToggleButton(BUTTON, OFF);
  printf("BUTTON off\r\n");
  dummy = getchar();
  
}

#endif