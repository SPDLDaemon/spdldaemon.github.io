/*
 * State Machine Module for Capture the Flag
 *
 * This is the state machine module for capture the flag.  It contains
 * public functions for executing both the main play state machine and
 * the sudden death state machine.
 *
 * @author David Hoffert
 */

#ifndef STATE_MACHINES_H
#define STATE_MACHINES_H

/*
 * Which state machine to operate under
 */
#define MAIN_PLAY 0
#define SUDDEN_DEATH 1

/*
 * Which team to play for
 */
#define RED_TEAM 2
#define GREEN_TEAM 3

/*
 * Main play state machine sub-machines
 */
#define MP_FLAG_1 0
#define MP_FLAG_2 1
#define MP_PARK 2

/*
 * Main play state machine states (54 and counting...)
 */
#define MP_WAITING_FOR_FLASH 0
#define MP_NO_DATA 1
#define MP_FRONT_SAW_HOME 2
#define MP_BACK_SAW_HOME 3
#define MP_FRONT_SAW_FLAG 4
#define MP_BACK_SAW_FLAG 5
#define MP_SEEKING_FLAG_1_AHEAD 6
#define MP_SEEKING_FLAG_1_BEHIND 7
#define MP_FLAG_1_AHEAD_VEER_CW 8
#define MP_FLAG_1_BEHIND_VEER_CW 9
#define MP_FLAG_1_AHEAD_FOLLOW 10
#define MP_FLAG_1_BEHIND_FOLLOW 11
#define MP_FLAG_1_AHEAD_VEER_CCW 12
#define MP_FLAG_1_BEHIND_VEER_CCW 13
#define MP_FLAG_1_AHEAD_TWEAK_CW 14
#define MP_FLAG_1_BEHIND_TWEAK_CW 15
#define MP_FLAG_1_AHEAD_APPROACH 16
#define MP_FLAG_1_BEHIND_APPROACH 17
#define MP_FLAG_1_AHEAD_TWEAK_CCW 18
#define MP_FLAG_1_BEHIND_TWEAK_CCW 19
#define MP_FLAG_1_AHEAD_CLOSING_DOOR 20
#define MP_FLAG_1_BEHIND_CLOSING_DOOR 21
#define MP_SEEKING_FLAG_2_AHEAD 22
#define MP_SEEKING_FLAG_2_BEHIND 23
#define MP_FLAG_2_AHEAD_VEER_CW 24
#define MP_FLAG_2_BEHIND_VEER_CW 25
#define MP_FLAG_2_AHEAD_FOLLOW 26
#define MP_FLAG_2_BEHIND_FOLLOW 27
#define MP_FLAG_2_AHEAD_VEER_CCW 28
#define MP_FLAG_2_BEHIND_VEER_CCW 29
#define MP_FLAG_2_AHEAD_TWEAK_CW 30
#define MP_FLAG_2_BEHIND_TWEAK_CW 31
#define MP_FLAG_2_AHEAD_APPROACH 32
#define MP_FLAG_2_BEHIND_APPROACH 33
#define MP_FLAG_2_AHEAD_TWEAK_CCW 34
#define MP_FLAG_2_BEHIND_TWEAK_CCW 35
#define MP_FLAG_2_AHEAD_CLOSING_DOOR 36
#define MP_FLAG_2_BEHIND_CLOSING_DOOR 37
#define MP_FIND_BLACK_LINE_AHEAD 38
#define MP_FIND_BLACK_LINE_BEHIND 39
#define MP_BLACK_ON_FRONT_LEFT 40
#define MP_BLACK_ON_FRONT_RIGHT 41
#define MP_BLACK_ON_BACK_LEFT 42
#define MP_BLACK_ON_BACK_RIGHT 43
#define MP_UNKNOWN_GOAL_AHEAD 44
#define MP_UNKNOWN_GOAL_BEHIND 45
#define MP_RIGHT_GOAL_AHEAD 46
#define MP_RIGHT_GOAL_BEHIND 47
#define MP_WRONG_GOAL_AHEAD 48
#define MP_WRONG_GOAL_BEHIND 49
#define MP_ORIENT_BLACK_LINE_AHEAD_1 50
#define MP_ORIENT_BLACK_LINE_BEHIND_1 51
#define MP_VERIFY_HOME_AHEAD 52
#define MP_VERIFY_HOME_BEHIND 53
#define MP_ORIENT_BLACK_LINE_AHEAD_2 54
#define MP_ORIENT_BLACK_LINE_BEHIND_2 55
#define MP_BLACK_AHEAD_VEER_CW 56
#define MP_BLACK_BEHIND_VEER_CW 57
#define MP_BLACK_AHEAD_FOLLOW 58
#define MP_BLACK_BEHIND_FOLLOW 59
#define MP_BLACK_AHEAD_VEER_CCW 60
#define MP_BLACK_BEHIND_VEER_CCW 61
#define MP_DRIVING_GOAL_AHEAD 62
#define MP_DRIVING_GOAL_BEHIND 63
#define MP_CENTERED_ON_GOAL_AHEAD 64
#define MP_CENTERED_ON_GOAL_BEHIND 65
#define MP_PARKED_IN_GOAL 66
#define MP_TIME_EXPIRED 67

