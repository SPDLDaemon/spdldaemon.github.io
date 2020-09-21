/*
 * State Machine Module for Capture the Flag
 *
 * This is the state machine module for capture the flag.  It contains
 * public functions for executing both the main play state machine and
 * the sudden death state machine.
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
#include <stdio.h>
#include "stateMachines.h"
#include "events.h"
#include "drivetrain.h"
#include "Claw.h"
#include "TwoMinTimer.h"
#include "lightSensor.h"

// Turning on debug mode prints out each state transition and cause
#define DEBUG_STATE_MACHINES

// Toggle between IR sensor flag following modes
//#define IR_CLOSE_BOTH_GOOD
//#define IR_FAR_NEITHER_GOOD
#define ALL_THREE_LONG_RANGE

// Toggle between active claws and passive flaps design
//#define ACTIVE_CLAWS
#define PASSIVE_FLAPS

// Toggle between simple orientation and choreographed orientation
#define ORIENT_SIMPLE
//#define ORIENT_SCRIPT

// Toggle between parking using green and parking using only black and white
//#define PARK_IN_COLOR
#define PARK_BLACK_WHITE

// Limit the flag following test harness to only flag following
//#define IR_FOLLOW_TEST

// Module variables
static char homeDC;
static char awayDC;

/*
 * Public function to determine which state machine should be run, by
 * reading the state of a physical switch on the robot.
 *
 * @return Which state machine should be run this time
 *
 * @author David Hoffert
 */
char getStateMachine() {
  
  // Initialize the pin to be an input
  DDRP &= BIT5LO;
  
  // Read the physical switch
  if(PTP & BIT5HI){
    return SUDDEN_DEATH;
  } else{
    return MAIN_PLAY;
  }
}

/*
 * Public function to determine which team the robot is playing for, and
 * to set the corresponding variables internal to this module, by reading
 * the state of a physical switch on the robot.
 *
 * @return Which team the robot is playing for this time
 *
 * @author David Hoffert
 */
char getTeam() {
  
  // Declare local variables
  char team = RED_TEAM;
  
  // Initialize the pin to be an input
  DDRP &= BIT4LO;
  
  // Read the physical switch to determine the correct team
  if(PTP & BIT4HI) {
    team = GREEN_TEAM;
    homeDC = 70;
    awayDC = 30;
  } else {
    // If we get here, we must have still been the red team
    homeDC = 30;
    awayDC = 70;
  }
  
  // Tell the caller which team we're on
  return team;
}

/*
 * Public function to execute the main play state machine.  Takes an
 * event to respond to and returns nothing.
 *
 * @param event The event for the state machine to react to
 *
 * @author David Hoffert
 */
