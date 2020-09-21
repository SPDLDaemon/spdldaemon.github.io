/*
 * Event Checker Module for Capture the Flag
 *
 * This is the event-checking module for capture the flag.  When the
 * public event-checking function it provides is called, it polls all
 * of the low-level sensor modules for changes in states, and reports
 * back the first event it finds.
 *
 * @author David Hoffert
 */
 
// Generic E128 Includes
#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"

// Module-Specific Includes
#include "events.h"
#include "lightSensor.h"
#include "tape.h"
#include "Claw.h"
#include "flashSensor.h"
#include "bumpSensor.h"
#include "TwoMinTimer.h"

// IR sensor states
static char IR_12F_state = IS_0;
static char IR_12B_state = IS_0;
static char IR_9FL_LR_state = IS_0;
static char IR_9FR_LR_state = IS_0;
static char IR_9F_SR_state = IS_0;
static char IR_9BL_LR_state = IS_0;
static char IR_9BR_LR_state = IS_0;
static char IR_9B_SR_state = IS_0;

/*
 * Public function to return the current state of the IR sensors,
 * since the IR module does not provide this and the state machine
 * needs it for guarding.
 *
 * @author David Hoffert
 */
char getIRstate(char sensor) {
  switch(sensor) {
    case IR_9FL_LR: return IR_9FL_LR_state;
    case IR_9F_SR: return IR_9F_SR_state;
    case IR_9FR_LR: return IR_9FR_LR_state;
    case IR_9BL_LR: return IR_9BL_LR_state;
    case IR_9B_SR: return IR_9B_SR_state;
    case IR_9BR_LR: return IR_9BR_LR_state;
    case IR_12F: return IR_12F_state;
    case IR_12B: return IR_12B_state;
  }
}

/*
 * Public function to check for events.  Polls all low-level sensor
 * modules for changes in states, and reports back the first event it
 * finds.
 *
 * @author David Hoffert
 */
