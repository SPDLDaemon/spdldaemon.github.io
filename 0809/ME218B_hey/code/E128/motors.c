/***********************************************************************

Module
    motors.c

Description
    Everything related to the drive motors is in this module.
    
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
#include "motors.h"
#include "timer.h"

/*------------------------ Module Defines ----------------------------*/

//#define MOTORS_TEST
 
#define HUNDREDMS         1875

#define MAX_ACCEL 100
#define HISTORY_SIZE 5

// for calculating motor RPM
#define FREQUENCY      187500	 //clock speed divided by timer prescale
#define S_PER_MIN	     60      // seconds per minute
#define TICKS_PER_REV  12	 // ticks per encoder revolution

// Scale factors 
#define PWMSCALE       3	 // see calcs in lab
#define PWMPERIOD      100     // see calcs in lab
#define POTSCALEDUTY   4       // scale factor for pot
#define POTSCALESPEED  22      // scale factor for pot

// for PI control
#define YES            1
#define NO             0
//#define pGain	     0.3	 // set experimentally
#define iGain	     0.004	 // set experimentally

typedef union {
  struct {
    unsigned int overflows;
    unsigned int thetime;
  } ByInts;
  unsigned long AsLong;
} LongByInts;

/*------------------------ Module Variables --------------------------*/

static unsigned char LeftRPM;
static unsigned char RightRPM;
static char LeftRPMTarget = 0;
static char RightRPMTarget = 0;
static unsigned char PWMOffset = 0;

//ForTesting
static float LeftDutyRequest;
static float pGain = 0.6;
static float LeftSumError = 0.0;
static float RPMError;
//ENDfortesting

static unsigned int OverFlows;

/*------------------------ Private Function Prototypes ---------------*/

static unsigned char EncoderInterrupt(LongByInts *LastTime, LongByInts ThisTime);

/*-------------------------- Module Code -----------------------------*/

/***********************************************************************
Function
    void InitMotors(void)
    
Description
    Initializes PWM signals for the Motors
    
************************************************   ***********************/

void InitMotors (void)  {
   // enable PWM0,PWM1,PWM2,PWM3
   PWME = (_S12_PWME0 | _S12_PWME1 | _S12_PWME2 | _S12_PWME3);	 
   // set clock A and B to /4
   PWMPRCLK = (_S12_PCKA1 | _S12_PCKB1);                         
   // set PWM Output polarity as initially high
   PWMPOL = (_S12_PPOL0 | _S12_PPOL1 | _S12_PPOL2 | _S12_PPOL3); 
   // set clock select to scaled clock A and B
   PWMCLK = (_S12_PCLK0 | _S12_PCLK1 | _S12_PCLK2 | _S12_PCLK3); 
   // set scale for A
   PWMSCLA = PWMSCALE;                                           
   // set scale for B
   PWMSCLB = PWMSCALE; 
   // map PWM output to U0, U1, U2, U3                                          
   DDRU |= (BIT0HI | BIT1HI | BIT2HI | BIT3HI);				

   //Sets the PWN period			   
   PWMPER0 = PWMPERIOD;
   PWMPER1 = PWMPERIOD; 
   PWMPER2 = PWMPERIOD;
   PWMPER3 = PWMPERIOD;	
   
   //Initialize duty to 0   
   PWMDTY0 = 0; 
   PWMDTY1 = 0;
   PWMDTY2 = 0;
   PWMDTY3 = 0;

}

/***********************************************************************
Function
    void SetPWMOffset(unsigned char Offset)
    
Description
    Sets an offset to increase/decrease the magnitude of SimpleMove 
    commands.
    
***********************************************************************/
void SetPWMOffset(unsigned char Offset) {
  PWMOffset = Offset;
}

/***********************************************************************
Function
    void SimpleMove(float LeftDuty, float RightDuty)
    
Description
    Implements Drive-Brake via and H-bridge. Sends PWM signal to the 
    robot left and right wheel. 
    -100 (full backwards) to 0 (stopped) to 100 (full forwards)
    
***********************************************************************/
void SimpleMove (float LeftDuty, float RightDuty){  
  
  if(LeftDuty == 0) {
    //Left stopped
    PWMDTY1 = 0;
    PWMDTY0 = 0;
  } else if(LeftDuty < 0) {
    LeftDuty -= PWMOffset;
    //Left going backwards.
    PWMDTY1 = (char)(-1 * (int)LeftDuty * PWMPERIOD / 100); //Scale to PWMPERIOD
    PWMDTY0 = 0;
  } else {
    LeftDuty += PWMOffset;
    //Left going forwards.
    PWMDTY1 = 0;
    PWMDTY0 = (char)((int)LeftDuty * PWMPERIOD / 100);
  }
  
  if(RightDuty == 0) {
    //Right stopped
    PWMDTY2 = 0;
    PWMDTY3 = 0;
  } else if(RightDuty < 0) {
    RightDuty -= PWMOffset;
    //Right going backwards.
    PWMDTY2 = (char)(-1 * (int)RightDuty * PWMPERIOD / 100);
    PWMDTY3 = 0;
  } else {
    RightDuty += PWMOffset;
    //Right going forwards.
    PWMDTY2 = 0;
    PWMDTY3 = (char)((int)RightDuty * PWMPERIOD / 100);
  }
}

