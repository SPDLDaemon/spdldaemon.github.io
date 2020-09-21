//*********************************************************************
// CoachDisplay.c
//*********************************************************************
// Stuart Calder; ME218C
// Mississippi Queen Project: 05/09
//*********************************************************************

//*********************************************************************
// Header File Includes
//*********************************************************************

#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include <stdio.h>
#include "S12eVec.h"
#include "CoachDisplay.h"

//*********************************************************************
// Test Define
//*********************************************************************
//#define TEST1

//*********************************************************************
// Functions
//*********************************************************************

//*********************************************************************
// Mutiny LED Alert Functions
//*********************************************************************

void MutinyAlertLED(void) {

  // Turn the mutiny alert LED on
  PTP |= BIT3HI;
  
  (void)printf("The mutiny alert lamp was activated\n\r");
}

void MutinyAlertLEDOff(void) {

  // Turn the mutiny alert LED off
  PTP &= BIT3LO;
  
  (void)printf("The mutiny alert lamp was turned off\n\r");
}

//*********************************************************************
// Play Mississippi Queen Function
//*********************************************************************  
  
void PlayMississippiQueen(void) {
  // Play that funky music, white COACH
  PTP |= BIT1HI;      //turn on the music
  
  (void)printf("The Mississippi Queen music was played\n\r");
}
  
//*********************************************************************
// Not Paired Alert Functions
//*********************************************************************  
  
void NotPairedAlert(void) {
  
  // Turn the Not Paired LED on
  PTP |= BIT4HI;
  
  (void)printf("The 'Not Paired' LED was activated\n\r");
}

void NotPairedAlertOff(void) {
  
  // Turn the Not Paired LED off
  PTP &= BIT4LO;
  
  (void)printf("The 'Not Paired' LED was turned off\n\r");
}

//*********************************************************************
// Team Color Function
//*********************************************************************  

unsigned char GetTeamColor(void){
  
  (void)printf("The team color is %d\n\r", !(PTP&BIT0HI));
  
  // Check the status of the team switch
  return !(PTP&BIT0HI);
}