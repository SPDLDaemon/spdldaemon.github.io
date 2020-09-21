/*
 * Seven-segment display module for the COACH. Uses SPI to send a command
 * to a shift register, which powers the seven-segment display.  Two
 * seven-segment displays are needed, so bi-directional mode is used
 * to still only need the one jumper of pins.
 *
 * @author David Hoffert
 */
 
#ifndef SEG7_DISP_H
#define SEG7_DISP_H

// Bits are     ABCDEFGP
#define NTHNG 0b00000000
#define POS_0 0b11111100
#define POS_1 0b01100000
#define POS_2 0b11011010
#define POS_3 0b11110010
#define POS_4 0b01100110
#define POS_5 0b10110110
#define POS_6 0b10111110
#define POS_7 0b11100000
#define POS_8 0b11111110
#define POS_9 0b11110110
#define POS_A 0b11101110
#define POS_B 0b00111110
#define POS_C 0b10011100
#define POS_D 0b01111010
#define POS_E 0b10011110
#define POS_F 0b10001110
#define NEG_0 0b11111101
#define NEG_1 0b01100001
#define NEG_2 0b11011011
#define NEG_3 0b11110011
#define NEG_4 0b01100111
#define NEG_5 0b10110111
#define NEG_6 0b10111111
#define NEG_7 0b11100001
#define NEG_8 0b11111111
#define NEG_9 0b11110111
#define NEG_A 0b11101111
#define NEG_B 0b00111111
#define NEG_C 0b10011101
#define NEG_D 0b01111011
#define NEG_E 0b10011111
#define NEG_F 0b10001111

#define TOWRP_DISP 0
#define MORALE_DISP 1
#define BOTH 2

#define RAISE_TOWRP_SS BIT7HI
#define RAISE_MORALE_SS BIT4HI

#define FAIL_DISP 0
#define SUCC_DISP 1

/*
 * Initialize the SPI system that will run the seven-segment displays.
 *
 * @author David Hoffert
 */
void initSeg7(void);

/*
 * Display a number on a seven-segment display
 *
 * @param number #define representing the number to display
 *        display #define for which display to change
 *
 * @return Whether the transmission was successful
 *
 * @author David Hoffert
 */
char dispNum(char number, char display);

/*
 * Convert a number to the appropriate #define
 *
 * @param number The number to convert
 *
 * @return The corresponding #define
 *
 * @author David Hoffert
 */
char convNum(char number);

#endif