/***********************************************************************
Function
    void ControlLawMove(char RequestedLeftRPM, char RequestedRightRPM)
    
Description
    Sets the module level variables with the requested RPMs for use by 
    the interrupt Encoder.
    
***********************************************************************/

void ControlLawMove (char RequestedLeftRPM, char RequestedRightRPM)  {
   LeftRPMTarget = RequestedLeftRPM;
   RightRPMTarget = RequestedRightRPM;
}

/***********************************************************************
Function
    void interrupt _Vec_tim0ovf Tim0OverflowInterrupt(void)
    
Description
    Increments the overflow counter for timer 0, for absolute timing purposes
    
***********************************************************************/

void interrupt _Vec_tim0ovf Tim0OverflowInterrupt(void) {
  OverFlows++;		
  TIM0_TFLG2 = _S12_TOF;
}


/***********************************************************************
Function
    void interrupt _Vec_tim0ch4 LeftEncoder(void)
    
Description
    Interrupt response routine for the Left Encoder - using input 
    capture
    
***********************************************************************/

/*void interrupt _Vec_tim0ch4 LeftEncoder (void)  {		
  static unsigned char RPMHistory[HISTORY_SIZE] = {0, 0, 0, 0, 0};
  static unsigned char OldestRPMIndex = 0;
  static unsigned char LastRPM = 0;
  static LongByInts ThisTime = { 0,0 };
  static LongByInts LastTime = { 0,0 };
					 
  unsigned char CurrRPM;
  int DeltaRPM;
  int RPMSum = 0;
  int i;
  
  ThisTime.ByInts.thetime = TIM0_TC4;

  CurrRPM = EncoderInterrupt(&LastTime, ThisTime);
   				
  DeltaRPM = CurrRPM - LastRPM;
  if(DeltaRPM < MAX_ACCEL && DeltaRPM > -1*MAX_ACCEL) {
    RPMHistory[OldestRPMIndex] = CurrRPM;
    OldestRPMIndex++;
    if(OldestRPMIndex == HISTORY_SIZE) {
      OldestRPMIndex = 0;
    }
    
    for(i = 0; i < HISTORY_SIZE; i++) {
      RPMSum += RPMHistory[i];
    }
    LeftRPM = RPMSum / HISTORY_SIZE;
  }
  
  StartTimer(LEFT_ENCODER_TIMER,500);
  
  TIM0_TFLG1 = _S12_C4F;			// clear interrupt flag!	 	
}*/

/***********************************************************************
Function
    void interrupt _Vec_tim0ch5 RightEncoder(void)
    
Description
    Interrupt response routine for the Right Encoder - using input 
    capture
    
***********************************************************************/

/*void interrupt _Vec_tim0ch5 RightEncoder (void)  {
  static unsigned char LastRPM = 0;
  static LongByInts ThisTime = { 0,0 };
  static LongByInts LastTime = { 0,0 };
  static char RejectionCounter = 0;
  
  unsigned char CurrRPM;
  int DeltaRPM;
  

  ThisTime.ByInts.thetime = TIM0_TC5;
 
  CurrRPM = EncoderInterrupt(&LastTime, ThisTime);
  
  DeltaRPM = CurrRPM - LastRPM;

  if((DeltaRPM < MAX_ACCEL && DeltaRPM > -1*MAX_ACCEL) || RejectionCounter == 10) {
    RejectionCounter = 0;
    RightRPM = CurrRPM;
    LastRPM = CurrRPM;
  } else {
    RejectionCounter++;
  }
 
  
  
  
  StartTimer(RIGHT_ENCODER_TIMER,500);
  TIM0_TFLG1 = _S12_C5F;			// clear interrupt flag!	 
	
}*/

/***********************************************************************
Function
    static unsigned char EncoderInterrupt(LongByInts *LastTime, 
                                          LongByInts ThisTime)
    
Description
    Function to run in Interrupt response routine for either encoder. Takes pointers so it
    can modify the correct variables.
    
***********************************************************************/
static unsigned char EncoderInterrupt(LongByInts *LastTime, LongByInts ThisTime) {
   unsigned long Period;
   unsigned int Encoder_Ticks_Per_M;

// if TOF is pending, and captured time after rollover
// handle TOF: increment overflow counter, clear flag
  /* if((((TIM0_TFLG2 & _S12_TOF) == _S12_TOF) && (TIM0_TC5 < 0x8000))) {
       OverFlows++;
       TIM0_TFLG2 = _S12_TOF;   
	 }  */
   
 /*  if(ThisTime.ByInts.thetime < LastTime.ByInts.thetime){
      ThisTime.ByInts.overflows -= 1;
   }  */
   
   ThisTime.ByInts.overflows = OverFlows;
	 
   Period = ThisTime.AsLong - (*LastTime).AsLong; 
   (*LastTime).AsLong = ThisTime.AsLong;
 
   Encoder_Ticks_Per_M = (unsigned int)((FREQUENCY * S_PER_MIN) / Period);
   return (unsigned char)(Encoder_Ticks_Per_M / TICKS_PER_REV);
}