void mainPlayStateMachine(char event) {
  
  // Declare local variables
  static char machine = MP_FLAG_1;
  static char state = MP_WAITING_FOR_FLASH;
  
  // Outer switch statement for the sub-machines
  // Middle switch statement for the states
  // Inner switch statements for the events
  switch(machine) {
  
    case MP_FLAG_1:
      
      switch(state) {
        
        case MP_WAITING_FOR_FLASH:
        
          switch(event) {
            case FLSH_NOW_ON:
              commandMotion(ROT_CW__FAST);
              startTimer();
              state = MP_NO_DATA;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Flash-->No Data\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_NO_DATA:
        
          switch(event) {
            #ifdef ORIENT_SCRIPT
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_BACK_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Back Saw Home\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_BACK_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Back Saw Home\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_FRONT_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Front Saw Home\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_FRONT_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Front Saw Home\r\n");
                #endif
              }
              break;
            case IR_9FL_LR_NOW_50:
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FRONT_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw 50-->Front Saw Flag\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_BACK_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw 50-->Back Saw Flag\r\n");
              #endif
              break;
            #endif
            
            #ifdef ORIENT_SIMPLE
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
            #endif
          }
          
          break;
          
        case MP_FRONT_SAW_HOME:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_BACK_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw 50-->Back Saw Flag\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BACK_SAW_HOME:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FRONT_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw 50-->Front Saw Flag\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FRONT_SAW_FLAG:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12F_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_SEEKING_FLAG_1_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Seeking Flag 1 Ahead\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_SEEKING_FLAG_1_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Seeking Flag 1 Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case MP_BACK_SAW_FLAG:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_SEEKING_FLAG_1_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Seeking Flag 1 Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_SEEKING_FLAG_1_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Seeking Flag 1 Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case MP_SEEKING_FLAG_1_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_SEEKING_FLAG_1_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
        
        #ifdef ALL_THREE_LONG_RANGE
        
        case MP_FLAG_1_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_1_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Center Saw 50-->Flag 1 Ahead Follow\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_1_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Center Saw 50-->Flag 1 Behind Follow\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_0:
            case IR_9F_SR_NOW_OTHER:
              if(getIRstate(IR_9FL_LR) != IS_50) {
                commandMotion(ROT_CW__FAST);
                state = MP_FLAG_1_AHEAD_VEER_CW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Center and Left Lost 50-->Flag 1 Ahead Veer CW\r\n");
                #endif
              } else if(getIRstate(IR_9FR_LR) != IS_50) {
                commandMotion(ROT_CCW_FAST);
                state = MP_FLAG_1_AHEAD_VEER_CCW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Center and Right Lost 50-->Flag 1 Ahead Veer CCW\r\n");
                #endif
              }
              break;
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = MP_FLAG_1_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 1 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Seeking Flag 2 Behind\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_0:
            case IR_9B_SR_NOW_OTHER:
              if(getIRstate(IR_9BR_LR) != IS_50) {
                commandMotion(ROT_CW__FAST);
                state = MP_FLAG_1_BEHIND_VEER_CW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Center and Right Lost 50-->Flag 1 Behind Veer CW\r\n");
                #endif
              } else if(getIRstate(IR_9BL_LR) != IS_50) {
                commandMotion(ROT_CCW_FAST);
                state = MP_FLAG_1_BEHIND_VEER_CCW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Center and Left Lost 50-->Flag 1 Behind Veer CCW\r\n");
                #endif
              }
              break;
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = MP_FLAG_1_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 1 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Seeking Flag 2 Ahead\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_1_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Center Saw 50-->Flag 1 Ahead Follow\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_1_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Center Saw 50-->Flag 1 Behind Follow\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef IR_CLOSE_BOTH_GOOD
        
        case MP_FLAG_1_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_1_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Follow\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_1_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Follow\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 1 Ahead Veer CCW\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Short Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 1 Behind Veer CCW\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Short Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_1_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Follow\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_1_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Follow\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = MP_FLAG_1_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 1 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Seeking Flag 2 Behind\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = MP_FLAG_1_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 1 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Seeking Flag 2 Ahead\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef IR_FAR_NEITHER_GOOD
        
        case MP_FLAG_1_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_1_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 1 Ahead Follow\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_1_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 1 Behind Follow\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Veer CW\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Veer CCW\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Short Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_1_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Veer CW\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Veer CCW\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Short Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_1_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 1 Ahead Follow\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_1_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 1 Behind Follow\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_1_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = MP_FLAG_1_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 1 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Seeking Flag 2 Behind\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = MP_FLAG_1_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 1 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Seeking Flag 2 Ahead\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        case MP_FLAG_1_AHEAD_CLOSING_DOOR:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case DR_F_NOW_CLSD:
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Door Closed-->Seeking Flag 2 Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_1_BEHIND_CLOSING_DOOR:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case DR_B_NOW_CLSD:
              commandMotion(ROT_CCW_FAST);
              machine = MP_FLAG_2;
              state = MP_SEEKING_FLAG_2_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Door Closed-->Seeking Flag 2 Ahead\r\n");
              #endif
              break;
          }
          
          break;
      }
      
      break;
      
    case MP_FLAG_2:
    
      switch(state) {
      
        case MP_SEEKING_FLAG_2_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_2_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 2 Ahead Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_SEEKING_FLAG_2_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_2_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 2 Behind Veer CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #ifdef ALL_THREE_LONG_RANGE
        
        case MP_FLAG_2_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_2_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Center Saw 50-->Flag 2 Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_2_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Center Saw 50-->Flag 2 Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_0:
            case IR_9F_SR_NOW_OTHER:
              if(getIRstate(IR_9FL_LR) != IS_50) {
                commandMotion(ROT_CW__FAST);
                state = MP_FLAG_2_AHEAD_VEER_CW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Center and Left Lost 50-->Flag 2 Ahead Veer CW\r\n");
                #endif
              } else if(getIRstate(IR_9FR_LR) != IS_50) {
                commandMotion(ROT_CCW_FAST);
                state = MP_FLAG_2_AHEAD_VEER_CCW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Center and Right Lost 50-->Flag 2 Ahead Veer CCW\r\n");
                #endif
              }
              break;
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = MP_FLAG_2_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 2 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_FWD_FAST);
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Find Black Line Ahead\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_0:
            case IR_9B_SR_NOW_OTHER:
              if(getIRstate(IR_9BR_LR) != IS_50) {
                commandMotion(ROT_CW__FAST);
                state = MP_FLAG_2_BEHIND_VEER_CW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Center and Right Lost 50-->Flag 2 Behind Veer CW\r\n");
                #endif
              } else if(getIRstate(IR_9BL_LR) != IS_50) {
                commandMotion(ROT_CCW_FAST);
                state = MP_FLAG_2_BEHIND_VEER_CCW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Center and Left Lost 50-->Flag 2 Behind Veer CCW\r\n");
                #endif
              }
              break;
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = MP_FLAG_2_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 2 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_REV_FAST);
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Find Black Line Behind\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_2_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Center Saw 50-->Flag 2 Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_2_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Center Saw 50-->Flag 2 Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef IR_CLOSE_BOTH_GOOD
          
        case MP_FLAG_2_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_2_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 2 Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_2_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 2 Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_2_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 2 Ahead Veer CW\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_2_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 2 Ahead Veer CCW\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_2_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Short Saw 50-->Flag 2 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_2_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 2 Behind Veer CW\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_2_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 2 Behind Veer CCW\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_2_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Short Saw 50-->Flag 2 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_2_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 2 Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_2_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 2 Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_2_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 2 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_2_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 2 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_2_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 2 Ahead Tweak CW\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_2_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 2 Ahead Tweak CCW\r\n");
              #endif
              break;
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = MP_FLAG_2_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 2 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_FWD_FAST);
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Find Black Line Ahead\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_2_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 2 Behind Tweak CW\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_2_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 2 Behind Tweak CCW\r\n");
              #endif
              break;
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = MP_FLAG_2_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 2 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_REV_FAST);
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Find Black Line Behind\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_2_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 2 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_2_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 2 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef IR_FAR_NEITHER_GOOD
          
        case MP_FLAG_2_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_2_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 2 Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_2_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 2 Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_2_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 2 Ahead Veer CW\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_2_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 2 Ahead Veer CCW\r\n");
              #endif
              break;
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_2_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Short Saw 50-->Flag 2 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__FAST);
              state = MP_FLAG_2_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 2 Behind Veer CW\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = MP_FLAG_2_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 2 Behind Veer CCW\r\n");
              #endif
              break;
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_2_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Short Saw 50-->Flag 2 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(DRV_FWD_FAST);
              state = MP_FLAG_2_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 2 Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(DRV_REV_FAST);
              state = MP_FLAG_2_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 2 Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_2_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 2 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_TWEAK_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_2_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 2 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_2_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 2 Ahead Tweak CW\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_2_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 2 Ahead Tweak CCW\r\n");
              #endif
              break;
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = MP_FLAG_2_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 2 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_FWD_FAST);
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Find Black Line Ahead\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_APPROACH:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = MP_FLAG_2_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 2 Behind Tweak CW\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = MP_FLAG_2_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 2 Behind Tweak CCW\r\n");
              #endif
              break;
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = MP_FLAG_2_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 2 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_REV_FAST);
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Find Black Line Behind\r\n");
              #endif
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(DRV_FWD_SLOW);
              state = MP_FLAG_2_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 2 Ahead Approach\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(DRV_REV_SLOW);
              state = MP_FLAG_2_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 2 Behind Approach\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
          
        case MP_FLAG_2_AHEAD_CLOSING_DOOR:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case DR_F_NOW_CLSD:
              commandMotion(DRV_FWD_FAST);
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Door Closed-->Find Black Line Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FLAG_2_BEHIND_CLOSING_DOOR:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case DR_B_NOW_CLSD:
              commandMotion(DRV_REV_FAST);
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Door Closed-->Find Black Line Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FIND_BLACK_LINE_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_C_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              machine = MP_PARK;
              state = MP_ORIENT_BLACK_LINE_AHEAD_1;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Center Saw Black-->Orient Black Line Ahead 1\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = MP_BLACK_ON_FRONT_LEFT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black on Front Left\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = MP_BLACK_ON_FRONT_RIGHT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Black on Front Right\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_FIND_BLACK_LINE_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_C_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              machine = MP_PARK;
              state = MP_ORIENT_BLACK_LINE_BEHIND_1;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Center Saw Black-->Orient Black Line Behind 1\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = MP_BLACK_ON_BACK_LEFT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Black on Back Left\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = MP_BLACK_ON_BACK_RIGHT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black on Back Right\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_ON_FRONT_LEFT:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw White-->Find Black Line Ahead\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              machine = MP_PARK;
              state = MP_UNKNOWN_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Unknown Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_ON_FRONT_RIGHT:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw White-->Find Black Line Ahead\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              machine = MP_PARK;
              state = MP_UNKNOWN_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Unknown Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_ON_BACK_LEFT:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BL_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw White-->Find Black Line Behind\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              machine = MP_PARK;
              state = MP_UNKNOWN_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Unknown Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_ON_BACK_RIGHT:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw White-->Find Black Line Behind\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              machine = MP_PARK;
              state = MP_UNKNOWN_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Unknown Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
      }
      
      break;
      
    case MP_PARK:
    
      switch(state) {
          
        case MP_UNKNOWN_GOAL_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12F_NOW_30:
            case IR_12B_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Right Goal Ahead\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
            case IR_12B_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Right Goal Ahead\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_UNKNOWN_GOAL_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Right Goal Behind\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Right Goal Behind\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_RIGHT_GOAL_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = MP_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Driving to Goal Ahead\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_RIGHT_GOAL_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = MP_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Driving to Goal Behind\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(awayDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(awayDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_WRONG_GOAL_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              machine = MP_FLAG_2;
              state = MP_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Find Black Line Behind\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(awayDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Right Goal Ahead\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(awayDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Right Goal Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_WRONG_GOAL_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BL_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              machine = MP_FLAG_2;
              state = MP_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Find Black Line Ahead\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Right Goal Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Right Goal Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_ORIENT_BLACK_LINE_AHEAD_1:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_VERIFY_HOME_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Verify Home Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_ORIENT_BLACK_LINE_BEHIND_1:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_VERIFY_HOME_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Verify Home Behind\r\n");
              #endif
              break;
          }
          
          break;
        
        case MP_VERIFY_HOME_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12F_NOW_30:
            case IR_12B_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Orient Black Line Ahead 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
            case IR_12B_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Orient Black Line Ahead 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case MP_VERIFY_HOME_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Orient Black Line Behind 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Orient Black Line Behind 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case MP_ORIENT_BLACK_LINE_AHEAD_2:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_BLACK_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black Ahead Veer CCW\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case MP_ORIENT_BLACK_LINE_BEHIND_2:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_BLACK_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black Behind Veer CCW\r\n");
              #endif
              break;
            case IR_12F_NOW_30:
            case IR_12B_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
            case IR_12B_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = MP_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        #ifdef PARK_IN_COLOR
        
        case MP_BLACK_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FR_NOW_WHITE:
            case TP_FR_NOW_GREEN:
              commandMotion(DRV_FWD_FAST);
              state = MP_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BL_NOW_WHITE:
            case TP_BL_NOW_GREEN:
              commandMotion(DRV_REV_FAST);
              state = MP_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_BLACK_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black Ahead Veer CCW\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = MP_BLACK_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Black Ahead Veer CW\r\n");
              #endif
              break;
            case TP_FL_NOW_GREEN:
            case TP_FR_NOW_GREEN:
              commandMotion(DRV_FWD_FAST);
              state = MP_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw Green-->Driving to Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_BLACK_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black Behind Veer CCW\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = MP_BLACK_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Black Behind Veer CW\r\n");
              #endif
              break;
            case TP_BL_NOW_GREEN:
            case TP_BR_NOW_GREEN:
              commandMotion(DRV_REV_FAST);
              state = MP_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw Green-->Driving to Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_WHITE:
            case TP_FL_NOW_GREEN:
              commandMotion(DRV_FWD_FAST);
              state = MP_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_WHITE:
            case TP_BR_NOW_GREEN:
              commandMotion(DRV_REV_FAST);
              state = MP_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef PARK_BLACK_WHITE
        
        case MP_BLACK_AHEAD_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = MP_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = MP_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw Black-->Driving to Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_BEHIND_VEER_CW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BL_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = MP_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = MP_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw Black-->Driving to Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_AHEAD_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_BLACK_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black Ahead Veer CCW\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = MP_BLACK_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Black Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_BEHIND_FOLLOW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = MP_BLACK_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black Behind Veer CCW\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = MP_BLACK_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Black Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_AHEAD_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = MP_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = MP_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw Black-->Driving to Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_BLACK_BEHIND_VEER_CCW:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = MP_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = MP_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw Black-->Driving to Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
          
        case MP_DRIVING_GOAL_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FL_NOW_WHITE:
            case TP_FR_NOW_WHITE:
              commandMotion(ROT_CW__FAST);
              state = MP_CENTERED_ON_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw White-->Centered on Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_DRIVING_GOAL_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_BL_NOW_WHITE:
            case TP_BR_NOW_WHITE:
              commandMotion(ROT_CW__FAST);
              state = MP_CENTERED_ON_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw White-->Centered on Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_CENTERED_ON_GOAL_AHEAD:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
            case TP_BL_NOW_WHITE:
              commandMotion(STOP_COMMAND);
              state = MP_PARKED_IN_GOAL;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Rotated 90 deg-->Parked in Goal\r\n");
              #endif
              break;
          }
          
          break;
          
        case MP_CENTERED_ON_GOAL_BEHIND:
        
          switch(event) {
            case TIMER_2MIN_EXP:
              commandMotion(STOP_COMMAND);
              state = MP_TIME_EXPIRED;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("2 mins elapsed-->Time Expired\r\n");
              #endif
              break;
            case TP_FR_NOW_WHITE:
            case TP_BL_NOW_BLACK:
              commandMotion(STOP_COMMAND);
              state = MP_PARKED_IN_GOAL;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Rotated 90 deg-->Parked in Goal\r\n");
              #endif
              break;
          }
          
          break;
      }
      
      break;
  }
}

