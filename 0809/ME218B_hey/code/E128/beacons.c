/***********************************************************************

Module
    beacons.c

Description
    Everything related to the beacons is in this module
    
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
#include "beacons.h"
#include "motors.h"
#include "timer.h"

/*------------------------ Module Variables --------------------------*/

static char BeaconDuty4; //Left Top
static char BeaconDuty5; //Right Top
static char BeaconDuty6; //Left Flag
static char BeaconDuty7; //Right Flag

//Test stuff
static char LeftCounter50 = 0;
static char LeftCounter0 = 0;
//end test stuff

static CornerBeacons_t corners;
static char GateBlocked = FALSE;

/*------------------------ Module Functions --------------------------*/

/*------------------------ Module Defines ----------------------------*/
//#define BEACON_TEST

#define BEACONPERIOD		 150

#define MOVE_STRAIGHT	       20	      // set experimentally
#define iGain			 0.002	// set experimentally

/*-------------------------- Module Code -----------------------------*/

/***********************************************************************
Function
    void InitBeacons(void)
    
Description
    Initializes the timers for each of the beacons to 1ms. 
    
***********************************************************************/

void InitBeacons(void) {
  StartTimer(TOP_LEFT_BEACON_TIMER, 1);
  StartTimer(TOP_RIGHT_BEACON_TIMER, 1);
  StartTimer(FLAG_LEFT_BEACON_TIMER, 1);
  StartTimer(FLAG_RIGHT_BEACON_TIMER, 1);
}

/***********************************************************************
Function
    void InitSide(void)
    
Description
    Initializes the beacons based on which side we are on. 
    If Pin 5 on the AD port is low, sets the side to 'R' and the     
    own/opposing duty variables to 30%/70%. 
    If Pin 6 on the AD port is low, sets the side to 'G' and the     
    own/opposing duty variables to 70%/30%. 
    If neither are low, then no side is selected.
    
***********************************************************************/

void InitSide(void) {
  if((PTIAD & BIT5HI) == 0) {
    //Red side
    (void)printf("Red side!\r\n");
    corners.side = 'R';
    corners.own = 30;
    corners.opposing = 70;
  } else if((PTIAD & BIT6HI) == 0) {
    //Green side
    (void)printf("Green side!\r\n");
    corners.side = 'G';
    corners.own = 70;
    corners.opposing = 30;
  } else {
    //No side
    (void)printf("No side selected!\r\n");
  }
}

/***********************************************************************
Function
    CornerBeacons_t GetSide(void)
    
Description
    Returns the CornerBeacons structure for the current side.
    
***********************************************************************/
CornerBeacons_t GetSide(void) {
  return corners;
}

/***********************************************************************
Function
    char DetectFlash(void)
    
Description
    Returns a TRUE when the flash is detected, and clears the flash-detected flag. 
    
***********************************************************************/

char DetectFlash (void) {
    if((TIM0_TFLG1 & _S12_C7F) != 0) {
        TIM0_TFLG1 = _S12_C7F;
        return TRUE;
    } else {
        return FALSE;
    }
}

/***********************************************************************
Function
    char BeaconDuty(char BeaconNumber)
    
Description
    Returns the the detected duty of the referenced beacon.
    
***********************************************************************/

char BeaconDuty (char BeaconNumber) {
   if (BeaconNumber == 4) 
   {
   	return BeaconDuty4;
   }
   else if (BeaconNumber == 5) 
   {
   	return BeaconDuty5;
   }
   else if (BeaconNumber == 6) 
   {
   	return BeaconDuty6;
   }
   else if (BeaconNumber == 7) 
   {
   	return BeaconDuty7;
   }
}

/***********************************************************************
Function
    short GetBeaconLevel(char BeaconPin)
    
Description
    Returns the intensity level of the referenced beacon(Value:0-1023).
    
***********************************************************************/
short GetBeaconLevel(char BeaconPin) {
  return ADS12_ReadADPin(BeaconPin);
}

