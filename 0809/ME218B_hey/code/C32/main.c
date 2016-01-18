/***********************************************************************

Module
    main.c

Description
    C32 code for the ME218B Project - slave module
    
Notes
    Requires Board Connections as noted in the lab writeup

***********************************************************************/


/*------------------------- Include Files ----------------------------*/

#include <termio.h>
#include <stdio.h>
#include <ME218_C32.h>
#include <S12Vec.h>
#include "ADS12.h"

/*------------------------- Module Defines ---------------------------*/

#define YES 1
#define NO 0

#define MS *188
#define UPDATE_PERIOD (340 MS)

#define LOW_VOLTAGE 130

/*------------------------ Module Functions --------------------------*/

void InitPorts ( void);
void InitSPI 	 ( void);
void InitTimer 	 ( void);
void DisplayState 	 ( char);
void ClockLEDState 	 ( char[]);
unsigned int CheckBattery(void);

/*------------------------ Module Variables --------------------------*/

static unsigned char data = 0;
static char SPI_Data_Received;
unsigned int pause;
unsigned int battery_indicator = 0;
/*-------------------------- Module Code -----------------------------*/

/***********************************************************************
Function
    void main(void)
    
Description
    Intializes Ports, SPI, AD.
	Runs in infinite loop to detect SPI communications and check battery level.
    
***********************************************************************/
void main(void) {
  EnableInterrupts;
  InitPorts();
  InitSPI(); 
  SPI_Data_Received = NO;
  ADS12_Init("AAAAAAAA"); 
  printf("program started on C32");
 
  data = 0;
  
  while(1)
  {
    battery_indicator = CheckBattery();
  	if(battery_indicator<LOW_VOLTAGE){
  	    printf("Low Battery Level: %d\r\n", battery_indicator);
  	    data = 9;
  	    DisplayState(data);
  	}else{ 

      	if(SPI_Data_Received==YES)
      	{
      		TIE = TIE&!_S12_C3I;
      		printf("Current State:%d",data);
      		DisplayState(data);
        	SPI_Data_Received = NO;
        	
        	TIE = TIE|_S12_C3I;
      	  }
  	}

  }
}

/***********************************************************************
Function
    void InitPorts(void)
    
Description
    Intializes Ports on the C32
	Enable the shift register outputs
        T0 = Shift Clock
        T1 = Storage Clock
        T2 = Serial Input
    
***********************************************************************/
void InitPorts (void) { 
   
   //Enable Shift Register outputs
   DDRT |= (BIT0HI|BIT1HI|BIT2HI);
   PTT &= (BIT0LO|BIT1LO|BIT2LO);
}

/***********************************************************************
Function
    void InitSPI(void)
    
Description
    Intializes SPI on C32.
    
***********************************************************************/
void InitSPI (void)  {  
   SPIBR = (_S12_SPR2 | _S12_SPR1 | _S12_SPR0);	  // set baud rate to /256
   SPICR1 |= (_S12_CPOL | _S12_CPHA);  			  // set phase to mode 3
   SPICR1 &= ~(_S12_MSTR);			 // make slave
   SPICR1 |= (_S12_SPIE);		 // enable interrupts
   SPICR1 |= _S12_SPE;                     // enable SPI 
}

/***********************************************************************
Function
    int Checkbattery(void)
    
Description
    Reads the voltage across the resistor network on the AD pin.
    
***********************************************************************/
unsigned int CheckBattery(void){
  return ADS12_ReadADPin(0);
}

/***********************************************************************
Function
    void ClockLEDState(char [])
    
Description
    Clocks data in the LEDs array through the shift register.
    Enables digits to display on the 7-segment.
    
***********************************************************************/

void ClockLEDState(char LEDs[])
{
	char count;
	PTT|= BIT1HI;
	for(count = 7;count>=0;count--)
	{

	  //Put data on pin 2
	  if(LEDs[count]==1)
	  {
	  	PTT|=BIT2HI;
	  }
	  else
	  {
	  	PTT&=BIT2LO;
	  }
	  
	  //Clock data
	  PTT|=BIT0HI;
	  
	  for(pause = 0;pause<555;pause++)
	  {
	  }
	  PTT&=BIT0LO;
	  for(pause = 0;pause<555;pause++)
	  {
	  }
	}
	
	//Enable data on the storage register
	PTT&= BIT1LO;
	for(pause = 0;pause<555;pause++)
	{
	}	

}

/***********************************************************************
Function
    void DisplayState(char)
    
Description
    Maps desired segment digit to the data to shift in the register and
	populates LED array.
	Calls ClockLEDState to display data.
    
***********************************************************************/
void DisplayState(char state)  { 
	char STLEDs[] = {0,0,0,0,0,0,0,0};
	//Segment<->byte mapping
	char A = 0;
	char B = 1;
	char C = 2;
	char D = 3;
	char E = 4;
	char F = 5;
	char G = 6;
	char dot = 7;
	
	//Shift in 'dot' bit if desired digit is greater than 9
	if(state>9)
	{
		state = state - 10;
		STLEDs[dot] = 1;
	}
  //Store shift bits for displaying the appropriate digits
  switch(state)
   {
   	case 0:
	   	STLEDs[A] = 1;
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	STLEDs[D] = 1;
	   	STLEDs[E] = 1;
	   	STLEDs[F] = 1;
	   	break;
	  case 1:
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	break;
	   case 2:
	   	STLEDs[A] = 1;
	   	STLEDs[B] = 1;
	   	STLEDs[D] = 1;
	   	STLEDs[E] = 1;
	   	STLEDs[G] = 1;
	   	break;
	   case 3:
	   	STLEDs[A] = 1;
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	STLEDs[D] = 1;
	   	STLEDs[G] = 1;
	   	break;
	   case 4:
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	STLEDs[F] = 1;
	   	STLEDs[G] = 1;
	   	break;
	   case 5:
	   	STLEDs[A] = 1;
	   	STLEDs[C] = 1;
	   	STLEDs[D] = 1;
	   	STLEDs[F] = 1;
	   	STLEDs[G] = 1;
	   	break;
	   case 6:
	   	STLEDs[C] = 1;
	   	STLEDs[D] = 1;
	   	STLEDs[E] = 1;
	   	STLEDs[F] = 1;
	   	STLEDs[G] = 1;
	   	break;
	   case 7:
	   	STLEDs[A] = 1;
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	break;
	   case 8:
	   	STLEDs[A] = 1;
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	STLEDs[D] = 1;
	   	STLEDs[E] = 1;
	   	STLEDs[F] = 1;
	   	STLEDs[G] = 1;
	   	break;
	   case 9:
	   	STLEDs[A] = 1;
	   	STLEDs[B] = 1;
	   	STLEDs[C] = 1;
	   	STLEDs[F] = 1;
	   	STLEDs[G] = 1;
	   	break;
   }
   
   //Output data to shift register
   ClockLEDState(STLEDs);
}

 /***********************************************************************
Function
    void interrupt _Vec_SPI HandleSPI(void)
    
Description
    SPI interrupt response routine 
    
***********************************************************************/
void interrupt _Vec_SPI HandleSPI ( void) {
   static char dummy;	 
   //Check SPISR to see if data was received then write to SPIDR to clear
   dummy = SPISR;
   data = SPIDR;
   //Set the data received flag to YES
   SPI_Data_Received = YES;
}		

/*-------------------------- End of file -----------------------------*/