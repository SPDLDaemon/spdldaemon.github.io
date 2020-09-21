/*
 * Module to handle the rotary encoder input that represents
 * a steering command on the COACH.  Uses interrupts to keep
 * track of steering wheel position.  Offers a public function
 * to take the current position and package it up as a rotational
 * velocity command for the TOWRP (one nibble of bits.)
 *
 * @author David Hoffert
 */

#ifndef ENCODER_H
#define ENCODER_H

/*
 * Initialize the encoder module by setting up input capture
 *
 * @author David Hoffert
 */
void initEncoder(void);

/*
 * Convert current encoder count to a protocol-following nibble
 *
 * @author David Hoffert
 */
char getSteering(void);

#endif