/***********************************************************************
Function
    void interrupt _Vec_tim0ch6 ControlLaw(void)
    
Description
    Interrupt response routine for updating the control laws every 
    100ms
    
***********************************************************************/

void interrupt _Vec_tim0ch6 ControlLaw (void)  {
   static int ThisRPM;s
	 //static float RPMError;
   //static float LeftDutyRequest;
   static float RightDutyRequest;
   //static float LeftSumError = 0.0;
   static float RightSumError = 0.0;
   
   TIM0_TFLG1 = _S12_C6F;			                  // clear interrupt flag!
   EnableInterrupts;
   
  // Left Side
  if (IsTimerExpired(LEFT_ENCODER_TIMER)) {
     LeftRPM = 0;
  } else {
    ThisRPM = LeftRPM;					       // grab the current RPM
  }
  
  RPMError = LeftRPMTarget - ThisRPM;	 
 
  LeftSumError += RPMError;				 // update error sum
  LeftDutyRequest = ((RPMError * pGain) + (LeftSumError * iGain));
 
  if (LeftDutyRequest > 100) {
    LeftDutyRequest = 100;
    LeftSumError -= RPMError;      // anti-windup
  } else if (LeftDutyRequest < -100) {
    LeftDutyRequest = -100;
    LeftSumError -= RPMError;      // anti-windup
  } else if(LeftDutyRequest > -12 && LeftDutyRequest < 0) {
    LeftDutyRequest = -12;
  } else if(LeftDutyRequest > 0 && LeftDutyRequest < 12) {
    LeftDutyRequest = 12;
  }

  /*
  // Right Side
   if (IsTimerExpired(RIGHT_ENCODER_TIMER)) {
     RightRPM = 0;
  } else {																				 
    ThisRPM = RightRPM;					       // grab the current RPM
  }		 

  RPMError = RightRPMTarget - ThisRPM;
 
  RightSumError += RPMError;				 // update error sum
  RightDutyRequest = ((RPMError * pGain) + (RightSumError * iGain));
 
  if (RightRPMTarget == 0) {
    RightDutyRequest = 0;
    RightSumError -= RPMError;      // anti-windup
  } else if (RightDutyRequest > 100) {
    RightDutyRequest = 100;
    RightSumError -= RPMError;      // anti-windup
  } else if (RightDutyRequest < -100) {
    RightDutyRequest = -100;
    RightSumError -= RPMError;      // anti-windup
  }*/
    
  SimpleMove(LeftDutyRequest, RightDutyRequest);

  TIM0_TC6 = TIM0_TCNT + HUNDREDMS;              // schedule next OC6 event
}

/*------------------------ Test Harness ----------------------------*/

#ifdef MOTORS_TEST

void main(void) {
  char command;
  char left = 0;
  char right = 0;
  OverFlows = 0;
  (void)puts("\r\nIn test harness for motors.c \r\n");
  InitMotors();
  InitTimer();

  EnableInterrupts;
  
  /*
  while(1) {
    SimpleMove(40,40);
    (void)printf("\r\n %d,\t%d", LeftRPM, RightRPM);
  }*/
  

  
  
  while(1) 
  {
  //(void)printf("LEFT; RPMTarg: %d\tDty: %f RIGHT; RPMTarg: %d\tDty: %f\r\n", LeftRPMTarget, LeftDutyRequest, RightRPMTarget, RightDutyRequest);
  (void)printf("LEFT; RPMAct: %d  RPMTarg: %d  ReqDty: %f  ErrSum: %f\tRPMErr: %f\r\n", LeftRPM, LeftRPMTarget, LeftDutyRequest, LeftSumError, RPMError);

  	if(kbhit()) { 
  	    command = getchar(); 
  	    switch(command) {
  	        case '8':
  	          SimpleMove(25, 25);   //Go forward
  	          break;
  	        case '2':
  	          SimpleMove(-25, -25);		//Go reverse
  	          break;
  	        case '4':
  	          SimpleMove(0, 25);   //Rotate Right 
  	          break;
  	        case '6':
  	          SimpleMove(25, 0);   //Rotate Left
  	          break; 								 
  	        case '5':			
  	          SimpleMove(0, 0);   //Stand Still
  	          break; 								 
  	        case '7':			
  	          left = 60;
  	          right = 60;
  	          break;
  	        case '1':
  	          left = 0;
  	          right = 0;
  	          break;
  	        case '3':
  	          left = 90; 
  	          right = 90;
  	          break;
  	        case '+':
  	          pGain += 0.1;
  	          printf("pGain: %f\r\n", pGain);
  	          break;
  	        case '-':
  	          pGain -= 0.1;
  	          printf("pGain: %f\r\n", pGain);
  	          break;
  	        default:
  	          break;
  	    }
  	ControlLawMove(left,right);
  	}	  	
  }
  	
  return;
}
#endif

/*-------------------------- End of file -----------------------------*/




