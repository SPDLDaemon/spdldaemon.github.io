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

#ifndef EVENTS_H
#define EVENTS_H

/*
 * Sensor states and thresholds
 */

// IR sensor states
#define IS_0 0
#define IS_30 1
#define IS_50 2
#define IS_70 3
#define IS_OTHER 4

// IR sensor thresholds
#define UPPER_0 5
#define LOWER_30 25
#define UPPER_30 35
#define LOWER_50 40
#define UPPER_50 60
#define LOWER_70 65
#define UPPER_70 75
  
/*
 * Events
 */

#define NO_EVENT 0

// IR events
#define IR_12F_NOW_0 1
#define IR_12F_NOW_30 2
#define IR_12F_NOW_70 3
#define IR_12F_NOW_OTHER 4
#define IR_12B_NOW_0 5
#define IR_12B_NOW_30 6
#define IR_12B_NOW_70 7
#define IR_12B_NOW_OTHER 8
#define IR_9FL_LR_NOW_0 9
#define IR_9FL_LR_NOW_50 10
#define IR_9FL_LR_NOW_OTHER 11
#define IR_9FR_LR_NOW_0 12
#define IR_9FR_LR_NOW_50 13
#define IR_9FR_LR_NOW_OTHER 14
#define IR_9F_SR_NOW_0 15
#define IR_9F_SR_NOW_50 16
#define IR_9F_SR_NOW_OTHER 17
#define IR_9BL_LR_NOW_0 18
#define IR_9BL_LR_NOW_50 19
#define IR_9BL_LR_NOW_OTHER 20
#define IR_9BR_LR_NOW_0 21
#define IR_9BR_LR_NOW_50 22
#define IR_9BR_LR_NOW_OTHER 23
#define IR_9B_SR_NOW_0 24
#define IR_9B_SR_NOW_50 25
#define IR_9B_SR_NOW_OTHER 26

// Tape events
#define TP_C_NOW_BLACK 27
#define TP_C_NOW_GREEN 28
#define TP_C_NOW_WHITE 29
#define TP_FL_NOW_BLACK 30
#define TP_FL_NOW_GREEN 31
#define TP_FL_NOW_WHITE 32
#define TP_FR_NOW_BLACK 33
#define TP_FR_NOW_GREEN 34
#define TP_FR_NOW_WHITE 35
#define TP_BL_NOW_BLACK 36
#define TP_BL_NOW_GREEN 37
#define TP_BL_NOW_WHITE 38
#define TP_BR_NOW_BLACK 39
#define TP_BR_NOW_GREEN 40
#define TP_BR_NOW_WHITE 41

// Other events
#define FLSH_NOW_ON 42
#define FLSH_NOW_OFF 43
#define BMP_F_NOW_ON 44
#define BMP_F_NOW_OFF 45
#define BMP_B_NOW_ON 46
#define BMP_B_NOW_OFF 47
#define DR_F_NOW_CLSD 48
#define DR_F_NOW_OPEN 49
#define DR_B_NOW_CLSD 50
#define DR_B_NOW_OPEN 51
#define TIMER_2MIN_EXP 52

/*
 * Public function to return the current state of the IR sensors,
 * since the IR module does not provide this and the state machine
 * needs it for guarding.
 *
 * @author David Hoffert
 */
char getIRstate(char sensor);

/*
 * Public function to check for events.  Polls all low-level sensor
 * modules for changes in states, and reports back the first event it
 * finds.
 *
 * @author David Hoffert
 */
char checkEvents(void);

#endif