char checkEvents() {
  
  /*
   * Declare static variables
   */
  
  // Tape sensor states
  static char TP_C_state = GREEN;
  static char TP_FL_state = GREEN;
  static char TP_FR_state = GREEN;
  static char TP_BL_state = GREEN;
  static char TP_BR_state = GREEN;
  
  // Other states
  static char FLSH_state = FLSH_OFF;
  static char BMP_F_state = BMP_OFF;
  static char BMP_B_state = BMP_OFF;
  static char DR_F_state = DR_OPEN;
  static char DR_B_state = DR_OPEN;
  static char TMR_state = NOT_EXPIRED;
  
  /*
   * Declare local variables
   */
  char event = NO_EVENT;
  char newIRDutyCycle = 0;
  char newTapeColor = BLACK;
  char newBinaryState = 0;
  
  /*
   * Check for events, roughly in increasing order of frequency
   */
  
  // Check for miscellaneous events
  
  newBinaryState = getTimer();
  if(newBinaryState == NOT_EXPIRED) {
    if(TMR_state != NOT_EXPIRED) {
      TMR_state = NOT_EXPIRED;
      return NO_EVENT;
    }
  } else if(newBinaryState == EXPIRED) {
    if(TMR_state != EXPIRED) {
      TMR_state = EXPIRED;
      return TIMER_2MIN_EXP;
    }
  }
  
  newBinaryState = getBump(BMP_F);
  if(newBinaryState == BMP_ON) {
    if(BMP_F_state != BMP_ON) {
      BMP_F_state = BMP_ON;
      return BMP_F_NOW_ON;
    }
  } else if(newBinaryState == BMP_OFF) {
    if(BMP_F_state != BMP_OFF) {
      BMP_F_state = BMP_OFF;
      return BMP_F_NOW_OFF;
    }
  }
  
  newBinaryState = getBump(BMP_B);
  if(newBinaryState == BMP_ON) {
    if(BMP_B_state != BMP_ON) {
      BMP_B_state = BMP_ON;
      return BMP_B_NOW_ON;
    }
  } else if(newBinaryState == BMP_OFF) {
    if(BMP_B_state != BMP_OFF) {
      BMP_B_state = BMP_OFF;
      return BMP_B_NOW_OFF;
    }
  }
  
  newBinaryState = getDoor(DR_FRONT);
  if(newBinaryState == DR_OPEN) {
    if(DR_F_state != DR_OPEN) {
      DR_F_state = DR_OPEN;
      return DR_F_NOW_OPEN;
    }
  } else if(newBinaryState == DR_CLSD) {
    if(DR_F_state != DR_CLSD) {
      DR_F_state = DR_CLSD;
      return DR_F_NOW_CLSD;
    }
  }
  
  newBinaryState = getDoor(DR_BACK);
  if(newBinaryState == DR_OPEN) {
    if(DR_B_state != DR_OPEN) {
      DR_B_state = DR_OPEN;
      return DR_B_NOW_OPEN;
    }
  } else if(newBinaryState == DR_CLSD) {
    if(DR_B_state != DR_CLSD) {
      DR_B_state = DR_CLSD;
      return DR_B_NOW_CLSD;
    }
  }
  
  // Check for tape events
  newTapeColor = getTapeColor(TP_C);
  if(newTapeColor == BLACK) {
    if(TP_C_state != BLACK) {
      TP_C_state = BLACK;
      return TP_C_NOW_BLACK;
    }
  } else if(newTapeColor == GREEN) {
    if(TP_C_state != GREEN) {
      TP_C_state = GREEN;
      return TP_C_NOW_GREEN;
    }
  } else if(newTapeColor == WHITE){
    if(TP_C_state != WHITE) {
      TP_C_state = WHITE;
      return TP_C_NOW_WHITE;
    }
  }
  
  newTapeColor = getTapeColor(TP_FL);
  if(newTapeColor == BLACK) {
    if(TP_FL_state != BLACK) {
      TP_FL_state = BLACK;
      return TP_FL_NOW_BLACK;
    }
  } else if(newTapeColor == GREEN) {
    if(TP_FL_state != GREEN) {
      TP_FL_state = GREEN;
      return TP_FL_NOW_GREEN;
    }
  } else if(newTapeColor == WHITE){
    if(TP_FL_state != WHITE) {
      TP_FL_state = WHITE;
      return TP_FL_NOW_WHITE;
    }
  }
  
  newTapeColor = getTapeColor(TP_FR);
  if(newTapeColor == BLACK) {
    if(TP_FR_state != BLACK) {
      TP_FR_state = BLACK;
      return TP_FR_NOW_BLACK;
    }
  } else if(newTapeColor == GREEN) {
    if(TP_FR_state != GREEN) {
      TP_FR_state = GREEN;
      return TP_FR_NOW_GREEN;
    }
  } else if(newTapeColor == WHITE){
    if(TP_FR_state != WHITE) {
      TP_FR_state = WHITE;
      return TP_FR_NOW_WHITE;
    }
  }
  
  newTapeColor = getTapeColor(TP_BL);
  if(newTapeColor == BLACK) {
    if(TP_BL_state != BLACK) {
      TP_BL_state = BLACK;
      return TP_BL_NOW_BLACK;
    }
  } else if(newTapeColor == GREEN) {
    if(TP_BL_state != GREEN) {
      TP_BL_state = GREEN;
      return TP_BL_NOW_GREEN;
    }
  } else if(newTapeColor == WHITE){
    if(TP_BL_state != WHITE) {
      TP_BL_state = WHITE;
      return TP_BL_NOW_WHITE;
    }
  }
  
  newTapeColor = getTapeColor(TP_BR);
  if(newTapeColor == BLACK) {
    if(TP_BR_state != BLACK) {
      TP_BR_state = BLACK;
      return TP_BR_NOW_BLACK;
    }
  } else if(newTapeColor == GREEN) {
    if(TP_BR_state != GREEN) {
      TP_BR_state = GREEN;
      return TP_BR_NOW_GREEN;
    }
  } else if(newTapeColor == WHITE){
    if(TP_BR_state != WHITE) {
      TP_BR_state = WHITE;
      return TP_BR_NOW_WHITE;
    }
  }
   
  // Check for IR events
  newIRDutyCycle = getIRDutyCycle(IR_12F);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_12F_state != IS_0) {
      IR_12F_state = IS_0;
      return IR_12F_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_30 && newIRDutyCycle <= UPPER_30) {
    if(IR_12F_state != IS_30) {
      IR_12F_state = IS_30;
      return IR_12F_NOW_30;
    }
  } else if(newIRDutyCycle >= LOWER_70 && newIRDutyCycle <= UPPER_70) {
    if(IR_12F_state != IS_70) {
      IR_12F_state = IS_70;
      return IR_12F_NOW_70;
    }
  } else {
    if(IR_12F_state != IS_OTHER) {
      IR_12F_state = IS_OTHER;
      return IR_12F_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_12B);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_12B_state != IS_0) {
      IR_12B_state = IS_0;
      return IR_12B_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_30 && newIRDutyCycle <= UPPER_30) {
    if(IR_12B_state != IS_30) {
      IR_12B_state = IS_30;
      return IR_12B_NOW_30;
    }
  } else if(newIRDutyCycle >= LOWER_70 && newIRDutyCycle <= UPPER_70) {
    if(IR_12B_state != IS_70) {
      IR_12B_state = IS_70;
      return IR_12B_NOW_70;
    }
  } else {
    if(IR_12B_state != IS_OTHER) {
      IR_12B_state = IS_OTHER;
      return IR_12B_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_9FL_LR);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_9FL_LR_state != IS_0) {
      IR_9FL_LR_state = IS_0;
      return IR_9FL_LR_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_50 && newIRDutyCycle <= UPPER_50) {
    if(IR_9FL_LR_state != IS_50) {
      IR_9FL_LR_state = IS_50;
      return IR_9FL_LR_NOW_50;
    }
  } else {
    if(IR_9FL_LR_state != IS_OTHER) {
      IR_9FL_LR_state = IS_OTHER;
      return IR_9FL_LR_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_9FR_LR);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_9FR_LR_state != IS_0) {
      IR_9FR_LR_state = IS_0;
      return IR_9FR_LR_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_50 && newIRDutyCycle <= UPPER_50) {
    if(IR_9FR_LR_state != IS_50) {
      IR_9FR_LR_state = IS_50;
      return IR_9FR_LR_NOW_50;
    }
  } else {
    if(IR_9FR_LR_state != IS_OTHER) {
      IR_9FR_LR_state = IS_OTHER;
      return IR_9FR_LR_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_9F_SR);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_9F_SR_state != IS_0) {
      IR_9F_SR_state = IS_0;
      return IR_9F_SR_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_50 && newIRDutyCycle <= UPPER_50) {
    if(IR_9F_SR_state != IS_50) {
      IR_9F_SR_state = IS_50;
      return IR_9F_SR_NOW_50;
    }
  } else {
    if(IR_9F_SR_state != IS_OTHER) {
      IR_9F_SR_state = IS_OTHER;
      return IR_9F_SR_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_9BL_LR);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_9BL_LR_state != IS_0) {
      IR_9BL_LR_state = IS_0;
      return IR_9BL_LR_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_50 && newIRDutyCycle <= UPPER_50) {
    if(IR_9BL_LR_state != IS_50) {
      IR_9BL_LR_state = IS_50;
      return IR_9BL_LR_NOW_50;
    }
  } else {
    if(IR_9BL_LR_state != IS_OTHER) {
      IR_9BL_LR_state = IS_OTHER;
      return IR_9BL_LR_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_9BR_LR);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_9BR_LR_state != IS_0) {
      IR_9BR_LR_state = IS_0;
      return IR_9BR_LR_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_50 && newIRDutyCycle <= UPPER_50) {
    if(IR_9BR_LR_state != IS_50) {
      IR_9BR_LR_state = IS_50;
      return IR_9BR_LR_NOW_50;
    }
  } else {
    if(IR_9BR_LR_state != IS_OTHER) {
      IR_9BR_LR_state = IS_OTHER;
      return IR_9BR_LR_NOW_OTHER;
    }
  }
  
  newIRDutyCycle = getIRDutyCycle(IR_9B_SR);
  if(newIRDutyCycle <= UPPER_0) {
    if(IR_9B_SR_state != IS_0) {
      IR_9B_SR_state = IS_0;
      return IR_9B_SR_NOW_0;
    }
  } else if(newIRDutyCycle >= LOWER_50 && newIRDutyCycle <= UPPER_50) {
    if(IR_9B_SR_state != IS_50) {
      IR_9B_SR_state = IS_50;
      return IR_9B_SR_NOW_50;
    }
  } else {
    if(IR_9B_SR_state != IS_OTHER) {
      IR_9B_SR_state = IS_OTHER;
      return IR_9B_SR_NOW_OTHER;
    }
  }
  
  // Check for the flash event
  newBinaryState = getFlash();
  if(newBinaryState == FLSH_ON) {
    if(FLSH_state != FLSH_ON) {
      FLSH_state = FLSH_ON;
      return FLSH_NOW_ON;
    }
  } else if(newBinaryState == FLSH_OFF) {
    if(FLSH_state != FLSH_OFF) {
      FLSH_state = FLSH_OFF;
      return FLSH_NOW_OFF;
    }
  }
    
  // If we get here, return the non-event
  return event;
}