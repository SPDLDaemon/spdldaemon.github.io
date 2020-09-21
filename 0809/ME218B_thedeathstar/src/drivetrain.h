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

#ifndef DRIVETRAIN_H
#define DRIVETRAIN_H

/*
 * Movement commands
 */
#define STOP_COMMAND 0
#define ROT_CCW_FAST 1
#define ROT_CCW_SLOW 2
#define ROT_CW__FAST 3
#define ROT_CW__SLOW 4
#define DRV_FWD_FAST 5
#define DRV_FWD_SLOW 6
#define DRV_REV_FAST 7
#define DRV_REV_SLOW 8

/*
 * Duty Cycles and Directions
 */
#define STOP_FWD_DUTY 0
#define STOP_REV_DUTY (100-STOP_FWD_DUTY)
#define LEFT_FWD_FAST 20
#define LEFT_FWD_SLOW 20
#define LEFT_REV_FAST (100-LEFT_FWD_FAST)
#define LEFT_REV_SLOW (100-LEFT_FWD_SLOW)
#define RGHT_FWD_FAST 20
#define RGHT_FWD_SLOW 20
#define RGHT_REV_FAST (100-RGHT_FWD_FAST)
#define RGHT_REV_SLOW (100-RGHT_FWD_SLOW)

/*
 * PWM Parameters
 */
#define PWM_RANGE FROM_400_TO_900 
#define PWM_FREQ HZ_500

/*
 * Pin Assignments
 */
#define LEFT_CHAN 3
#define RGHT_CHAN 4
#define DDR_DIRS_PT DDRU
#define PT_DIRS_PT PTU
#define LEFT_DIR_HI BIT5HI
#define LEFT_DIR_FWD BIT5LO
#define LEFT_DIR_REV LEFT_DIR_HI
#define RGHT_DIR_HI BIT2HI
#define RGHT_DIR_FWD BIT2LO
#define RGHT_DIR_REV RGHT_DIR_HI

/*
 * Initialization function.  Initializes the PWM subsystem, sets port/pin
 * directions as necessary, and makes sure that the robot starts stopped.
 * Must be run before sending motion commands.
 *
 * @author David Hoffert
 */
void drivetrainInit(void);

/*
 * Public function to accept movement commands.  Executes the movement
 * command passed in as a parameter.
 *
 * @param cmd The new motion to execute.
 *
 * @author David Hoffert
 */
void commandMotion(char cmd);

#endif