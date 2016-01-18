/* motion.h
 * ========
 * Module to interpret signals as motion events, and
 * handle those events by creating the proper response.
 */

#ifndef __MOTION__
#define __MOTION__

//Event constants
#define NO_EVENT -1
#define POWER_OFF -2

/* function CheckMotionEvents
 * ===============
 * Reads the location of each hand, and returns a directional event.
 * If the power is off, returns POWER_OFF. If there is no direction,
 * returns NO_EVENT.
 *
 * Returns:
 *   Constant indicating directional event, POWER_OFF, or NO_EVENT.
 */
char CheckMotionEvents(void);

/* function HandleMotionEvents
 * ===========================
 * Takes the motion event and sends the appropriate signal to the
 * Atari.
 *
 * Args:
 *   event: constant indicating directional event.
 */
void HandleMotionEvents(char event);

/* function HandleCheatEvents
 * ==========================
 * Takes the motion event and keeps track of the cheat status.
 * Once the cheat is entered, activates the cheat fans.
 *
 * Args:
 *   event: constant indicating directional event.
 */
void HandleCheatEvents(char event);

#endif
