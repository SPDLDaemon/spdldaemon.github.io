/*
 * Drivetrain Module for Capture the Flag
 *
 * This is the drivetrain module for capture the flag.  It provides a
 * public function to accept and execute movement commands from other
 * modules.  The available commands are rotate counter-clockwise, rotate
 * clockwise, drive forward, drive in reverse, and stop.  Each movement
 * command has two available speeds, fast and slow.  This module uses
 * two PWM pins for PWM and two output pins for direction control.
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
#include "pwm_e128.h"
#include "drivetrain.h"

// Test Harness Includes 
#include <termio.h>
#include <stdio.h>

// Select which main function to run
//#define TEST_DRIVETRAIN

/*
 * Initialization function.  Initializes the PWM subsystem, sets port/pin
 * directions as necessary, and makes sure that the robot starts stopped.
 * Must be run before sending motion commands.
 *
 * @author David Hoffert
 */
void drivetrainInit() {
  
  // Initialize the PWM subsystem for the left and right drive channels
  pwm_sys_init(PWM_RANGE, PWM_RANGE);
  pwm_chan_init(LEFT_CHAN, PWM_FREQ);
  pwm_chan_init(RGHT_CHAN, PWM_FREQ);
   
  // Set the data direction of the pins used for direction control
  DDR_DIRS_PT = LEFT_DIR_HI;
  DDR_DIRS_PT |= RGHT_DIR_HI;
  
  // Make sure the robot starts stopped
  commandMotion(STOP_COMMAND);
}

/*
 * Public function to accept movement commands.  Executes the movement
 * command passed in as a parameter.
 *
 * @param cmd The new motion to execute.
 *
 * @author David Hoffert
 */
void commandMotion(char cmd) {
  
  // Sanity check
  //(void)printf("Sending motion command\r\n");
  
  // Execute the given command
  switch(cmd) {
  
    // Stop is both wheels at 0% duty cycle
    case STOP_COMMAND: PT_DIRS_PT &= LEFT_DIR_FWD; 
                       pwm_change_dc(LEFT_CHAN, STOP_FWD_DUTY);
                       PT_DIRS_PT &= RGHT_DIR_FWD;
                       pwm_change_dc(RGHT_CHAN, STOP_FWD_DUTY);
                       break;
    
    // CCW is left backward and right forward
    case ROT_CCW_FAST: PT_DIRS_PT |= LEFT_DIR_REV; 
                       pwm_change_dc(LEFT_CHAN, LEFT_REV_FAST);
                       PT_DIRS_PT &= RGHT_DIR_FWD;
                       pwm_change_dc(RGHT_CHAN, RGHT_FWD_FAST);
                       break;
                       
    case ROT_CCW_SLOW: PT_DIRS_PT |= LEFT_DIR_REV; 
                       pwm_change_dc(LEFT_CHAN, LEFT_REV_SLOW);
                       PT_DIRS_PT &= RGHT_DIR_FWD;
                       pwm_change_dc(RGHT_CHAN, RGHT_FWD_SLOW);
                       break;
    
    // CW is left forward and right backward
    case ROT_CW__FAST: PT_DIRS_PT &= LEFT_DIR_FWD; 
                       pwm_change_dc(LEFT_CHAN, LEFT_FWD_FAST);
                       PT_DIRS_PT |= RGHT_DIR_REV;
                       pwm_change_dc(RGHT_CHAN, RGHT_REV_FAST);
                       break;
                       
    case ROT_CW__SLOW: PT_DIRS_PT &= LEFT_DIR_FWD; 
                       pwm_change_dc(LEFT_CHAN, LEFT_FWD_SLOW);
                       PT_DIRS_PT |= RGHT_DIR_REV;
                       pwm_change_dc(RGHT_CHAN, RGHT_REV_SLOW);
                       break;
        
    // Forward is both wheels forward
    case DRV_FWD_FAST: PT_DIRS_PT &= LEFT_DIR_FWD; 
                       pwm_change_dc(LEFT_CHAN, LEFT_FWD_FAST);
                       PT_DIRS_PT &= RGHT_DIR_FWD;
                       pwm_change_dc(RGHT_CHAN, RGHT_FWD_FAST);
                       break;
                       
    case DRV_FWD_SLOW: PT_DIRS_PT &= LEFT_DIR_FWD; 
                       pwm_change_dc(LEFT_CHAN, LEFT_FWD_SLOW);
                       PT_DIRS_PT &= RGHT_DIR_FWD;
                       pwm_change_dc(RGHT_CHAN, RGHT_FWD_SLOW);
                       break;
    
    // Backward is both wheels backward
    case DRV_REV_FAST: PT_DIRS_PT |= LEFT_DIR_REV; 
                       pwm_change_dc(LEFT_CHAN, LEFT_REV_FAST);
                       PT_DIRS_PT |= RGHT_DIR_REV;
                       pwm_change_dc(RGHT_CHAN, RGHT_REV_FAST);
                       break;
                       
    case DRV_REV_SLOW: PT_DIRS_PT |= LEFT_DIR_REV; 
                       pwm_change_dc(LEFT_CHAN, LEFT_REV_SLOW);
                       PT_DIRS_PT |= RGHT_DIR_REV;
                       pwm_change_dc(RGHT_CHAN, RGHT_REV_SLOW);
                       break;
  }
}

/*
 * Test harness for the drivetrain module
 *
 * @author David Hoffert
 */
#ifdef TEST_DRIVETRAIN

void main() {

  // Declare local variables
  char cmd = 0;

  // Initialize the necessary subsystems
  TERMIO_Init();
  drivetrainInit();

  // Indicate that this is the test harness for the drivetrain
  (void)printf("Welcome to the drivetrain test harness\r\n");
  (void)printf("Press w,a,s,d,x for fast; press i,j,k,l,m for slow\r\n");

  // Enter a blocking loop for getting user input
  while(1) {
    
    // Get a command
    cmd = TERMIO_GetChar();
  
    // Execute the command
    switch(cmd) {
    
      case 'w': 
        (void)printf("Pressed w\r\n");
        commandMotion(DRV_FWD_FAST);
        break;
        
      case 'a': 
        (void)printf("Pressed a\r\n");
        commandMotion(ROT_CCW_FAST);
        break;
        
      case 's': 
        (void)printf("Pressed s\r\n");
        commandMotion(DRV_REV_FAST);
        break;
        
      case 'd': 
        (void)printf("Pressed d\r\n");
        commandMotion(ROT_CW__FAST);
        break;
        
      case 'x': 
        (void)printf("Pressed x\r\n");
        commandMotion(STOP_COMMAND);
        break;
        
      case 'i': 
        (void)printf("Pressed i\r\n");
        commandMotion(DRV_FWD_SLOW);
        break;
        
      case 'j': 
        (void)printf("Pressed j\r\n");
        commandMotion(ROT_CCW_SLOW);
        break;
        
      case 'k': 
        (void)printf("Pressed k\r\n");
        commandMotion(DRV_REV_SLOW);
        break;
        
      case 'l': 
        (void)printf("Pressed l\r\n");
        commandMotion(ROT_CW__SLOW);
        break;
        
      case 'm': 
        (void)printf("Pressed m\r\n");
        commandMotion(STOP_COMMAND);
        break;
      default:
        (void)printf("Invalid character\r\n");
        commandMotion(STOP_COMMAND);
        break;
    }
  }
}

#endif