/***********************************************************************
Function
    unsigned char Orientation(void)
    
Description
    Returns the absolute orientation of the robot on the field
    
***********************************************************************/

unsigned char Orientation(void) {
   if (BeaconDuty4 == 70 && BeaconDuty5 == 70) {
     (void)printf("\r\n Facing 70");
     return 1;
   }
   else if (BeaconDuty4 == 30 && BeaconDuty5 == 30) {
     (void)printf("\r\n Facing 30");
     return 2;
   }
   else if (BeaconDuty4 == 70 && BeaconDuty5 == 30) {
     (void)printf("\r\n 70 Left, 30 Right");
     return 3;
   } else if (BeaconDuty4 == 30 && BeaconDuty5 == 70) {
     (void)printf("\r\n 30 Left, 70 Right");
     return 4;
   }
   else {
     (void)printf("\r\n No Signal");
     return 5;
   }
}

/***********************************************************************
Function
    void BeaconControlMove(void)
    
Description
    Implements control law to head towards a beacon. The function     
    takes the difference between two beacons(either both flag- or   
    both orientation-dection beacons) and calculates the appropriate   
    motor commands to minimize the difference.
    
    
***********************************************************************/

void BeaconControlMove(void) {
    int threshold;
    int delta = 0;
    int average = 0;
    short LeftFlagLevel;
    short RightFlagLevel;
    /*float pGain = 0;
    int deltaSumError = 0;
    float deltaRequest = 0.0;
    float sumRequest = 0.0;
    float totalRequest = 0.0; 
   
    //When the beacon timers expire, reset the measured duty to 0.
    if(IsTimerExpired(TOP_LEFT_BEACON_TIMER)) {
      BeaconDuty4 = 0;
    }
    if(IsTimerExpired(TOP_RIGHT_BEACON_TIMER)) {
      BeaconDuty5 = 0;
    }
    if(IsTimerExpired(FLAG_LEFT_BEACON_TIMER)) {
      BeaconDuty6 = 0;
    }
    if(IsTimerExpired(FLAG_RIGHT_BEACON_TIMER)) {
      BeaconDuty7 = 0;
    }
    
    //When the control law timer expires, restart the timer and    
    //calculate the appropriate motor commands
    if (IsTimerExpired(BEACON_CONTROL_TIMER)) {
    	  
      StartTimer(BEACON_CONTROL_TIMER, 100);
	
	//Form the delta and the average of the Orientation Beacons
      delta = ADS12_ReadADPin(3) - ADS12_ReadADPin(4);
   	average = (int)((ADS12_ReadADPin(3) + ADS12_ReadADPin(4))/2);

   	//Calculate the appropriate motor correction commands based on 	//the PI control law
   	  if (average < 200) {
   	    pGain = 0.002;
   	  
      } else if(average < 400) {
   	    pGain = 0.002;
   	  } else if (average > 400) {
   	    pGain = 0.01;
   	  }
   	  
   	  deltaRequest = (delta*pGain); 
   	  
   	  if (deltaRequest > 40) {
         deltaRequest = 40;
      } else if (deltaRequest < -40) {
         deltaRequest = -40;
      }	
   	  
   	  deltaSumError += delta;
   		
   		sumRequest = (deltaSumError * iGain); 
   		if (sumRequest > 20) {
         sumRequest = 20;
   		   deltaSumError -= delta;      // anti-windup
      } else if (sumRequest < -20) {
         sumRequest = -20;
         deltaSumError -= delta;      // anti-windup
      }			 
   		
   		totalRequest = deltaRequest + sumRequest;
   		
   	if (deltaRequest > 0) {// left is higher, so it needs to turn 
				     //right more
   	    SimpleMove(MOVE_STRAIGHT,MOVE_STRAIGHT + (int)deltaRequest);
   	}
   	if (deltaRequest < 0) {// right is higher, so it needs to turn 				     //left more
   	    SimpleMove(MOVE_STRAIGHT + (-1)*(int)											deltaRequest,MOVE_STRAIGHT);   	}

    }*/
  
  //Form the delta and the average of the Flag Beacons
  LeftFlagLevel = GetBeaconLevel(LEFT_FLAG_LEVEL);
  RightFlagLevel = GetBeaconLevel(RIGHT_FLAG_LEVEL);
  if(LeftFlagLevel > 400 || RightFlagLevel > 400) {
    average = (int)((LeftFlagLevel + RightFlagLevel)/2);
    delta = LeftFlagLevel - RightFlagLevel; 

    /*if(average < 200) {//farther away
      threshold = 500;
  	} else  {
  	  threshold = 300;
  	}*/
  	threshold = 200;

    if((delta < threshold) && (delta > -1*threshold) ) {
    	  SimpleMove(30,30);		 // go straight
    	} else if (delta > threshold) {
    		SimpleMove(12, 25);		 // turn left
    	} else if(delta < -1*threshold) {
    		SimpleMove(25, 12);		 // turn right
    	}
    }
}

