/***********************************************************************

Module
    tape.c

Description
    everything related to tape sensing is in this module
    
Notes
    Requires Board Connections as noted in the lab writeup

***********************************************************************/


/*------------------------- Include Files ----------------------------*/

#include <termio.h>
#include <stdio.h>
#include <hidef.h>          /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"
#include "ADS12.h"
#include "tape.h"
#include "timer.h"
			 

/*------------------------ Module Defines ----------------------------*/
//#define TAPE_TEST

//Front Left tape sensor thresholds
#define WHITE_THRESHOLD_FL 388
#define GREEN_THRESHOLD_FL 615
#define BLACK_THRESHOLD_FL 702


//Front Right tape sensor thresholds
#define WHITE_THRESHOLD_FR 602
#define GREEN_THRESHOLD_FR 750
#define BLACK_THRESHOLD_FR 791

//Rear tape sensor thresholds
#define WHITE_THRESHOLD_R 596
#define GREEN_THRESHOLD_R 715
#define BLACK_THRESHOLD_R 788


/*------------------------ Module Prototypes -------------------------*/



/*------------------------ Module Variables --------------------------*/



/*-------------------------- Module Code -----------------------------*/

/***********************************************************************
Function
    int GetTapeValue(char sensor)
    
Description
    Returns the value of the requested tape sensor(0-1023)
 
***********************************************************************/
int GetTapeValue(char sensor)
{		

	 return ADS12_ReadADPin(sensor);

}

/***********************************************************************
Function
    char GetTapeColor(char sensor)
    
Description
    Returns the color of the requested tape sensor
 
***********************************************************************/
char GetTapeColor(char sensor) {	
  int WhiteThreshold, GreenThreshold, BlackThreshold;
	int value = GetTapeValue(sensor);
	
	switch(sensor) {
	  case FRONT_RIGHT:
	    WhiteThreshold = WHITE_THRESHOLD_FR;
	    GreenThreshold = GREEN_THRESHOLD_FR;
	    BlackThreshold = BLACK_THRESHOLD_FR;
	    break;
	    
	  case FRONT_LEFT:
	    WhiteThreshold = WHITE_THRESHOLD_FL; 
	    GreenThreshold = GREEN_THRESHOLD_FL;
	    BlackThreshold = BLACK_THRESHOLD_FL;
	    break;
	    
	  case REAR:
	    WhiteThreshold = WHITE_THRESHOLD_R; 
	    GreenThreshold = GREEN_THRESHOLD_R;
	    BlackThreshold = BLACK_THRESHOLD_R;
	    break;
	}
	
	if(value < WhiteThreshold) {
		return WHITE;
	}
	if(value > WhiteThreshold && value < GreenThreshold) {
		return GREEN;
	}
	if(value > BlackThreshold) {
		return BLACK;
	}
	
	return GREEN_TAPE;
}

/*-------------------------- Test Harness ----------------------------*/
#ifdef TAPE_TEST

void main(void) {
  (void)puts("\r\nIn test harness for tape.c \r\n");
	InitPorts();
  
  while(1) 
  {
  	if(kbhit()) {
  	  (void)getchar(); 
  		(void)printf("Front Left;\tValue: %d Color: %d\r\n",GetTapeValue(FRONT_LEFT),GetTapeColor(FRONT_LEFT));
  		(void)printf("Front Right;\tValue: %d Color: %d\r\n",GetTapeValue(FRONT_RIGHT),GetTapeColor(FRONT_RIGHT));
  		(void)printf("Rear;\t\tValue: %d Color: %d\r\n\r\n",GetTapeValue(REAR),GetTapeColor(REAR));
  	}
  	  	
  }
  	
  return;
}
#endif


/*-------------------------- End of file -----------------------------*/