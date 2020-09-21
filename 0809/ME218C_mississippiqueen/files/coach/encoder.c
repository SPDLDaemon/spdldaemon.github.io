/*
 * Module to handle the rotary encoder input that represents
 * a steering command on the COACH.  Uses interrupts to keep
 * track of steering wheel position.  Offers a public function
 * to take the current position and package it up as a rotational
 * velocity command for the TOWRP (one nibble of bits.)
 *
 * @author David Hoffert
 */

#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include <stdio.h>
#include "S12eVec.h"
#include "encoder.h"
#include "TimerS12.h"

//#define TEST_ENCODER

#define TICKS_PER_COUNT 4

/*
 * Declare module variables
 */
static signed char encoderCnt = 0;
static signed char encoderSpc = 0;

/*
 * Initialize the encoder module by setting up input capture
 *
 * @author David Hoffert
 */
void initEncoder(void) {

  // Set PT0 and PU6 to be an input
  DDRT &= BIT0LO;
  DDRU &= BIT6LO;
  
  // Use the fastest possible clock rate for increased resolution
  TIM0_TSCR2 &= BIT2LO & BIT1LO & BIT0LO;
  
  // Enable the timer subsystem
  TIM0_TSCR1 = _S12_TEN;
  
  // Set up TIM0_TC4 (PT0) as an input capture
  TIM0_TIE |= _S12_C4I;                    // Enable interrupt
	TIM0_TCTL3 |= _S12_EDG4A;                // Capture rising edges only
	TIM0_TCTL3 &= ~_S12_EDG4B;
	//TIM0_TCTL3 |= (_S12_EDG4A | _S12_EDG4B); // Capture any edge
	TIM0_TFLG1 = _S12_C4F;                   // Clear flag
}

/*
 * Whenever Channel A has an edge, update encoder count
 *
 * @author David Hoffert
 */
void interrupt _Vec_tim0ch4 ChanA_IC(void) {
  
  // CLEAR THE DAMN FLAG!
  TIM0_TFLG1 = _S12_C4F;
  
  // If Channel A had a rising edge
  //if(PTT & BIT0HI) {
    
    // If Channel B is high
    if(PTU & BIT6HI) {
      
      // Counter-clockwise --> Left --> Positive
      encoderSpc++;
      if(encoderSpc%TICKS_PER_COUNT == 0) encoderCnt++;
      
      // Saturate the encoder count at 7 (there are 7 levels)
      // Wheel other way --> something should happen immediately
      if(encoderCnt > 7) {
        encoderCnt = 7;
        encoderSpc--;
      }
    }
    
    // If Channel B is low
    else {
      
      // Clockwise --> Right --> Negative
      encoderSpc--;
      if(encoderSpc%TICKS_PER_COUNT == 0) encoderCnt--;
      
      // Saturate the encoder count at -7 (there are 7 levels)
      // Wheel other way --> something should happen immediately
      if(encoderCnt < -7) {
        encoderCnt = -7;
        encoderSpc++;
      }
    }
  //}
  /*
  // If Channel A had a falling edge
  else {
    
    // If Channel B is high
    if(PTU & BIT6HI) {
      
      // Clockwise --> Right --> Negative
      encoderCnt--;
      
      // Saturate the encoder count at -7 (there are 7 levels)
      // Wheel other way --> something should happen immediately
      if(encoderCnt < -7) encoderCnt = -7;
    }
    
    // If Channel B is low
    else {
      
      // Counter-clockwise --> Left --> Positive
      encoderCnt++;
      
      // Saturate the encoder count at 7 (there are 7 levels)
      // Wheel other way --> something should happen immediately
      if(encoderCnt > 7) encoderCnt = 7;
    }
  }
  */
}

/*
 * Convert current encoder count to a protocol-following nibble
 *
 * @author David Hoffert
 */
char getSteering(void) {
  
  // Declare local variables
  char lowerNibble;
  
  // Translate encoder count into lower nibble
  switch(encoderCnt) {
    case -7: 
             lowerNibble = 0b00001111; // 0x0F
             break;
    case -6: 
             lowerNibble = 0b00001110; // 0x0E
             break;
    case -5: 
             lowerNibble = 0b00001101; // 0x0D
             break;
    case -4: 
             lowerNibble = 0b00001100; // 0x0C
             break;
    case -3: 
             lowerNibble = 0b00001011; // 0x0B
             break;
    case -2: 
             lowerNibble = 0b00001010; // 0x0A
             break;
    case -1: 
             lowerNibble = 0b00001001; // 0x09
             break;
    case 0: 
             lowerNibble = 0b00000000; // 0x00
             break;
    case 1:  
             lowerNibble = 0b00000001; // 0x01
             break;
    case 2: 
             lowerNibble = 0b00000010; // 0x02
             break;
    case 3: 
             lowerNibble = 0b00000011; // 0x03
             break;
    case 4: 
             lowerNibble = 0b00000100; // 0x04
             break;
    case 5: 
             lowerNibble = 0b00000101; // 0x05
             break;
    case 6: 
             lowerNibble = 0b00000110; // 0x06
             break;
    case 7: 
             lowerNibble = 0b00000111; // 0x07
             break;
  }
  
  // Send the little guy off into the big world
  return lowerNibble;
}

#ifdef TEST_ENCODER

void main(void) {
  
  unsigned int Encoder_Time = 0;
  
  TMRS12_Init(TMRS12_RATE_16MS);
  
  // Initialize the encoder module
  initEncoder();
  EnableInterrupts;
  
  Encoder_Time = TMRS12_GetTime();
  
  // Print out the encoder count and corresponding nibble
  while(1) {
    
    if((TMRS12_GetTime() - Encoder_Time) > 61) {
      
      (void)printf("Encoder at %d --> nibble is %#x\n\r",
                 encoderCnt, getSteering());
                 
      Encoder_Time = TMRS12_GetTime();
    }
  }
}

#endif