/***********************************************************************
Function
    void interrupt _Vec_tim1ch4 BeaconEdge4(void)  
    
Description
    Interrupt response routine for left top beacon - using input 
    capture
    
***********************************************************************/

void interrupt _Vec_tim1ch4 BeaconEdge4 (void)  {
  
   static int ThisRisingEdge  = 0;
   static int ThisFallingEdge = 0;
   static int HighTime;
   static char DutyCycle;   
   static char counter70 = 0;
   static char counter30 = 0;
   static char counter0 = 0;   
     
   if((PTT & BIT4HI) != 0) {                   // if this was a rising edge
      ThisRisingEdge = TIM1_TC4;  		 // store timer
   } else {								 // this was a falling edge
      ThisFallingEdge = TIM1_TC4;        // store timer 
   	  if(ThisFallingEdge > ThisRisingEdge) {    // in case of timer overflow
         HighTime = ThisFallingEdge - ThisRisingEdge;
   	  }
   }
   
   DutyCycle = (char)((HighTime*100) / BEACONPERIOD);
   
   //Increment the appropriate counter depending on the recorded duty cycle
   if((DutyCycle > 20) && (DutyCycle < 40)){
      counter30++;
   } else if ((DutyCycle > 60) && (DutyCycle < 80)){
      counter70++;
   } else if(DutyCycle <= 20 || (DutyCycle >= 40 && DutyCycle <= 60)) {
      counter0++;
   }
   
   //Whichever counter gets to 40 first becomes the duty cycle
   if(counter30 == 40) {
     BeaconDuty4 = 30;
     counter70 = 0;
     counter30 = 0;
     counter0 = 0;
   } else if(counter70 == 40) {
     BeaconDuty4 = 70;
     counter70 = 0;
     counter30 = 0;
     counter0 = 0;
   } else if(counter0 == 40) {
     BeaconDuty4 = 0;
     counter70 = 0;
     counter30 = 0;
     counter0 = 0;
   }
   
   //Reset no-edge timer
   StartTimer(TOP_LEFT_BEACON_TIMER, 50);

   TIM1_TFLG1 = _S12_C4F;				 // Clear the Interrupt Flag	 

}

/***********************************************************************
Function
    void interrupt _Vec_tim1ch5 BeaconEdge5(void)
    
Description
    Interrupt response routine for right top beacon - using input 
    capture
    
***********************************************************************/