/*
 * Public function to execute the sudden death state machine.  Takes an
 * event to respond to and returns nothing.
 *
 * @param event The event for the state machine to react to
 *
 * @author David Hoffert
 */
void suddenDeathStateMachine(char event) {
  
  // Declare local variables
  static char machine = SD_ORIENT;
  static char state = SD_WAITING_FOR_FLASH;
  
  // Initial state for flag sensing (flag following)
  // Place a flag just left of the front of the robot, so that it can't yet see it
  // The robot will rotate CCW and then start following (try moving the flag)
  //static char machine = SD_GET_FLAG;
  //static char state = SD_FLAG_1_AHEAD_TWEAK_CCW;
  
  // Initial state for beacon sensing
  // This is the actual starting state, minus the flash part
  // Place the robot facing exactly the opposite side, and place a flag halfway left on the field
  // The idea is that when the robot rotates CCW, it will see the flag and then the far away beacon
  // It should then drive towards that away beacon
  //static char machine = SD_ORIENT;
  //static char state = SD_NO_DATA;
  
  // Initial state for tape sensing (green and black)
  // Place the robot in the starting corner, facing exactly forward to the opposite side
  // The robot will drive forward until it sees the green line, and rotate to follow it
  // When it sees the first black intersection, it should rotate CW
  //static char machine = SD_ORIENT;
  //static char state = SD_FIND_GREEN_LINE_AHEAD_1;
  
  // Outer switch statement for the sub-machines
  // Middle switch statement for the states
  // Inner switch statements for the events
  switch(machine) {
    
    case SD_ORIENT:
    
      switch(state) {
        
        case SD_WAITING_FOR_FLASH:
        
          switch(event) {
            case FLSH_NOW_ON:
              commandMotion(ROT_CCW_FAST);
              state = SD_NO_DATA;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Flash-->No Data\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_NO_DATA:
        
          switch(event) {
            #ifdef ORIENT_SCRIPT
            case IR_12B_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_BACK_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Back Saw Home\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_BACK_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Back Saw Home\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_FRONT_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Front Saw Home\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_FRONT_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Front Saw Home\r\n");
                #endif
              }
              break;
            case IR_9FL_LR_NOW_50:
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = SD_FRONT_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw 50-->Front Saw Flag\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = SD_BACK_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw 50-->Back Saw Flag\r\n");
              #endif
              break;
            #endif
              
            #ifdef ORIENT_SIMPLE
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              machine = SD_GET_FLAG;
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              machine = SD_GET_FLAG;
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FRONT_SAW_HOME:
        
          switch(event) {
            case IR_9BL_LR_NOW_50:
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = SD_BACK_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw 50-->Back Saw Flag\r\n");
              #endif
              break;
            case IR_12F_NOW_30:
              if(awayDC == 30) {
                commandMotion(DRV_FWD_FAST);
                state = SD_FIND_GREEN_LINE_AHEAD_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Seeking Green Ahead 1\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
              if(awayDC == 70) {
                commandMotion(DRV_FWD_FAST);
                state = SD_FIND_GREEN_LINE_AHEAD_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Seeking Green Ahead 1\r\n");
                #endif
              }
              break; 
          }
          
          break;
          
        case SD_BACK_SAW_HOME:
        
          switch(event) {
            case IR_9FL_LR_NOW_50:
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CCW_FAST);
              state = SD_FRONT_SAW_FLAG;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw 50-->Front Saw Flag\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
              if(awayDC == 30) {
                commandMotion(DRV_REV_FAST);
                state = SD_FIND_GREEN_LINE_BEHIND_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Seeking Green Behind 1\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
              if(awayDC == 70) {
                commandMotion(DRV_REV_FAST);
                state = SD_FIND_GREEN_LINE_BEHIND_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Seeking Green Behind 1\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case SD_FRONT_SAW_FLAG:
        
          switch(event) {
            case IR_12F_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_FRONT_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Front Saw Home\r\n");
                #endif
              } else {
                commandMotion(DRV_FWD_FAST);
                state = SD_FIND_GREEN_LINE_AHEAD_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Seeking Green Ahead 1\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_FRONT_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Front Saw Home\r\n");
                #endif
              } else {
                commandMotion(DRV_FWD_FAST);
                state = SD_FIND_GREEN_LINE_AHEAD_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Seeking Green Ahead 1\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case SD_BACK_SAW_FLAG:
        
          switch(event) {
            case IR_12B_NOW_30:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_BACK_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Back Saw Home\r\n");
                #endif
              } else {
                commandMotion(DRV_REV_FAST);
                state = SD_FIND_GREEN_LINE_BEHIND_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Seeking Green Behind 1\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_BACK_SAW_HOME;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Back Saw Home\r\n");
                #endif
              } else {
                commandMotion(DRV_REV_FAST);
                state = SD_FIND_GREEN_LINE_BEHIND_1;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Seeking Green Behind 1\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case SD_FIND_GREEN_LINE_AHEAD_1:
        
          switch(event) {
            case TP_FL_NOW_WHITE:
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_GREEN_LINE_AHEAD_2;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw White-->Seeking Green Ahead 2\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FIND_GREEN_LINE_BEHIND_1:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
            case TP_BR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_GREEN_LINE_BEHIND_2;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw White-->Seeking Green Behind 2\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FIND_GREEN_LINE_AHEAD_2:
        
          switch(event) {
            case TP_C_NOW_GREEN:
              commandMotion(ROT_CCW_FAST);
              state = SD_ORIENT_GREEN_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Center Saw Green-->Orient Green Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FIND_GREEN_LINE_BEHIND_2:
        
          switch(event) {
            case TP_C_NOW_GREEN:
              commandMotion(ROT_CCW_FAST);
              state = SD_ORIENT_GREEN_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Center Saw Green-->Orient Green Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_ORIENT_GREEN_LINE_AHEAD:
        
          switch(event) {
            case TP_FR_NOW_GREEN:
              commandMotion(ROT_CW__FAST);
              state = SD_GREEN_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Green-->Green Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_ORIENT_GREEN_LINE_BEHIND:
        
          switch(event) {
            case TP_BL_NOW_GREEN:
              commandMotion(ROT_CW__FAST);
              state = SD_GREEN_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Green-->Green Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_GREEN_AHEAD_VEER_CW:
        
          switch(event) {
            case TP_FR_NOW_WHITE:
            case TP_FR_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = SD_GREEN_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost Green-->Green Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_GREEN_BEHIND_VEER_CW:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
            case TP_BL_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = SD_GREEN_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost Green-->Green Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_GREEN_AHEAD_FOLLOW:
        
          switch(event) {
            case TP_FL_NOW_GREEN:
              commandMotion(ROT_CCW_FAST);
              state = SD_GREEN_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Green-->Green Ahead Veer CCW\r\n");
              #endif
              break;
            case TP_FR_NOW_GREEN:
              commandMotion(ROT_CW__FAST);
              state = SD_GREEN_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Green-->Green Ahead Veer CW\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = SD_ROTATE_TO_FLAG_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Rotate to Flag Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_GREEN_BEHIND_FOLLOW:
        
          switch(event) {
            case TP_BR_NOW_GREEN:
              commandMotion(ROT_CCW_FAST);
              state = SD_GREEN_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Green-->Green Behind Veer CCW\r\n");
              #endif
              break;
            case TP_BL_NOW_GREEN:
              commandMotion(ROT_CW__FAST);
              state = SD_GREEN_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Green-->Green Behind Veer CW\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = SD_ROTATE_TO_FLAG_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Rotate to Flag Behind\r\n");
              #endif
          }
          
          break;
          
        case SD_GREEN_AHEAD_VEER_CCW:
        
          switch(event) {
            case TP_FL_NOW_WHITE:
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = SD_GREEN_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost Green-->Green Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_GREEN_BEHIND_VEER_CCW:
        
          switch(event) {
            case TP_BR_NOW_WHITE:
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = SD_GREEN_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost Green-->Green Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_ROTATE_TO_FLAG_AHEAD:
        
          switch(event) {
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_SLOW);
              machine = SD_GET_FLAG;
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_ROTATE_TO_FLAG_BEHIND:
        
          switch(event) {
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_SLOW);
              machine = SD_GET_FLAG;
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
      }
      
      break;
      
    case SD_GET_FLAG:
    
      switch(state) {
        
        #ifdef ALL_THREE_LONG_RANGE
        
        case SD_FLAG_1_AHEAD_TWEAK_CW:
        
          switch(event) {
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = SD_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Center Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_TWEAK_CW:
        
          switch(event) {
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = SD_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Center Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_AHEAD_APPROACH:
        
          switch(event) {
            case IR_9F_SR_NOW_0:
            case IR_9F_SR_NOW_OTHER:
              if(getIRstate(IR_9FL_LR) != IS_50) {
                commandMotion(ROT_CW__SLOW);
                state = SD_FLAG_1_AHEAD_TWEAK_CW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Center and Left Lost 50-->Flag 1 Ahead Tweak CW\r\n");
                #endif
              } else if(getIRstate(IR_9FR_LR) != IS_50) {
                commandMotion(ROT_CCW_SLOW);
                state = SD_FLAG_1_AHEAD_TWEAK_CCW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Center and Right Lost 50-->Flag 1 Ahead Tweak CCW\r\n");
                #endif
              }
              break;
            #ifndef IR_FOLLOW_TEST
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = SD_FLAG_1_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 1 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Find Black Line Ahead\r\n");
              #endif
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_APPROACH:
        
          switch(event) {
            case IR_9B_SR_NOW_0:
            case IR_9B_SR_NOW_OTHER:
              if(getIRstate(IR_9BR_LR) != IS_50) {
                commandMotion(ROT_CW__SLOW);
                state = SD_FLAG_1_BEHIND_TWEAK_CW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Center and Right Lost 50-->Flag 1 Behind Tweak CW\r\n");
                #endif
              } else if(getIRstate(IR_9BL_LR) != IS_50) {
                commandMotion(ROT_CCW_SLOW);
                state = SD_FLAG_1_BEHIND_TWEAK_CCW;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Center and Left Lost 50-->Flag 1 Behind Tweak CCW\r\n");
                #endif
              }
              break;
            #ifndef IR_FOLLOW_TEST
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = SD_FLAG_1_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 1 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Find Black Line Behind\r\n");
              #endif
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FLAG_1_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case IR_9F_SR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = SD_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Center Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case IR_9B_SR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = SD_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Center Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef IR_CLOSE_BOTH_GOOD
        
        case SD_FLAG_1_AHEAD_TWEAK_CW:
        
          switch(event) {
            case IR_9FL_LR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = SD_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_TWEAK_CW:
        
          switch(event) {
            case IR_9BR_LR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = SD_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_AHEAD_APPROACH:
        
          switch(event) {
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
            #ifndef IR_FOLLOW_TEST
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = SD_FLAG_1_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 1 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Find Black Line Ahead\r\n");
              #endif
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_APPROACH:
        
          switch(event) {
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
            #ifndef IR_FOLLOW_TEST
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = SD_FLAG_1_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 1 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Find Black Line Behind\r\n");
              #endif
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FLAG_1_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case IR_9FR_LR_NOW_50:
              commandMotion(DRV_FWD_SLOW);
              state = SD_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case IR_9BL_LR_NOW_50:
              commandMotion(DRV_REV_SLOW);
              state = SD_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
                
        #ifdef IR_FAR_NEITHER_GOOD
        
        case SD_FLAG_1_AHEAD_TWEAK_CW:
        
          switch(event) {
            case IR_9FR_LR_NOW_0:
            case IR_9FR_LR_NOW_OTHER:
              commandMotion(DRV_FWD_SLOW);
              state = SD_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_TWEAK_CW:
        
          switch(event) {
            case IR_9BL_LR_NOW_0:
            case IR_9BL_LR_NOW_OTHER:
              commandMotion(DRV_REV_SLOW);
              state = SD_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_AHEAD_APPROACH:
        
          switch(event) {
            case IR_9FR_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw 50-->Flag 1 Ahead Tweak CW\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
            #ifndef IR_FOLLOW_TEST
            case BMP_F_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_FRONT, DR_CLSD);
              state = SD_FLAG_1_AHEAD_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Flag 1 Ahead Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Bump Felt Flag-->Find Black Line Ahead\r\n");
              #endif
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_APPROACH:
        
          switch(event) {
            case IR_9BL_LR_NOW_50:
              commandMotion(ROT_CW__SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw 50-->Flag 1 Behind Tweak CW\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
            #ifndef IR_FOLLOW_TEST
            case BMP_B_NOW_ON:
              #ifdef ACTIVE_CLAWS
              commandMotion(STOP_COMMAND);
              commandDoor(DR_BACK, DR_CLSD);
              state = SD_FLAG_1_BEHIND_CLOSING_DOOR;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Flag 1 Behind Closing Door\r\n");
              #endif
              #endif
              
              #ifdef PASSIVE_FLAPS
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Bump Felt Flag-->Find Black Line Behind\r\n");
              #endif
              #endif
              break;
            #endif
          }
          
          break;
          
        case SD_FLAG_1_AHEAD_TWEAK_CCW:
        
          switch(event) {
            case IR_9FL_LR_NOW_0:
            case IR_9FL_LR_NOW_OTHER:
              commandMotion(DRV_FWD_SLOW);
              state = SD_FLAG_1_AHEAD_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost 50-->Flag 1 Ahead Approach\r\n");
              #endif
              break;
            case IR_9BR_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_BEHIND_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw 50-->Flag 1 Behind Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_TWEAK_CCW:
        
          switch(event) {
            case IR_9BR_LR_NOW_0:
            case IR_9BR_LR_NOW_OTHER:
              commandMotion(DRV_REV_SLOW);
              state = SD_FLAG_1_BEHIND_APPROACH;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost 50-->Flag 1 Behind Approach\r\n");
              #endif
              break;
            case IR_9FL_LR_NOW_50:
              commandMotion(ROT_CCW_SLOW);
              state = SD_FLAG_1_AHEAD_TWEAK_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw 50-->Flag 1 Ahead Tweak CCW\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        case SD_FLAG_1_AHEAD_CLOSING_DOOR:
        
          switch(event) {
            case DR_F_NOW_CLSD:
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Door Closed-->Find Black Line Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FLAG_1_BEHIND_CLOSING_DOOR:
        
          switch(event) {
            case DR_B_NOW_CLSD:
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Door Closed-->Find Black Line Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FIND_BLACK_LINE_AHEAD:
        
          switch(event) {
            case TP_C_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              machine = SD_PARK;
              state = SD_ORIENT_BLACK_LINE_AHEAD_1;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Center Saw Black-->Orient Black Line Ahead 1\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = SD_BLACK_ON_FRONT_LEFT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black on Front Left\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = SD_BLACK_ON_FRONT_RIGHT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Black on Front Right\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_FIND_BLACK_LINE_BEHIND:
        
          switch(event) {
            case TP_C_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              machine = SD_PARK;
              state = SD_ORIENT_BLACK_LINE_BEHIND_1;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Center Saw Black-->Orient Black Line Behind 1\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = SD_BLACK_ON_BACK_LEFT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Black on Back Left\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = SD_BLACK_ON_BACK_RIGHT;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black on Back Right\r\n");
              #endif
              break;
          }
          
          break;
        
        case SD_BLACK_ON_FRONT_LEFT:
        
          switch(event) {
            case TP_FL_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw White-->Find Black Line Ahead\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_UNKNOWN_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Unknown Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_ON_FRONT_RIGHT:
        
          switch(event) {
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw White-->Find Black Line Ahead\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_UNKNOWN_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Unknown Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_ON_BACK_LEFT:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw White-->Find Black Line Behind\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_UNKNOWN_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Unknown Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_ON_BACK_RIGHT:
        
          switch(event) {
            case TP_BR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw White-->Find Black Line Behind\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_UNKNOWN_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Unknown Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
            
        case SD_UNKNOWN_GOAL_AHEAD:
        
          switch(event) {
            case IR_12F_NOW_30:
            case IR_12B_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Right Goal Ahead\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
            case IR_12B_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Right Goal Ahead\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case SD_UNKNOWN_GOAL_BEHIND:
        
          switch(event) {
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Right Goal Behind\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Right Goal Behind\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case SD_RIGHT_GOAL_AHEAD:
        
          switch(event) {
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              machine = SD_PARK;
              state = SD_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Driving to Goal Ahead\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Wrong Goal Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case SD_RIGHT_GOAL_BEHIND:
        
          switch(event) {
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              machine = SD_PARK;
              state = SD_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Driving to Goal Behind\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(awayDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(awayDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_WRONG_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Wrong Goal Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case SD_WRONG_GOAL_AHEAD:
        
          switch(event) {
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = SD_FIND_BLACK_LINE_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Find Black Line Behind\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(awayDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Right Goal Ahead\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(awayDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_AHEAD;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Right Goal Ahead\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case SD_WRONG_GOAL_BEHIND:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = SD_FIND_BLACK_LINE_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Find Black Line Ahead\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Right Goal Behind\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_RIGHT_GOAL_BEHIND;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Right Goal Behind\r\n");
                #endif
              }
              break;
          }
          
          break;
      }
      
      break;
      
    case SD_PARK:
    
      switch(state) {
        
        case SD_ORIENT_BLACK_LINE_AHEAD_1:
        
          switch(event) {
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_VERIFY_HOME_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Verify Home Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_ORIENT_BLACK_LINE_BEHIND_1:
        
          switch(event) {
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_VERIFY_HOME_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Verify Home Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_VERIFY_HOME_AHEAD:
        
          switch(event) {
            case IR_12F_NOW_30:
            case IR_12B_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Orient Black Line Ahead 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
            case IR_12B_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Orient Black Line Ahead 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case SD_VERIFY_HOME_BEHIND:
        
          switch(event) {
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Orient Black Line Behind 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Orient Black Line Behind 2\r\n");
                #endif
              } else {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        case SD_ORIENT_BLACK_LINE_AHEAD_2:
        
          switch(event) {
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_BLACK_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black Ahead Veer CCW\r\n");
              #endif
              break;
            case IR_12B_NOW_30:
            case IR_12F_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 30-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
            case IR_12B_NOW_70:
            case IR_12F_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_BEHIND_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Back Saw 70-->Orient Black Line Behind 2\r\n");
                #endif
              }
              break;
          }
          
          break;
          
        case SD_ORIENT_BLACK_LINE_BEHIND_2:
        
          switch(event) {
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_BLACK_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black Behind Veer CCW\r\n");
              #endif
              break;
            case IR_12F_NOW_30:
            case IR_12B_NOW_70:
              if(homeDC == 30) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 30-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
            case IR_12F_NOW_70:
            case IR_12B_NOW_30:
              if(homeDC == 70) {
                commandMotion(ROT_CW__FAST);
                state = SD_ORIENT_BLACK_LINE_AHEAD_2;
                #ifdef DEBUG_STATE_MACHINES
                (void)printf("Front Saw 70-->Orient Black Line Ahead 2\r\n");
                #endif
              }
              break;
          }
          
          break;
        
        #ifdef PARK_IN_COLOR
        
        case SD_BLACK_AHEAD_VEER_CW:
        
          switch(event) {
            case TP_FR_NOW_WHITE:
            case TP_FR_NOW_GREEN:
              commandMotion(DRV_FWD_FAST);
              state = SD_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_BEHIND_VEER_CW:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
            case TP_BL_NOW_GREEN:
              commandMotion(DRV_REV_FAST);
              state = SD_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_AHEAD_FOLLOW:
        
          switch(event) {
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_BLACK_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black Ahead Veer CCW\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = SD_BLACK_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Black Ahead Veer CW\r\n");
              #endif
              break;
            case TP_FL_NOW_GREEN:
            case TP_FR_NOW_GREEN:
              commandMotion(DRV_FWD_FAST);
              state = SD_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw Green-->Driving to Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_BEHIND_FOLLOW:
        
          switch(event) {
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_BLACK_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black Behind Veer CCW\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = SD_BLACK_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Black Behind Veer CW\r\n");
              #endif
              break;
            case TP_BL_NOW_GREEN:
            case TP_BR_NOW_GREEN:
              commandMotion(DRV_REV_FAST);
              state = SD_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw Green-->Driving to Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_AHEAD_VEER_CCW:
        
          switch(event) {
            case TP_FL_NOW_WHITE:
            case TP_FL_NOW_GREEN:
              commandMotion(DRV_FWD_FAST);
              state = SD_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_BEHIND_VEER_CCW:
        
          switch(event) {
            case TP_BR_NOW_WHITE:
            case TP_BR_NOW_GREEN:
              commandMotion(DRV_REV_FAST);
              state = SD_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
        
        #ifdef PARK_BLACK_WHITE
        
        case SD_BLACK_AHEAD_VEER_CW:
        
          switch(event) {
            case TP_FR_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = SD_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
            case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = SD_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw Black-->Driving to Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_BEHIND_VEER_CW:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = SD_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
            case TP_BR_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = SD_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw Black-->Driving to Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_AHEAD_FOLLOW:
        
          switch(event) {
            case TP_FL_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_BLACK_AHEAD_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Saw Black-->Black Ahead Veer CCW\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = SD_BLACK_AHEAD_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Right Saw Black-->Black Ahead Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_BEHIND_FOLLOW:
        
          switch(event) {
            case TP_BR_NOW_BLACK:
              commandMotion(ROT_CCW_FAST);
              state = SD_BLACK_BEHIND_VEER_CCW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Saw Black-->Black Behind Veer CCW\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(ROT_CW__FAST);
              state = SD_BLACK_BEHIND_VEER_CW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Left Saw Black-->Black Behind Veer CW\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_AHEAD_VEER_CCW:
        
          switch(event) {
            case TP_FL_NOW_WHITE:
              commandMotion(DRV_FWD_FAST);
              state = SD_BLACK_AHEAD_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Left Lost Black-->Black Ahead Follow\r\n");
              #endif
              break;
            case TP_FR_NOW_BLACK:
              case TP_FL_NOW_BLACK:
              commandMotion(DRV_FWD_FAST);
              state = SD_DRIVING_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw Black-->Driving to Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_BLACK_BEHIND_VEER_CCW:
        
          switch(event) {
            case TP_BR_NOW_WHITE:
              commandMotion(DRV_REV_FAST);
              state = SD_BLACK_BEHIND_FOLLOW;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Right Lost Black-->Black Behind Follow\r\n");
              #endif
              break;
            case TP_BL_NOW_BLACK:
              commandMotion(DRV_REV_FAST);
              state = SD_DRIVING_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw Black-->Driving to Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
        
        #endif
          
        case SD_DRIVING_GOAL_AHEAD:
        
          switch(event) {
            case TP_FL_NOW_WHITE:
            case TP_FR_NOW_WHITE:
              commandMotion(ROT_CW__FAST);
              state = SD_CENTERED_ON_GOAL_AHEAD;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Front Saw White-->Centered on Goal Ahead\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_DRIVING_GOAL_BEHIND:
        
          switch(event) {
            case TP_BL_NOW_WHITE:
            case TP_BR_NOW_WHITE:
              commandMotion(ROT_CW__FAST);
              state = SD_CENTERED_ON_GOAL_BEHIND;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Back Saw White-->Centered on Goal Behind\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_CENTERED_ON_GOAL_AHEAD:
        
          switch(event) {
            case TP_FR_NOW_BLACK:
            case TP_BL_NOW_WHITE:
              commandMotion(STOP_COMMAND);
              state = SD_PARKED_IN_GOAL;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Rotated 90 deg-->Parked in Goal\r\n");
              #endif
              break;
          }
          
          break;
          
        case SD_CENTERED_ON_GOAL_BEHIND:
        
          switch(event) {
            case TP_FR_NOW_WHITE:
            case TP_BL_NOW_BLACK:
              commandMotion(STOP_COMMAND);
              state = SD_PARKED_IN_GOAL;
              #ifdef DEBUG_STATE_MACHINES
              (void)printf("Rotated 90 deg-->Parked in Goal\r\n");
              #endif
              break;
          }
          
          break;
          
      }
      
      break;
      
  }
}