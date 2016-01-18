/* our_c32.h
 * =====
 * Module for easier interface with C32.
 */

#ifndef __OUR_C32__
#define __OUR_C32__

//Port constants
#define T       0
#define M       1
#define AD      2

//Data direction constants
#define READ    0
#define WRITE   1
#define ANALOG  2

//Data constants
#define LO      0
#define HI      1

//Utility constants
#define OFF     LO
#define ON      HI
#define TOGGLE  2

//Return code constants
#define SUCCESS 1
#define ERROR   -1

/* function SetDDR
 * ===============
 * Sets the data direction of the specified port and pin.
 * To set the data direction of an entire port at once,
 * pass in -1 as the pin, and the hex char value as
 * direction (instead of READ or WRITE). This does not
 * work for the AD pins.
 *
 * Args:
 *   port: constant of port to set
 *   pin: pin numberto set (typically 0-7), or -1 to set
 *        all (doesn't work for AD).
 *   direction: DDR constant, or the hex of all DDRs.
 *
 * Returns:
 *   success code
 */
char SetDDR(char port, char pin, char direction);

/* function ReadPort
 * =================
 * Reads the value of the specified port and pin.
 * A quirk: can't read in from analog pins set to 'I'.
 *
 * Args:
 *   port: constant of port to read
 *   pin: pin number to read, -1 to read all
 *        (does not work for AD)
 *
 * Returns:
 *   HI, LO, AD value, hex of values of entire port,
 *   or ERROR.
 */
short ReadPort(char port, char pin);

/* function WritePort
 * ==================
 *  Writes passed value to the specified port and pin.
 *
 * Args:
 *   port: constant of port to write to
 *   pin: pin number to write to, -1 to write
 *        to all
 *   value: value to write, or hex of all values
 *
 * Return:
 *   Success constant.
 */
char WritePort(char port, char pin, char value);

#endif