void interrupt _Vec_tim1ch5 BeaconEdge5 (void)  {
  
   static int ThisRisingEdge  = 0;
   static int ThisFallingEdge = 0;
   static int HighTime;
   static char DutyCycle;   
   static char counter70 = 0;
   static char counter30 = 0;
   static char counter0 = 0;   
     
   if((PTT & BIT5HI) != 0) {                   // if this was a rising edge
      ThisRisingEdge = TIM1_TC5;  		 // store timer
   } else {								 // this was a falling edge
      ThisFallingEdge = TIM1_TC5;        // store timer 
   	  if(ThisFallingEdge > ThisRisingEdge) {    // in case of timer overflow
         HighTime = ThisFallingEdge - ThisRisingEdge;
   	  }
   }
   
   DutyCycle = (char)((HighTime*100) / BEACONPERIOD);

   //Increment the appropriate counter depending on the recorded duty cycle
   if((DutyCycle > 20) && (DutyCycle < 40)){
      counter30++;
   } else if ((DutyCycle > 60) && (DutyCycle < 80)){
      counter70++;
   } else if(DutyCycle <= 20 || (DutyCycle >= 40 && DutyCycle <= 60)) {
      counter0++;
   }
   
   //Whichever counter gets to 40 first becomes the duty cycle
   if(counter30 == 40) {
     BeaconDuty5 = 30;
     counter70 = 0;
     counter30 = 0;
     counter0 = 0;
   } else if(counter70 == 40) {
     BeaconDuty5 = 70;
     counter70 = 0;
     counter30 = 0;
     counter0 = 0;
   } else if(counter0 == 40) {
     BeaconDuty5 = 0;
     counter70 = 0;
     counter30 = 0;
     counter0 = 0;
   }

   //Reset no-edge timer
   StartTimer(TOP_RIGHT_BEACON_TIMER, 50);

   TIM1_TFLG1 = _S12_C5F;				 // Clear the Interrupt Flag	 

}



/***********************************************************************
Function
    void interrupt _Vec_tim1ch6 BeaconEdge6(void)
    
Description
    Interrupt response routine for left flag beacon - using input 
    capture
    
***********************************************************************/

void interrupt _Vec_tim1ch6 BeaconEdge6 (void)  {
  
   static int ThisRisingEdge  = 0;
   static int ThisFallingEdge = 0;
   static int HighTime;
   static char DutyCycle;
   static char counter50 = 0;
   static char counter0 = 0;   
     
   if((PTT & BIT6HI) != 0) {                   // if this was a rising edge
      ThisRisingEdge = TIM1_TC6;  		 // store timer
   } else {								 // this was a falling edge
      ThisFallingEdge = TIM1_TC6;        // store timer 
   	  if(ThisFallingEdge > ThisRisingEdge) {    // in case of timer overflow
         HighTime = ThisFallingEdge - ThisRisingEdge;
   	  }
   }
   
   DutyCycle = (char)((HighTime*100) / BEACONPERIOD);
   
   //Increment the appropriate counter depending on the recorded duty cycle
   if((DutyCycle >= 40) && (DutyCycle <= 60)){
      counter50++;
   } else {
      counter0++;
   }
   
   //Whichever counter gets to 40 first becomes the duty cycle
   if(counter50 == 40) {
     BeaconDuty6 = 50;
     counter50 = 0;
     counter0 = 0;
   } else if(counter0 == 40) {
     BeaconDuty6 = 0;
     counter50 = 0;
     counter0 = 0;
   }
   
   LeftCounter50 = counter50;
   LeftCounter0 = counter0;
   
   //Reset no-edge timer
   StartTimer(FLAG_LEFT_BEACON_TIMER, 50);
   
   TIM1_TFLG1 = _S12_C6F;				 // Clear the Interrupt Flag	 

}

/***********************************************************************
Function
    void interrupt _Vec_tim1ch7 BeaconEdge7 (void)
    
Description
    Interrupt response routine for right flag beacon - using input 
    capture
    
***********************************************************************/

void interrupt _Vec_tim1ch7 BeaconEdge7 (void)  {
  
   static int ThisRisingEdge  = 0;
   static int ThisFallingEdge = 0;
   static int HighTime;
   static char DutyCycle;  
   static char counter50 = 0;
   static char counter0 = 0; 
     
   if((PTT & BIT7HI) != 0) {                   // if this was a rising edge
      ThisRisingEdge = TIM1_TC7;  		 // store timer
   } else {								 // this was a falling edge
      ThisFallingEdge = TIM1_TC7;        // store timer 
   	  if(ThisFallingEdge > ThisRisingEdge) {    // in case of timer overflow
         HighTime = ThisFallingEdge - ThisRisingEdge;
   	  }
   }
   
   DutyCycle = (char)((HighTime*100) / BEACONPERIOD);
   
   //Increment the appropriate counter depending on the recorded duty cycle
   if((DutyCycle >= 40) && (DutyCycle <= 60)){
      counter50++;
   } else {
      counter0++;
   }
   
   //Whichever counter gets to 40 first becomes the duty cycle
   if(counter50 == 40) {
     BeaconDuty7 = 50;
     counter50 = 0;
     counter0 = 0;
   } else if(counter0 == 40) {
     BeaconDuty7 = 0;
     counter50 = 0;
     counter0 = 0;
   }

   //Reset no-edge timer
   StartTimer(FLAG_RIGHT_BEACON_TIMER, 50);

   TIM1_TFLG1 = _S12_C7F;				 // Clear the Interrupt Flag	 

}

