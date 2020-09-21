/*
 * PWM Module for the E128 Microcontroller
 *
 * This PWM Module allows PWM signals to be sent from pins U0-U5
 * at frequencies ranging from approximately 150 Hz to 20000 Hz.
 * These frequencies come in quantized ranges; at any given time,
 * only two of the ranges are achievable, one through channels 0, 1,
 * 4, and 5 and the other through channels 2 and 3.
 *
 * @author David Hoffert
 */

// Generic E128 Includes
#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"

// Module-Specific Includes
#include "pwm_e128.h"

/*
 * Initializes the overall PWM subsystem.  Must be run before any
 * individual channel is initialized.
 *
 * @param chan0145range The frequency range available to channels 0,1,4,5
 *                      Must use a predefined module constant for this
 * @param chan23range   The frequency range available to channels 2,3
 *                      Must use a predefined module constant for this
 *
 * @author David Hoffert
 */
void pwm_sys_init(unsigned char chan0145range, unsigned char chan23range) {
  
  // Set all channels to have the output polarity that makes sense
  PWMPOL = 0x3F;
  
  // Set all channels to use the scaled versions of the clocks
  PWMCLK = 0x3F;
  
  // Set all channels to be in center-align mode
  PWMCAE = 0x3F;
  
  // Set the clock pre-scale and scale for group A
  switch(chan0145range) {
      case FROM_150_TO_300:		 PWMPRCLK = 0x02;
                               PWMSCLA = 0x25;
                               break;      
      case FROM_400_TO_900:		 PWMPRCLK = 0x01;
                               PWMSCLA = 0x21;
                               break;
      case FROM_1000_TO_2500:	 PWMPRCLK = 0x03;
                               PWMSCLA = 0x03;
                               break;
      case FROM_3000_TO_7500:	 PWMPRCLK = 0x03;
                               PWMSCLA = 0x01;
                               break;
      case FROM_8000_TO_20000: PWMPRCLK = 0x00;
                               PWMSCLA = 0x03;
                               break;
      default: 								 PWMPRCLK = 0x00;
                               PWMSCLA = 0x03;
                               break;
  }
  
  // Set the clock pre-scale and scale for group B
  switch(chan23range) {
      case FROM_150_TO_300:		 PWMPRCLK |= 0x20;
                               PWMSCLB = 0x25;
                               break;      
      case FROM_400_TO_900:		 PWMPRCLK |= 0x10;
                               PWMSCLB = 0x21;
                               break;
      case FROM_1000_TO_2500:	 PWMPRCLK |= 0x30;
                               PWMSCLB = 0x03;
                               break;
      case FROM_3000_TO_7500:	 PWMPRCLK |= 0x30;
                               PWMSCLB = 0x01;
                               break;
      case FROM_8000_TO_20000: PWMPRCLK |= 0x00;
                               PWMSCLB = 0x03;
                               break;
      default: 								 PWMPRCLK |= 0x00;
                               PWMSCLB = 0x03;
                               break;
  }
}

/*
 * Initializes an individual PWM channel.  Must be run after the
 * system initialization is performed.  Default duty cycle is 0.
 *
 * @param chan  Which channel to initialize, 0-5 (others ignored)
 * @param freq  The initial frequency this channel should run at
 *              Must use a predefined module constant for this
 *
 * @author David Hoffert
 */
void pwm_chan_init(unsigned char chan, unsigned char freq) {
  
  // For the given channel, set the frequency, duty cycle, and enables
  switch(chan) {
    case 0:  PWMPER0 = freq;
             PWMDTY0 = 0;
             MODRR |= BIT0HI;
             PWME |= BIT0HI;
             break;
    case 1:  PWMPER1 = freq;
             PWMDTY1 = 0;
             MODRR |= BIT1HI;
             PWME |= BIT1HI;
             break;
    case 2:  PWMPER2 = freq;
             PWMDTY2 = 0;
             MODRR |= BIT2HI;
             PWME |= BIT2HI;
             break;
    case 3:  PWMPER3 = freq;
             PWMDTY3 = 0;
             MODRR |= BIT3HI;
             PWME |= BIT3HI;
             break;
    case 4:  PWMPER4 = freq;
             PWMDTY4 = 0;
             MODRR |= BIT4HI;
             PWME |= BIT4HI;
             break;
    case 5:  PWMPER5 = freq;
             PWMDTY5 = 0;
             MODRR |= BIT5HI;
             PWME |= BIT5HI;
             break;
    default: break;
  } 
}