/*
 * Sudden death state machine sub-machines
 */
#define SD_ORIENT 0
#define SD_GET_FLAG 1
#define SD_PARK 2

/*
 * Sudden death state machine states (39 states)
 */
#define SD_WAITING_FOR_FLASH 0
#define SD_NO_DATA 1
#define SD_FRONT_SAW_HOME 2
#define SD_BACK_SAW_HOME 3
#define SD_FRONT_SAW_FLAG 4
#define SD_BACK_SAW_FLAG 5
#define SD_FIND_GREEN_LINE_AHEAD_1 6
#define SD_FIND_GREEN_LINE_BEHIND_1 7
#define SD_FIND_GREEN_LINE_AHEAD_2 8
#define SD_FIND_GREEN_LINE_BEHIND_2 9
#define SD_ORIENT_GREEN_LINE_AHEAD 10
#define SD_ORIENT_GREEN_LINE_BEHIND 11
#define SD_GREEN_AHEAD_VEER_CW 12
#define SD_GREEN_BEHIND_VEER_CW 13
#define SD_GREEN_AHEAD_FOLLOW 14
#define SD_GREEN_BEHIND_FOLLOW 15
#define SD_GREEN_AHEAD_VEER_CCW 16
#define SD_GREEN_BEHIND_VEER_CCW 17
#define SD_ROTATE_TO_FLAG_AHEAD 18
#define SD_ROTATE_TO_FLAG_BEHIND 19
#define SD_FLAG_1_AHEAD_TWEAK_CW 20
#define SD_FLAG_1_BEHIND_TWEAK_CW 21
#define SD_FLAG_1_AHEAD_APPROACH 22
#define SD_FLAG_1_BEHIND_APPROACH 23
#define SD_FLAG_1_AHEAD_TWEAK_CCW 24
#define SD_FLAG_1_BEHIND_TWEAK_CCW 25
#define SD_FLAG_1_AHEAD_CLOSING_DOOR 26
#define SD_FLAG_1_BEHIND_CLOSING_DOOR 27
#define SD_FIND_BLACK_LINE_AHEAD 28
#define SD_FIND_BLACK_LINE_BEHIND 29
#define SD_BLACK_ON_FRONT_LEFT 30
#define SD_BLACK_ON_FRONT_RIGHT 31
#define SD_BLACK_ON_BACK_LEFT 32
#define SD_BLACK_ON_BACK_RIGHT 33
#define SD_UNKNOWN_GOAL_AHEAD 34
#define SD_UNKNOWN_GOAL_BEHIND 35
#define SD_RIGHT_GOAL_AHEAD 36
#define SD_RIGHT_GOAL_BEHIND 37
#define SD_WRONG_GOAL_AHEAD 38
#define SD_WRONG_GOAL_BEHIND 39
#define SD_ORIENT_BLACK_LINE_AHEAD_1 40
#define SD_ORIENT_BLACK_LINE_BEHIND_1 41
#define SD_VERIFY_HOME_AHEAD 42
#define SD_VERIFY_HOME_BEHIND 43
#define SD_ORIENT_BLACK_LINE_AHEAD_2 44
#define SD_ORIENT_BLACK_LINE_BEHIND_2 45
#define SD_BLACK_AHEAD_VEER_CW 46
#define SD_BLACK_BEHIND_VEER_CW 47
#define SD_BLACK_AHEAD_FOLLOW 48
#define SD_BLACK_BEHIND_FOLLOW 49
#define SD_BLACK_AHEAD_VEER_CCW 50
#define SD_BLACK_BEHIND_VEER_CCW 51
#define SD_DRIVING_GOAL_AHEAD 52
#define SD_DRIVING_GOAL_BEHIND 53
#define SD_CENTERED_ON_GOAL_AHEAD 54
#define SD_CENTERED_ON_GOAL_BEHIND 55
#define SD_PARKED_IN_GOAL 56

/*
 * Public function to determine which state machine should be run, by
 * reading the state of a physical switch on the robot.
 *
 * @return Which state machine should be run this time
 *
 * @author David Hoffert
 */
char getStateMachine(void);

/*
 * Public function to determine which team the robot is playing for, and
 * to set the corresponding variables internal to this module, by reading
 * the state of a physical switch on the robot.
 *
 * @return Which team the robot is playing for this time
 *
 * @author David Hoffert
 */
char getTeam(void);

/*
 * Public function to execute the main play state machine.  Takes an
 * event to respond to and returns nothing.
 *
 * @param event The event for the state machine to react to
 *
 * @author David Hoffert
 */
void mainPlayStateMachine(char event);

/*
 * Public function to execute the sudden death state machine.  Takes an
 * event to respond to and returns nothing.
 *
 * @param event The event for the state machine to react to
 *
 * @author David Hoffert
 */
void suddenDeathStateMachine(char event);

#endif