void interrupt _Vec_tim0ch4 GateBeam (void) {
  if((PTT & BIT0HI) != 0) {
    GateBlocked = FALSE;
  } else {
    GateBlocked = TRUE;
  }
  
  TIM0_TFLG1 = _S12_C4F;			// clear interrupt flag!
}

char IsGateBlocked(void) {
  return GateBlocked;
}

/*-------------------------- Test Harness-----------------------------*/

#ifdef BEACON_TEST

void main(void) {

  
  (void)puts("\r\nIn test harness for beacons.c \r\n");
  InitTimer();
  InitMotors();
  InitPorts();
  InitBeacons();
  InitSide();
  EnableInterrupts;


   while(1) {
    if(IsTimerExpired(TOP_LEFT_BEACON_TIMER)) {
      BeaconDuty4 = 0;
    }
    if(IsTimerExpired(TOP_RIGHT_BEACON_TIMER)) {
      BeaconDuty5 = 0;
    }
    if(IsTimerExpired(FLAG_LEFT_BEACON_TIMER)) {
      BeaconDuty6 = 0;
    }
    if(IsTimerExpired(FLAG_RIGHT_BEACON_TIMER)) {
      BeaconDuty7 = 0;
    }
     (void)printf("TOP; Left: %d\tRight: %d\r\n", BeaconDuty(LEFT_TOP_BEACON), BeaconDuty(RIGHT_TOP_BEACON));
     //(void)printf("FLAG; Left: %d\t50count: %d\t0count: %d\tRight: %d\r\n", BeaconDuty(LEFT_FLAG_BEACON), LeftCounter50, LeftCounter0, BeaconDuty(RIGHT_FLAG_BEACON));
   }				

  
  // Beacon 6,3 is on left, Beacon 7,4 is on right
	 
  /*while (1) {
  	(void)printf("\r\n Left: %d\tRight: %d", ADS12_ReadADPin(3),ADS12_ReadADPin(4));
  	if ((ADS12_ReadADPin(3) > 400) || (ADS12_ReadADPin(4) > 400)) {
  	  BeaconControlMove();
  	} else {
  	  //SimpleMove(20,0);
  	}
  }*/
	

//  (void)Orientation();
     /*
		if ((ADS12_ReadADPin(3) > 100) || (ADS12_ReadADPin(4) > 100)) {
		delta = ADS12_ReadADPin(3) - ADS12_ReadADPin(4); 
		  
    if ((delta < 100) && (delta > -100) )
    {
  	  (void)printf("\r\n Moving Straight");
  	  SimpleMove(21,21);		 // go straight
  	}
  	else if ((delta > 100)) 
  	{
  		(void)printf("\r\n Turning Right");
  		SimpleMove(20,17);		 // turn right
  	}
  	else if ((delta < -100))
  	{
  		(void)printf("\r\n Turning Left");
  		SimpleMove(17,20);		 // turn left
  	}
  }
  	else // (BeaconDuty(RIGHT) == 0 && BeaconDuty(LEFT) == 0)
  	{
  		(void)printf("\r\n Looking for Beacons");
  		SimpleMove(0,15);		 // turn left
  	}	 
  		
    
 }
 
 */ 
  return;
}
#endif

/*-------------------------- End of file -----------------------------*/