/*
 * Disables an individual PWM channel.
 *
 * @param chan Which channel to disable, 0-5 (others ignored)
 *
 * @author David Hoffert
 */
void pwm_chan_disable(unsigned char chan) {
  
  // For the given channel, clear both enable bits
  switch(chan) {
    case 0:  MODRR &= BIT0LO;
             PWME &= BIT0LO;
             break;
    case 1:  MODRR &= BIT1LO;
             PWME &= BIT1LO;
             break;
    case 2:  MODRR &= BIT2LO;
             PWME &= BIT2LO;
             break;
    case 3:  MODRR &= BIT3LO;
             PWME &= BIT3LO;
             break;
    case 4:  MODRR &= BIT4LO;
             PWME &= BIT4LO;
             break;
    case 5:  MODRR &= BIT5LO;
             PWME &= BIT5LO;
             break;
    default: break;
  }
}

/*
 * Changes the duty cycle of an individual PWM channel.
 *
 * @param chan Which channel to modify, 0-5 (others ignored)
 * @param dc The new duty cycle, a number between 0 and 100
 *
 * @author David Hoffert
 */
void pwm_change_dc(unsigned char chan, unsigned char dc) {
  
  // Set aside a larger variable for the duty cycle conversion
  unsigned int temp = 0;
  
  // Truncate the duty cycle at 100, if necessary
  if(dc > 100) dc = 100;
  
  // For the given channel, update the duty cycle
  switch(chan) {
    case 0:  temp = PWMPER0*((unsigned int)dc);
             PWMDTY0 = (unsigned char)(temp/100);
             break;
    case 1:  temp = PWMPER1*((unsigned int)dc);
             PWMDTY1 = (unsigned char)(temp/100);
             break;
    case 2:  temp = PWMPER2*((unsigned int)dc);
             PWMDTY2 = (unsigned char)(temp/100);
             break;
    case 3:  temp = PWMPER3*((unsigned int)dc);
             PWMDTY3 = (unsigned char)(temp/100);
             break;
    case 4:  temp = PWMPER4*((unsigned int)dc);
             PWMDTY4 = (unsigned char)(temp/100);
             break;
    case 5:  temp = PWMPER5*((unsigned int)dc);
             PWMDTY5 = (unsigned char)(temp/100);
             break;
    default: break;
  }
}

/*
 * Changes the frequency of an individual PWM channel.
 *
 * @param chan Which channel to modify, 0-3 (others ignored)
 * @param freq The new frequency this channel should run at
 *             Must use a predefined module constant for this
 *
 * @author David Hoffert
 */
void pwm_change_freq(unsigned char chan, unsigned char freq) {
  
  // Set aside a larger variable for the duty cycle conversion
  unsigned int temp = 0;
  
  // For the given channel, update the frequency and duty cycle
  switch(chan) {
    case 0:  temp = ((unsigned int)PWMDTY0)*100;
             temp /= PWMPER0; // We have now extracted the percentage
             temp *= freq;
             PWMPER0 = freq;
             PWMDTY0 = (unsigned char)(temp/100);
             break;
    case 1:  temp = ((unsigned int)PWMDTY1)*100;
             temp /= PWMPER1; // We have now extracted the percentage
             temp *= freq;
             PWMPER1 = freq;
             PWMDTY1 = (unsigned char)(temp/100);
             break;
    case 2:  temp = ((unsigned int)PWMDTY2)*100;
             temp /= PWMPER2; // We have now extracted the percentage
             temp *= freq;
             PWMPER2 = freq;
             PWMDTY2 = (unsigned char)(temp/100);
             break;
    case 3:  temp = ((unsigned int)PWMDTY3)*100;
             temp /= PWMPER3; // We have now extracted the percentage
             temp *= freq;
             PWMPER3 = freq;
             PWMDTY3 = (unsigned char)(temp/100);
             break;
    case 4:  temp = ((unsigned int)PWMDTY4)*100;
             temp /= PWMPER4; // We have now extracted the percentage
             temp *= freq;
             PWMPER4 = freq;
             PWMDTY4 = (unsigned char)(temp/100);
             break;
    case 5:  temp = ((unsigned int)PWMDTY5)*100;
             temp /= PWMPER5; // We have now extracted the percentage
             temp *= freq;
             PWMPER5 = freq;
             PWMDTY5 = (unsigned char)(temp/100);
             break;
    default: break;
  }
}