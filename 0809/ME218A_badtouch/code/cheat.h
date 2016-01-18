#ifndef __CHEAT__
#define __CHEAT__

/* CheatInit
 * =========
 * Initializes the port used to control the fan
 * that goes on when the cheat is activated.
 */
void InitCheat(void);

/* ExecuteCheat
 * ============
 * Turns on and off the fans.
 *
 * Args:
 *   state:	The state of the servo (ON or OFF)
 */  
void ExecuteCheat(char state);

#endif
