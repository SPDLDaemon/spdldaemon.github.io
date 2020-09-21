/*
 * Seven-segment display module for the COACH. Uses SPI to send a command
 * to a shift register, which powers the seven-segment display.  Two
 * seven-segment displays are needed, so bi-directional mode is used
 * to still only need the one jumper of pins.
 *
 * @author David Hoffert
 */

#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include <stdio.h>
#include "S12eVec.h"
#include "seg7_display.h"
#include "TimerS12.h"

//#define TEST_SEG7

/*
 * Initialize the SPI system that will run the seven-segment displays.
 *
 * @author David Hoffert
 */
void initSeg7(void) {
  
  // Set this processor to be the master
  SPICR1 |= BIT4HI;
  
  // Based on how we drew the schematic, things will be easier with LSB first
  SPICR1 |= BIT0HI;
  
  // Set the baud rate to the slowest possible
  SPIBR = 0x77;
  
  // The shift register samples lo-hi, so mode 3 is the one for us
  SPICR1 |= BIT2HI | BIT3HI;
  
  // Since we have two slaves, we need to manually control the SS line
  // We'll also use bidirectional mode to save a pin (and since we can)
  SPICR1 &= BIT1LO;
  SPICR2 &= BIT4LO;
  SPICR2 |= BIT0HI | BIT3HI;
  
  // We therefore need to set the data directions of the unused pins
  DDRS |= RAISE_MORALE_SS | RAISE_TOWRP_SS;
  
  // Enable the SPI system
  SPICR1 |= BIT6HI;
  
  // Send a first message so that the flag gets set
  while(!(SPISR & _S12_SPTEF))
    ;
  PTS |= RAISE_TOWRP_SS;
  PTS |= RAISE_MORALE_SS;
  SPIDR = NTHNG;
  PTS &= ~RAISE_MORALE_SS;
  PTS &= ~RAISE_TOWRP_SS;
}

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
char dispNum(char number, char display) {
  
  // If the previous transmission completed, send the new one
  if(SPISR & _S12_SPTEF) {
    
    // Send to the appropriate display
    switch(display) {
      
      case TOWRP_DISP:  
                        PTS &= ~RAISE_MORALE_SS;
                        PTS |= RAISE_TOWRP_SS;
                        SPIDR = number;
                        return SUCC_DISP;
                        break;
      case MORALE_DISP: 
                        PTS &= ~RAISE_TOWRP_SS;
                        PTS |= RAISE_MORALE_SS;
                        SPIDR = number;
                        return SUCC_DISP;
                        break;
      case BOTH:        
                        PTS |= RAISE_TOWRP_SS;
                        PTS |= RAISE_MORALE_SS;
                        SPIDR = number;
                        return SUCC_DISP;
                        break;
      default:          
                        return FAIL_DISP;
                        break;
    }
  } else return FAIL_DISP;
}

/*
 * Convert a number to the appropriate #define
 *
 * @param number The number to convert
 *
 * @return The corresponding #define
 *
 * @author David Hoffert
 */
char convNum(char number) {
  
  switch(number) {
    case -15: return NEG_F;
    case -14: return NEG_E;
    case -13: return NEG_D;
    case -12: return NEG_C;
    case -11: return NEG_B;
    case -10: return NEG_A;
    case -9:  return NEG_9;
    case -8:  return NEG_8;
    case -7:  return NEG_7;
    case -6:  return NEG_6;
    case -5:  return NEG_5;
    case -4:  return NEG_4;
    case -3:  return NEG_3;
    case -2:  return NEG_2;
    case -1:  return NEG_1;
    case 0:   return POS_0;
    case 1:   return POS_1;
    case 2:   return POS_2;
    case 3:   return POS_3;
    case 4:   return POS_4;
    case 5:   return POS_5;
    case 6:   return POS_6;
    case 7:   return POS_7;
    case 8:   return POS_8;
    case 9:   return POS_9;
    case 10:  return POS_A;
    case 11:  return POS_B;
    case 12:  return POS_C;
    case 13:  return POS_D;
    case 14:  return POS_E;
    case 15:  return POS_F;
  }
}

/*
 * Main function test harness
 *
 * @author David Hoffert
 */
