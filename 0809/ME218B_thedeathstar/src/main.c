/*
 * Main Module for Capture the Flag
 *
 * This is the main module for capture the flag.  The main function
 * performs various initializations and then enters an event-driven
 * loop to run the various state machines of the project.
 *
 * @author David Hoffert
 */

/*
 * Module includes
 */

// Generic E128 includes
#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"

// Low-level module includes
#include "events.h"
#include "stateMachines.h"
#include "drivetrain.h"
#include "tape.h"
#include "lightSensor.h"
#include "Claw.h"

// Main function includes
#include <stdio.h>
#include "ads12.h"

// Select the main function to be run
#define NO_TEST

/*
 * Main funcion
 *
 * Performs various initializations and then enters an event-driven
 * loop to run the various state machines of the project.
 *
 * @author David Hoffert
 */
#ifdef NO_TEST

void main() {
  
  // Declare local variables
  char event;
  char whichMachine;
  char whichTeam;
  
  // Variables for debugging print-outs
  int i;
  char duty;
  long j;
  
  // Initialize the necessary subsystems
  drivetrainInit();
  TapeSensorInit();
  initLights();
  ClawInit();
  EnableInterrupts;
    
  // Indicate which state machine will be run
  whichMachine = getStateMachine();
  if(whichMachine == MAIN_PLAY) {
    (void)printf("Running primary state machine\r\n");
  } else {
    (void)printf("Running sudden death state machine\r\n");
  }
  
  // Determine which team we're playing for
  whichTeam = getTeam();
  if(whichTeam == RED_TEAM) {
    (void)printf("Playing as red team\r\n");
  } else {
    (void)printf("Playing as green team\r\n");
  }
  
  // Set the initial motion command for the robot
  commandMotion(STOP_COMMAND); // Real setting
  //commandMotion(ROT_CCW_SLOW); // Flag sensing check-off
  //commandMotion(ROT_CCW_FAST); // Beacon sensing check-off
  //commandMotion(DRV_FWD_FAST); // Tape sensing check-off
  
  
  // Enter an event-driven loop to run the proper state machine
  while(1) {
  
    // Let the tape sensors read their values
    TapeSensorRead();
    
    /*
    // Print out the values of the IR sensors
    for(i=0; i<8; i++){
	    duty = getIRDutyCycle(i);
	    //if(duty != 0) {
	      (void)printf("DC%d: ", i);
		    (void)printf("%d    ",duty);
	    //}
	  }
	  (void)printf("\r\n");
	  */
	  
	  /*
	  // Print out the values of the tape sensors
	  if(j%2000 == 0){
      (void)printf("FL: %d,%d " ,(int)getTapeColor(TP_FL), ADS12_ReadADPin(TP_FL));
      (void)printf("  FR: %d,%d " ,(int)getTapeColor(TP_FR), ADS12_ReadADPin(TP_FR));
      (void)printf("  C: %d,%d",(int)getTapeColor(TP_C), ADS12_ReadADPin(TP_C));
      (void)printf("  BL: %d,%d ",(int)getTapeColor(TP_BL), ADS12_ReadADPin(TP_BL));
      (void)printf("  BR: %d,%d",(int)getTapeColor(TP_BR), ADS12_ReadADPin(TP_BR));
      (void)printf("\r\n");
      j = 0;
    }
    j++;
    */
    
    // Check for events
    event = checkEvents();
    if(event != NO_EVENT) {
      
      // Sanity check
      //(void)printf("%d\r\n", event);
      
      // Run the appropriate state machine
      if(whichMachine == MAIN_PLAY) mainPlayStateMachine(event);
      else suddenDeathStateMachine(event);
    }
  }
}

#endif
