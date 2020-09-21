//*********************************************************************
// CoachEvents.c
//*********************************************************************
// Stuart Calder; ME218C
// Mississippi Queen Project: 05/09
//*********************************************************************

//*********************************************************************
// Header File Includes
//*********************************************************************

#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include <stdio.h>
#include "S12eVec.h"
#include "ADS12.H"
#include "TimerS12.h"
#include "CommunicationsTest.h"
#include "encoder.h"
#include "seg7_display.h"

//*********************************************************************
// Test Define
//*********************************************************************
//#define TEST1

//*********************************************************************
// Constant Defines
//*********************************************************************

#define ADPORTIO "IIIIIIAA"

#define LeverCenterVoltage 603
#define Division 27


//*********************************************************************
// Variable Definitions
//*********************************************************************

static char AoCPressed = 0;

//*********************************************************************
// Function Definitions
//*********************************************************************

// Initializes the analog ports
void PTAD_INIT(void);


//*********************************************************************
// Functions
//*********************************************************************

//*********************************************************************
// AD Port Initialization Function
//*********************************************************************

void PTAD_INIT(void)
{

    if(ADS12_Init(ADPORTIO) == ADS12_OK){
      

      (void)printf(" AD Port Initialized \n\r");
    }
    else{
      
      (void)printf(" Error Initializing Port AD \n\r");
    }
    return; 

}


//*********************************************************************
// COACH Initialization Function
//*********************************************************************

void COACH_INIT(void){
  
  PTAD_INIT();
  
  // Set P port 0 to be input and ports 1-5 to be outputs
  DDRP = 0x3E;
  
  // Initialize the encoder
  initEncoder();
  
  // Initialize the seven-segment displays
  initSeg7();
  
  // Set initial output values (all low)
  PTP &= BIT1LO & BIT3LO & BIT4LO; 
}


//*********************************************************************
// Crew Jettison Check Function
//*********************************************************************

char CrewJettison(void) {
  
  static char OldStatus = 0;
  char JettisonStatus;
  
  // Check the status of the jettison command bit
  // Active low switch + active low functionality = active high
  if((PTAD&BIT5HI)!=0) {
    
    JettisonStatus = 1;
  } 
  else {
    
    JettisonStatus = 0;
  }
  
  // Test if the jettison command has been initiated
  if((JettisonStatus==1)&&(OldStatus==0)) {
    
    OldStatus = JettisonStatus;
    return 1;
  } 
  else{
    OldStatus = JettisonStatus;
    return 0;
  }
}

//*********************************************************************
// Check Mutiny Suppression Function
//*********************************************************************

char CheckMutinySuppression(void) {
  
  static char OldStatus = 0;
  char MutinyStatus;
  
  // Check the status of the mutiny bit
  if((PTAD&BIT4HI)!=0) {
    
    MutinyStatus = 1;
  } 
  else {
    
    MutinyStatus = 0;
  }
  
  // Test if the mutiny suppresion has been initiated
  if((MutinyStatus==1)&&(OldStatus==0)) {
    
    OldStatus = MutinyStatus;
    return 1;
  } 
  else{
    OldStatus = MutinyStatus;
    return 0;
  }
}

//*********************************************************************
// Check Assertion of Command Function
//*********************************************************************

char CheckAssertionCommand(void) {
  
  static char OldStatus = 0;
  char AssertionStatus;
  
  // Check the status of the assertion of command bit
  // Active low switch --> active low
  if((PTAD&BIT6HI)==0) {
    
    AssertionStatus = 1;
    AoCPressed = 1;
  } 
  else {
    
    AssertionStatus = 0;
  }
  
  // Test if the assertion of command has been initiated
  if((AssertionStatus==1)&&(OldStatus==0)) {
    
    OldStatus = AssertionStatus;
    return 1;
  } 
  else{
    OldStatus = AssertionStatus;
    return 0;
  }
}

//*********************************************************************
// Free Digital Command Activation Function
//*********************************************************************
char FreeDigitalActivate(void) {
  
  static char OldStatus = 0;
  char FreeDigitalStatus;
  
  // Check the status of the extra digital command bit
  // Active low switch --> active low
  if((PTAD&BIT6HI)!=0) {  //Test if the button has been pressed
    
    if(!AoCPressed) {     //Test if this is just the remnants of AoC
      FreeDigitalStatus = 1;
    } 
    
    else{
      FreeDigitalStatus = 0;
    }
  } 
  
  else {
    FreeDigitalStatus = 0;
    AoCPressed = 0;
  }
  
  // Test if the digital command has been initiated
  if((FreeDigitalStatus==1)&&(OldStatus==0)) {
    
    OldStatus = FreeDigitalStatus;
    return 1;
  } 
  
  else{
    OldStatus = FreeDigitalStatus;
    return 0;
  }
}
  


//*********************************************************************
// Free Analog Command Activation Function
//*********************************************************************
char FreeAnalogActivate(void) {
  
  static char OldStatus = 0;
  char FreeAnalogStatus;
  
  // Check the status of the extra analog command bit
  // Active low switch --> active low
  if((PTAD&BIT7HI)==0) {
    
    FreeAnalogStatus = 1;
  } 
  else {
    
    FreeAnalogStatus = 0;
  }
  
  // Test if the analog command has been initiated
  if((FreeAnalogStatus==1)&&(OldStatus==0)) {
    
    OldStatus = FreeAnalogStatus;
    return 1;
  } 
  else{
    OldStatus = FreeAnalogStatus;
    return 0;
  }
}
  

//*********************************************************************
// Get Translational Velocity Function
//*********************************************************************

