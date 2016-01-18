/* atari.h
 * =======
 */

#ifndef __ATARI__
#define __ATARI__

//Button/pin constants
#define UP      6
#define DOWN    5
#define RIGHT   4
#define LEFT    3
#define BUTTON  2

//Interface

/* AtariInit
 * =============
 * Initializes the ports for use with the Atari
 * interface.
 */
void AtariInit(void);

/* ToggleButton
 * ============
 * Changes the state of a button. Can specify either on,
 * off, or toggle.
 *
 * Args:
 *   state: A constant specifying what state to make the
 *          button. Takes ON, OFF, or TOGGLE.
 */
void ToggleButton(char button, char state);

/* GetPowerStatus
 * ==============
 * Gives whether the power status from the protection
 * board is HI or LO.
 *
 * Returns:
 *   The power status. Either HI or LO.
 */
char GetPowerStatus(void);

#endif