/***********************************************************************

Module
    main.c

Description
    E128 code for the Project!
    
Notes
    Requires Board Connections as noted in the lab writeup

***********************************************************************/


/*------------------------- Include Files ----------------------------*/

#include <termio.h>
#include <stdio.h>
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"
#include "ADS12.h"

#include "timer.h"
#include "motors.h"
#include "beacons.h"
#include "tape.h"

#include "master_sm.h"
#include "events.h"

/*------------------------ Module Variables --------------------------*/

/*------------------------ Module Functions --------------------------*/

/*------------------------ Module Defines ----------------------------*/

/*-------------------------- Module Code -----------------------------*/

/***********************************************************************
Function
    void main(void)
    
Description
    High level: This module runs all operations of the Project. 
	It initializes the Ports, Timers, Motors, Beacons, and State Machine.
    It then infinitely runs the master state machine and checks for events.
    
***********************************************************************/

void main(void) {
  (void)printf("Start of E128 program\r\n");
  InitPorts();
  InitTimer();
  InitMotors();
  InitSide();
  InitBeacons();
  EnableInterrupts;

  StartMasterSM();

  while(1) {		   		     // Repeat State Machine Forever
    (void)RunMasterSM(CheckEvents());
  }
}

/*-------------------------- End of file -----------------------------*/