/* motion.c
 * ========
 */

#include <stdio.h>
#include <timers12.h>
#include "ir_sensors.h"
#include "motion.h"
#include "atari.h"
#include "cheat.h"
#include "our_c32.h"

//#define DEBUG

//Single hand location constants
#define HAND_CENTER 0
#define HAND_UP     1
#define HAND_DOWN   2
#define HAND_RIGHT  3
#define HAND_LEFT   4
#define HAND_CLOSE  5

//Movement timing constants
#define BACKWARDS_WAIT 250

//Cheat time constants
#define CHEAT_TIMEOUT 3000

//Prototypes
static char NormalControls(char lDirection, char rDirection);

//Implementation

char CheckMotionEvents(void) {
  static char last_button = NO_EVENT;
  static unsigned int last_time = 0;

  char front, back, right, left;
  char rDirection = -1;
  char lDirection = -1;
  char returnDirection;
  unsigned int curr_time;
  
  if(GetPowerStatus() == LO) {
    return POWER_OFF;
  }
  
  //Left hand
  front = GetSensorValue(LF_SENSOR);
  back = GetSensorValue(LB_SENSOR);
  right = GetSensorValue(LR_SENSOR);
  left = GetSensorValue(LL_SENSOR);
  
//determine direction on left side  
  if(left != UNCOVERED) {
    lDirection = LEFT;
  } else if(right != UNCOVERED) {
    lDirection = RIGHT;
  } else if(front != UNCOVERED) {
    lDirection = UP;
  } else if(back == UNCOVERED) {
    lDirection = DOWN;
  }
  
  //Right hand
  front = GetSensorValue(RF_SENSOR);
  back = GetSensorValue(RB_SENSOR);
  right = GetSensorValue(RR_SENSOR);
  left = GetSensorValue(RL_SENSOR);
  
//determine direction on right side  
  if(left != UNCOVERED) {
    rDirection = LEFT;
  } else if(right != UNCOVERED) {
    rDirection = RIGHT;
  } else if(front != UNCOVERED) {
    rDirection = UP;
  } else if(back == UNCOVERED) {
    rDirection = DOWN;
  }

  returnDirection = NormalControls(lDirection, rDirection);
  //Ignore DOWN events too soon after other events because could
  //just be transition.
  curr_time = TMRS12_GetTime();
  if(returnDirection == DOWN && curr_time - last_time < BACKWARDS_WAIT) {
    return last_button;
  }				 
  last_time = curr_time;
  
  last_button = returnDirection;
  return returnDirection;
}

static char NormalControls(char lDirection, char rDirection) {
  if(lDirection == UP && rDirection == UP) {
    return UP;
  }
  if(lDirection == RIGHT && rDirection == RIGHT) {
    return RIGHT;
  }
  if(lDirection == LEFT && rDirection == LEFT) {
    return LEFT;
  }
  if(lDirection == DOWN && rDirection == DOWN) {
    return DOWN;
  }
  if(lDirection == LEFT && rDirection == RIGHT) {
    return BUTTON;
  }

  return NO_EVENT;
}

void HandleMotionEvents(char event) {
  static char last_button = NO_EVENT;
  
  if(last_button == event) {
    return;
  }
  
  //if something happened
  if(last_button != NO_EVENT) {
    //turn the previous signal to the atari off  
    ToggleButton(last_button, OFF);
  } 
  if(event != NO_EVENT && event != POWER_OFF) {
    ToggleButton(event, ON);
  }
   
  last_button = event;
}

void HandleCheatEvents(char event) {
  static char last_event = NO_EVENT;
  static char step = 0;
  static unsigned int last_time = 0;

  unsigned int time_elapsed;
  char next_move;

//turn off fan and reset cheat if Power is turned off  
  if(GetPowerStatus() == LO) {
    step = 0;
    ExecuteCheat(OFF);
    return;
  }
 
//exit cheat function if there is no event 
  if(event == NO_EVENT || last_event == event || step == 3) {
    return;
  }

  switch(step) {
    case 0:
      next_move = DOWN;
      last_time = TMRS12_GetTime();
      break;
    case 1:
      next_move = RIGHT;
      break;
    case 2:
      next_move = UP;
      break;
  }
  time_elapsed = TMRS12_GetTime() - last_time;
 
//if the timer has not yet expired and the correct event occurs
//increment the cheat step, otherwise reset it to zero 
  if(time_elapsed < CHEAT_TIMEOUT && event == next_move) {
    step++;
  } else {
    step = 0;
  }


//cheat successfully executed  
  if(step == 3) {
    ExecuteCheat(ON);
  }
  last_event = event;
}

//Test harness
#ifdef DEBUG

void main(void) {
  unsigned int time, start;
  char motion;

  /*
  //General test
  IrSensorsInit();
  TMRS12_Init(TMRS12_RATE_1MS);
  while(1) {
    motion = CheckMotionEvents();
    
    switch(motion) {
      case UP:
        printf("UP\r\n");
        break;
      case DOWN:
        printf("DOWN\r\n");
        break;
      case RIGHT:
        printf("RIGHT\r\n");
        break;
      case LEFT:
        printf("LEFT\r\n");
        break;
      case BUTTON:
        printf("BUTTON\r\n");
        break;
      case NO_EVENT:
        printf("NO_EVENT\r\n");
        break;
    }
  }
  */
  
  
  //Cheat timeout test
  TMRS12_Init(TMRS12_RATE_1MS);
  InitCheat();
  AtariInit();
  printf("DOWN\r\n");
  HandleCheatEvents(DOWN); 
  printf("RIGHT\r\n");
  HandleCheatEvents(RIGHT); 
  /*printf("waiting...\r\n");
  start = TMRS12_GetTime();
  while(1) {
    time = TMRS12_GetTime();
    if(time - start >= 5001) {
      break;
    }
  }*/
  printf("UP\r\n");
  HandleCheatEvents(UP);
  
  
  /*
  //DOWN wait test
  TMRS12_Init(TMRS12_RATE_1MS);
  InitCheat();
  AtariInit();
  printf("UP\r\n");
  HandleMotionEvents(UP);
  while(1) {
    HandleMotionEvents(DOWN);
  }
  */
}

#endif
