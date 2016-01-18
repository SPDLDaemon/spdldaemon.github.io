/* main.c
 * ======
 * Main module that initializes other modules, then loops
 * forever, checking and handling events.
 */


#include <stdio.h>
#include <timers12.h>
#include "ir_sensors.h"
#include "atari.h"
#include "cheat.h"
#include "motion.h"

//#define DEBUG

#ifndef DEBUG

void main(void) {
  char event;
  char player_ready;

	TMRS12_Init(TMRS12_RATE_1MS);
  AtariInit();
  IrSensorsInit();
  InitCheat();

  player_ready = FALSE;
  while(1) {
    //Wait for player to put hands on controller.
    while(player_ready == FALSE) {
      event = CheckMotionEvents();
      //Turn off all controls
      HandleMotionEvents(NO_EVENT);
      if(event != DOWN && event != NO_EVENT && event != POWER_OFF) {
        player_ready = TRUE;
      }
    }
    
    event = CheckMotionEvents();
    //If power is turned off, revert to starting state.
    if(event == POWER_OFF) {
      player_ready = FALSE;
      ExecuteCheat(CLOSE);
    } else {
      HandleMotionEvents(event);
      HandleCheatEvents(event);
    }
  }
}

#endif

//Test harness
#ifdef DEBUG

void main(void) {
  char event;
  char player_ready;

	TMRS12_Init(TMRS12_RATE_1MS);
  AtariInit();
  IrSensorsInit();
  InitCheat();
  
  
  player_ready = FALSE;
  while(1) {
    //Wait for player to put hands on controller.
    while(player_ready == FALSE) {
      event = CheckMotionEvents();
      //Turn off all controls
      HandleMotionEvents(NO_EVENT);
      if(event != DOWN && event != NO_EVENT && event != POWER_OFF) {
        player_ready = TRUE;
      }
    }
    
    event = CheckMotionEvents();
    //If power is turned off, revert to starting state.
    if(event == POWER_OFF) {
      player_ready = FALSE;
      ExecuteCheat(CLOSE);
    } else {
      HandleMotionEvents(event);
      HandleCheatEvents(event);

      switch(event) {
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
        case POWER_OFF:
          printf("POWER_OFF\r\n");
          break;
      }
    }
  } 
}

#endif