char GetTranslationalVelocity (void){
  
  char VelocityUN;
  int LeverAD;
  
  // Read the lever AD port
  LeverAD = ADS12_ReadADPin(0);
  
  // Determine what duty cycle should be sent to the motor 
  // Send in the form specified by the Communications Protocol
  
  //REVERSE
  
  if((LeverAD < LeverCenterVoltage)&&(LeverAD > (LeverCenterVoltage - (1*Division))))
  {
    
    // Send 0% forward velocity
    VelocityUN = 0b1000;
    
    return VelocityUN<<4;
  } 
  
  else if ((LeverAD < (LeverCenterVoltage-(1*Division))&&(LeverAD > (LeverCenterVoltage-(2*Division))))) 
  {
    
    // Send 15% forward velocity  
    VelocityUN = 0b1001;
    
    return VelocityUN<<4;
  } 
  
  else if ((LeverAD < (LeverCenterVoltage-(2*Division))&&(LeverAD > (LeverCenterVoltage-(3*Division)))))
  {
    
    // Send 29% forward velocity  
    VelocityUN = 0b1010;
    
    return VelocityUN<<4;
  } 
  
  else if ((LeverAD < (LeverCenterVoltage-(3*Division))&&(LeverAD > (LeverCenterVoltage - (4*Division))))) 
  {
    
    // Send 45% forward velocity  
    VelocityUN = 0b1011;
    
    return VelocityUN<<4;
  } 
  
  else if ((LeverAD < (LeverCenterVoltage-(4*Division))&&(LeverAD > (LeverCenterVoltage - (5*Division))))) 
  {
    
    // Send 59% forward velocity  
    VelocityUN = 0b1100;
    
    return VelocityUN<<4;
  } 
  
  else if ((LeverAD < (LeverCenterVoltage-(5*Division))&&(LeverAD > (LeverCenterVoltage - (6*Division)))))
  {
    
    // Send 75% forward velocity  
    VelocityUN = 0b1101<<4;
   
    return VelocityUN;
  } 
  
  else if ((LeverAD < (LeverCenterVoltage-(6*Division))&&(LeverAD > (LeverCenterVoltage - (7*Division)))))
  {
    
    // Send 89% forward velocity  
    VelocityUN = 0b1110<<4;
    
    return VelocityUN;
  } 
  
  else if (LeverAD < (LeverCenterVoltage-(7*Division)))
  {
    
    // Send 100% forward velocity  
    VelocityUN = 0b1111<<4;
    
    return VelocityUN;
  } 
  
  
  //FORWARD
  
  else if((LeverAD > LeverCenterVoltage)&&(LeverAD < (LeverCenterVoltage + (1*Division)))) 
  {
   
    // Send 0% forward velocity
    VelocityUN = 0b0000<<4;
    
    return VelocityUN;
  } 
  
  else if ((LeverAD > (LeverCenterVoltage+(1*Division))&&(LeverAD < (LeverCenterVoltage + (2*Division))))) 
  {
    
    // Send 15% forward velocity  
    VelocityUN = 0b0001<<4;
    
    return VelocityUN;
  } 
  
  else if ((LeverAD > (LeverCenterVoltage+(2*Division))&&(LeverAD < (LeverCenterVoltage + (3*Division)))))
  {
    
    // Send 29% forward velocity  
    VelocityUN = 0b0010<<4;
    
    return VelocityUN;
  } 
  
  else if ((LeverAD > (LeverCenterVoltage+(3*Division))&&(LeverAD < (LeverCenterVoltage + (4*Division)))))
  {
    
    // Send 45% forward velocity  
    VelocityUN = 0b0011<<4;
    
    return VelocityUN;
  } 
  
  else if ((LeverAD > (LeverCenterVoltage+(4*Division))&&(LeverAD < (LeverCenterVoltage + (5*Division)))))
  {
    
    // Send 59% forward velocity  
    VelocityUN = 0b0100<<4;
    
    return VelocityUN;
  } 
  
  else if ((LeverAD > (LeverCenterVoltage+(5*Division))&&(LeverAD < (LeverCenterVoltage + (6*Division)))))
  {
    
    // Send 75% forward velocity  
    VelocityUN = 0b0101<<4;
    
    return VelocityUN;
  } 
  
  else if ((LeverAD > (LeverCenterVoltage+(6*Division))&&(LeverAD < (LeverCenterVoltage + (7*Division)))))
  {
    
    // Send 89% forward velocity  
    VelocityUN = 0b0110<<4;
    
    return VelocityUN;
  } 
  
  else if (LeverAD > (LeverCenterVoltage+(7*Division)))
  {
    
    // Send 100% forward velocity  
    VelocityUN = 0b0111<<4;
    
    return VelocityUN;
  }
}

//*********************************************************************
// Get Free Analog Parameter Function
//*********************************************************************

char GetFreeAnalogParameter (void){
  
  char ParameterFA;
  int LeverAD;
  
  // Read the lever AD port
  LeverAD = ADS12_ReadADPin(1);
  
  // Shift and scale to get this value between -100 and 100
  ParameterFA = (char)(100*(LeverAD-LeverCenterVoltage)/LeverCenterVoltage);
  
  // Return this answer
  return ParameterFA;
}

//*********************************************************************
// Test Harness #1
//*********************************************************************

#ifdef TEST1

  void main (void){
  
  char V=0;
  unsigned int V_Time;
  
  COACH_INIT();
  
  TMRS12_Init(TMRS12_RATE_16MS);
  
  V_Time = TMRS12_GetTime();
  
  while(1) {
    
    
    if((TMRS12_GetTime()-V_Time)>61){
     
      V = GetTranslationalVelocity();
      V_Time = TMRS12_GetTime();
    
      (void)printf("The velocity is: %x ",V);
    }
  }
  }
  
#endif