#ifdef TEST_SEG7

void main(void) {
  
  unsigned int Seg7_Time = 0;
  char i = 0;
  char j = 0;
  char display = TOWRP_DISP;
  
  TMRS12_Init(TMRS12_RATE_16MS);
  
  // Initialize the seg7 module
  initSeg7();
  EnableInterrupts;
  
  // Test alternating displays
  Seg7_Time = TMRS12_GetTime();
  while(i<32) {
  
    if((TMRS12_GetTime() - Seg7_Time) > 61) {
      
      Seg7_Time = TMRS12_GetTime();    
      (void)dispNum(convNum(i>>1),TOWRP_DISP);
      
      i++;
    }
    
    if((TMRS12_GetTime() - Seg7_Time) > 61) {
      
      Seg7_Time = TMRS12_GetTime();
      (void)dispNum(convNum(-1*(i>>1)),MORALE_DISP);
      
      i++;
    }
  }
  
  // Test alternating displays
  Seg7_Time = TMRS12_GetTime();
  while(i<32) {
  
    if((TMRS12_GetTime() - Seg7_Time) > 61) {
      
      Seg7_Time = TMRS12_GetTime();    
      (void)dispNum(convNum(i>>1),MORALE_DISP);
      
      i++;
    }
    
    if((TMRS12_GetTime() - Seg7_Time) > 61) {
      
      Seg7_Time = TMRS12_GetTime();
      (void)dispNum(convNum(-1*(i>>1)),TOWRP_DISP);
      
      i++;
    }
  }
  
  // Outer loop through each display
  for(j=0;j<3;j++) {
  
    display = j;
    i = 0;
    Seg7_Time = TMRS12_GetTime();
    
    // Send each number to the display
    while(i<33) {
    
      if((TMRS12_GetTime() - Seg7_Time) > 61) {
      
        Seg7_Time = TMRS12_GetTime();
      
        switch(i) {
          case 0:  (void)dispNum(POS_0, display);
                   break;
          case 1:  (void)dispNum(POS_1, display);
                   break;
          case 2:  (void)dispNum(POS_2, display);
                   break;
          case 3:  (void)dispNum(POS_3, display);
                   break;
          case 4:  (void)dispNum(POS_4, display);
                   break;
          case 5:  (void)dispNum(POS_5, display);
                   break;
          case 6:  (void)dispNum(POS_6, display);
                   break;
          case 7:  (void)dispNum(POS_7, display);
                   break;
          case 8:  (void)dispNum(POS_8, display);
                   break;
          case 9:  (void)dispNum(POS_9, display);
                   break;
          case 10: (void)dispNum(POS_A, display);
                   break;
          case 11: (void)dispNum(POS_B, display);
                   break;
          case 12: (void)dispNum(POS_C, display);
                   break;
          case 13: (void)dispNum(POS_D, display);
                   break;
          case 14: (void)dispNum(POS_E, display);
                   break;
          case 15: (void)dispNum(POS_F, display);
                   break;
          case 16: (void)dispNum(NEG_0, display);
                   break;
          case 17: (void)dispNum(NEG_1, display);
                   break;
          case 18: (void)dispNum(NEG_2, display);
                   break;
          case 19: (void)dispNum(NEG_3, display);
                   break;
          case 20: (void)dispNum(NEG_4, display);
                   break;
          case 21: (void)dispNum(NEG_5, display);
                   break;
          case 22: (void)dispNum(NEG_6, display);
                   break;
          case 23: (void)dispNum(NEG_7, display);
                   break;
          case 24: (void)dispNum(NEG_8, display);
                   break;
          case 25: (void)dispNum(NEG_9, display);
                   break;
          case 26: (void)dispNum(NEG_A, display);
                   break;
          case 27: (void)dispNum(NEG_B, display);
                   break;
          case 28: (void)dispNum(NEG_C, display);
                   break;
          case 29: (void)dispNum(NEG_D, display);
                   break;
          case 30: (void)dispNum(NEG_E, display);
                   break;
          case 31: (void)dispNum(NEG_F, display);
                   break;
          case 32: (void)dispNum(NTHNG, display);
                   break;
        }
              
        i++;
      }
    }
  }
}

#endif