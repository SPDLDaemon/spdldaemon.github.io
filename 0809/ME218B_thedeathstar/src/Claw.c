#include <hidef.h>
#include <stdio.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"
#include "Claw.h"

//#define CLAW_TEST

int FirstStep1 = 1; 
int DriveStep1 = 0;
int Operation1 = FULL;

int FirstStep2 = 1; 
int DriveStep2 = 0;
int Operation2 = FULL;


static char CLAW_STATE_1 = DR_OPEN;
static char CLAW_STATE_2 = DR_OPEN;

int CONV_TIME_DELAY = 15;
int NumStepsClosing1 = 0;
int NumStepsOpening1 = 0;
int NumStepsClosing2 = 0;
int NumStepsOpening2 = 0;

int FullStep[4][6] = { {1,0,1,0,100,100}, {1,0,0,1,100,100}, 
                    {0,1,0,1,100,100},{0,1,1,0,100,100}};

void ClawInit(){

 // Initialize the TIM2 timer (it may be already initialized in beacon sensors)
 
 // ----- TIMER -----------
 TIM2_TSCR1 = _S12_TEN; /* turn the timer system on */
  
 // set pre-scale to /128 or 187.5 kHz clock 
 TIM2_TSCR2 |= _S12_PR0;
 TIM2_TSCR2 |= _S12_PR1;
 TIM2_TSCR2 |= _S12_PR2; 

 //------Output compare (just for 10ms timing)------
 //----------using TC4 for output compare-----------
  
 // (--- use TIM2_TC4 (fourth timer on tim2, PU0 port on E128) ---------)
  
 //set the fourth TCx on TIM1 to output compare
  TIM2_TIOS |= _S12_IOS4;
 
 // set first event to current time 
  TIM2_TC4 = TIM2_TCNT+step_delay;
 
 // clear the flag.
  TIM2_TFLG1 = _S12_C4F;
      
 //enable interrupts when rising edge happens
 // TIM2_TIE |= _S12_C4I;

EnableInterrupts;

  (void)printf("Output compare initialized for step timing... \r\n");
}

char getDoor(char door){

if(door == DR_FRONT)
 return CLAW_STATE_1;
else if(door == DR_BACK);
 return CLAW_STATE_2;

}


void commandDoor(char door, char action) {
  if(door == DR_FRONT) {
    if(action == DR_OPEN) {
      OpenClaw1();
    } else if(action == DR_CLSD) {
      CloseClaw1();
    }
  } else if(door == DR_BACK) {
    if(action == DR_OPEN) {
      OpenClaw2();
    } else if(action == DR_CLSD) {
      CloseClaw2();
    }
  }
}

/* Control of Claw 1 functions */

void CloseClaw1(){
 CLAW_STATE_1 = DR_CLSING;
 TIM2_TC4=TIM2_TCNT+step_delay;

 // enable interrupts when claw is closing
 TIM2_TIE |= _S12_C4I;
}


void OpenClaw1(){
 CLAW_STATE_1 = DR_OPENING;
 TIM2_TC4=TIM2_TCNT+step_delay;
 
 // enable interrupts when claw is closing
 TIM2_TIE |= _S12_C4I;
}

void ResetToOpen1(){
    CLAW_STATE_1 = DR_OPEN;
    NumStepsOpening1 = 0;   
    FirstStep1 = 1;  
}

void ResetToClosed1() {
     CLAW_STATE_1 = DR_CLSD;
    NumStepsClosing1 = 0;
    FirstStep1 = 1;      
}

/* Control of Claw 2 functions */

void CloseClaw2(){
 CLAW_STATE_2 = DR_CLSING;
 TIM2_TC4=TIM2_TCNT+step_delay;

 // enable interrupts when claw is closing
 TIM2_TIE |= _S12_C4I;
}


void OpenClaw2(){
 CLAW_STATE_2 = DR_OPENING;
 TIM2_TC4=TIM2_TCNT+step_delay;
 
 // enable interrupts when claw is closing
 TIM2_TIE |= _S12_C4I;
}

void ResetToOpen2(){
    CLAW_STATE_2 = DR_OPEN;
    NumStepsOpening2 = 0;   
    FirstStep2 = 1;  
}

void ResetToClosed2() {
     CLAW_STATE_2 = DR_CLSD;
    NumStepsClosing2 = 0;
    FirstStep2 = 1;      
}




