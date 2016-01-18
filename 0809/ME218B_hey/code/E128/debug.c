/***********************************************************************

Module
    debug.c

Description
    Code for using SPI to output states on a single digit display
    
Notes
    
***********************************************************************/


/*------------------------- Include Files ----------------------------*/

#include <termio.h>
#include <stdio.h>
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"

/*------------------------- Module Variables -------------------------*/

static char State;
static char SPI_Transmit_Ready;

/*------------------------ Module Functions --------------------------*/

/*------------------------ Module Defines ----------------------------*/

// #define DEBUG_TEST

#define YES      1
#define NO       1

/*-------------------------- Module Code -----------------------------*/


/***********************************************************************
Function
    void InitSPI(void)
    
Description
    Intializes SPI on the E128 - makes it a master
    
***********************************************************************/

void InitSPI(void) {
   char dummy; 
   
   // set baud rate to /256
   SPIBR = (_S12_SPR2 | _S12_SPR1 | _S12_SPR0);  
   // set phase to mode 3
   SPICR1 |= (_S12_CPOL | _S12_CPHA);  			     
   // make master
   SPICR1 |= (_S12_MSTR);
   // enable SPI 			                   
   SPICR1 |= (_S12_SPE);                         	
   
   //Send Initial Byte
   dummy = SPISR;
   SPIDR = 100;
}

/***********************************************************************
Function
    void InputSPI(char debugvalue)
    
Description
    Pass a number value 0-9 to send via SPI, sets the transmit ready 
    flag
    
***********************************************************************/

void InputSPI (char debugvalue) {
  State = debugvalue;
  SPI_Transmit_Ready = YES;
}

/***********************************************************************
Function
    void TransmitSPI(void)
    
Description
    Sends the value stored over the SPI line to the C32
    Calls this function periodically to update what appears on the display.
    
***********************************************************************/
  
void TransmitSPI ( void){
   char dummy;
   
   if(SPI_Transmit_Ready==YES){
 //Clear the transmit flag, send data, and reset the transmission flag 
   dummy = SPISR;
   SPIDR = State;
   SPI_Transmit_Ready = NO; 
   }
}

/*-------------------------- Test Harness-----------------------------*/

#ifdef DEBUG_TEST

void main(void) {
  InitSPI();
  InputSPI(5);
  TransmitSPI();

 return;
}
#endif

/*-------------------------- End of file -----------------------------*/