void interrupt _Vec_tim2ch4 StepSignalInterrupt (void) {

  // set up the next compare 
  TIM2_TC4 += step_delay;
  // clear the flag for the compare
  TIM2_TFLG1 = _S12_C4F;


  if(CLAW_STATE_1 == DR_OPENING) {
  
    if(NumStepsOpening1 == STEPS_TO_OPEN)
    {
      // get out of dr_opening state
      // change to door open state
      // change first step back to 1
      // change NumStepsOpening back to zero
        
    ResetToOpen1();
    TIM2_TIE &= ~_S12_C4I; // turn off interrupts
    } 
   
  else if(NumStepsOpening1 != STEPS_TO_OPEN)  
    {
      FirstStepHandle1();
      DecrementDriveStep1();
      StepOutput1();
    }
  
  } 

  else if(CLAW_STATE_1 == DR_CLSING)
  {
  
   if(NumStepsClosing1 == STEPS_TO_CLOSE)
   {
    // get out of door closing step
    // change door to closed state
    // change first step back to 1
    // change NumStepsClosing back to zero 
    ResetToClosed1();  
    TIM2_TIE &= ~_S12_C4I;  // turn off interrupts
   } 
   
   
   else if(NumStepsClosing1 != STEPS_TO_CLOSE)    
   {
    FirstStepHandle1();
    IncrementDriveStep1();
    StepOutput1();
   }
  
  }

  
 /* Second claw operations */
 
    if(CLAW_STATE_2 == DR_OPENING) {
  
    if(NumStepsOpening2 == STEPS_TO_OPEN)
    {
      // get out of dr_opening state
      // change to door open state
      // change first step back to 1
      // change NumStepsOpening back to zero
        
    ResetToOpen2();
    TIM2_TIE &= ~_S12_C4I; // turn off interrupts
    } 
   
  else if(NumStepsOpening2 != STEPS_TO_OPEN)  
    {
      FirstStepHandle2();
      DecrementDriveStep2();
      StepOutput2();
    }
  
  } 

  else if(CLAW_STATE_2 == DR_CLSING)
  {
  
   if(NumStepsClosing2 == STEPS_TO_CLOSE)
   {
    // get out of door closing step
    // change door to closed state
    // change first step back to 1
    // change NumStepsClosing back to zero 
    ResetToClosed2();  
    TIM2_TIE &= ~_S12_C4I;  // turn off interrupts
   } 
   
   
   else if(NumStepsClosing2 != STEPS_TO_CLOSE)    
   {
    FirstStepHandle2();
    IncrementDriveStep2();
    StepOutput2();
   }
  
  }
  
  
  

}



void FirstStepHandle1()
{
  // Starts operation for first step   
    if(FirstStep1 == 1)
    {

   //  printf("First step happened \r\n");
      FirstStep1 = 0;
     
      if(Operation1 == FULL) {
      OutputToPTP_W_inverter1(FullStep[0]);
      }
      
    }
}



void FirstStepHandle2()
{
  // Starts operation for first step   
    if(FirstStep2 == 1)
    {

   //  printf("First step happened \r\n");
      FirstStep2 = 0;
     
      if(Operation2 == FULL) {
      OutputToPTP_W_inverter2(FullStep[0]);
      }
      
    }
}


void OutputToPTP_W_inverter1(int arr[6]) 
{
    if((arr[0] == 1) && (arr[1] == 0))
    {
        PTP |= BIT0HI;
    }
    else
        PTP &= BIT0LO;
        
   if((arr[2] == 1) && (arr[3] == 0))
   {
        PTP |= BIT2HI;
   } 
    else
        PTP &= BIT2LO;
}



void OutputToPTP_W_inverter2(int arr[6]) 
{
    if((arr[0] == 1) && (arr[1] == 0))
    {
        PTP |= BIT1HI;
    }
    else
        PTP &= BIT1LO;
        
   if((arr[2] == 1) && (arr[3] == 0))
   {
        PTP |= BIT3HI;
   } 
    else
        PTP &= BIT3LO;
}



void StepOutput1()
{
   if(Operation1 == FULL) 
      {
        // for proper indexing
        
        DriveStep1 = DriveStep1%4;
        
        if(DriveStep1 == -1)
          DriveStep1 = 3;

        OutputToPTP_W_inverter1(FullStep[DriveStep1]);
        // for project testing
      }
        
}

void StepOutput2()
{
   if(Operation2 == FULL) 
      {
        // for proper indexing
        
        DriveStep2 = DriveStep1%4;
        
        if(DriveStep2 == -1)
          DriveStep2 = 3;

        OutputToPTP_W_inverter2(FullStep[DriveStep2]);
        // for project testing
      }
        
}



void IncrementDriveStep1() 
{
     DriveStep1 = DriveStep1+1;
     NumStepsClosing1++;     
}

void DecrementDriveStep1() 
{
     DriveStep1 = DriveStep1-1;
     NumStepsOpening1++;     
}


void IncrementDriveStep2() 
{
     DriveStep2 = DriveStep2+1;
     NumStepsClosing2++;     
}

void DecrementDriveStep2() 
{
     DriveStep2 = DriveStep2-1;
     NumStepsOpening2++;     
}












#ifdef CLAW_TEST

int main() 
{
 int i;

 ClawInit();
 DDRP = 0xFF;

 CloseClaw1();

 while(CLAW_STATE_1 != DR_CLSD){
  
 if(i%1000 == 0){
  (void)printf("Should be closing state: %d \r\n",getDoor(DR_FRONT));
  }
 i++;
 }
 i = 0;
 (void)printf("door now closed with state value: %d \r\n",getDoor(DR_FRONT));
 
 
 OpenClaw1();

 while(CLAW_STATE_1 != DR_OPEN){
  
 if(i%1000 == 0) {
  (void)printf("Should be opening state: %d \r\n",getDoor(DR_FRONT));
  }
 
 i++;
 }
 
 (void)printf("door now open with state value: %d \r\n",getDoor(DR_FRONT));


 CloseClaw2();

 while(CLAW_STATE_2 != DR_CLSD){
  
 if(i%1000 == 0){
  (void)printf("Should be closing state: %d \r\n",getDoor(DR_BACK));
  }
 i++;
 }
 i = 0;
 (void)printf("door now closed with state value: %d \r\n",getDoor(DR_BACK));
 
 
 OpenClaw2();

 while(CLAW_STATE_2 != DR_OPEN){
  
 if(i%1000 == 0) {
  (void)printf("Should be opening state: %d \r\n",getDoor(DR_BACK));
  }
 
 i++;
 }
 
 (void)printf("door now open with state value: %d \r\n",getDoor(DR_BACK));

 
 return 0;
}

#endif
