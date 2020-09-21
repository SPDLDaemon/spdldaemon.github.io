#include <hidef.h>      /* common defines and macros */
#include <stdio.h>
#include <mc9s12e128.h>     /* derivative information */
#include <S12E128bits.h> /* E128 bit definitions */
#include <bitdefs.h> /* BIT0HI, BIT0LO....definitions */
#include "ADS12.h"
#include "tape.h"


// Definitions of the states and the thresholds
/*
#define TAPE 1
#define NO_TAPE 0
#define TAPE_THRESH 250
*/

//#define TAPE_TEST


// thresholds are the highest reasonable value one of the tape sensors will give for 
// either white or green tape
#define RUNCALIBRATION

#ifndef RUNCALIBRATION
#define WHITE_SET_CENTER 200
#define BLACK_SET_CENTER 800
#define GREEN_SET_CENTER 560
#endif

#define WHITE_BAND 200
#define GREEN_BAND 120
#define BLACK_BAND 120

static int WhiteCenterValue[5];
static int GreenCenterValue[5];
static int BlackCenterValue[5];


// this is a 20ms delay
#define delay 3750

int TAPE_INTEGRAL_STEP = 1;

// Array that has the state information for each of the tape sensors.
int TAPE_STATE_ARRAY[5];

// Array that has the integral of all the values for the sensor 
int TAPE_INTEGRAL_ARRAY[5];

static void CalibrateTapeSensors(){
  
  //PTS Bit 3 is the calibration button
  DDRS &= ~BIT3HI;
  
  //PTS Bit 2 is the calibration LED
  DDRS |= BIT2HI;
  
  //Turn on LED
  PTS |= BIT2HI;
  (void)printf("\r\nPlace robot with center on black and forwards on green.");
  
  //Wait until the button is pressed
  while(PTS & BIT3HI){
    TapeSensorRead();
  }
  
  //Wait until the button is depressed
  while(!(PTS & BIT3HI)){
    TapeSensorRead();
  }
  BlackCenterValue[TP_C] = ADS12_ReadADPin(TP_C);
  GreenCenterValue[TP_FL] = ADS12_ReadADPin(TP_FL);
  GreenCenterValue[TP_FR] = ADS12_ReadADPin(TP_FR);
  (void)printf("\r\n C Black : %d",BlackCenterValue[TP_C]);
  (void)printf("\r\n FL Green : %d",GreenCenterValue[TP_FL]);
  (void)printf("\r\n FR Green : %d",GreenCenterValue[TP_FR]);
  (void)printf("\r\nTurn robot 180 degrees around");
  //Wait until the button is pressed
  while(PTS & BIT3HI){
    TapeSensorRead();
  }
  
  //Wait until the button is depressed
  while(!(PTS & BIT3HI)){
    TapeSensorRead();
  }
  GreenCenterValue[TP_BL] = ADS12_ReadADPin(TP_BL);
  GreenCenterValue[TP_BR] = ADS12_ReadADPin(TP_BR);
  (void)printf("\r\n BL Green : %d",GreenCenterValue[TP_BL]);
  (void)printf("\r\n BR Green : %d",GreenCenterValue[TP_BR]);
  (void)printf("\r\nShift the robot up, so we see white, green (middle), black");
  //Wait until the button is pressed
  while(PTS & BIT3HI){
    TapeSensorRead();
  }
  
  //Wait until the button is depressed
  while(!(PTS & BIT3HI)){
    TapeSensorRead();
  }
  WhiteCenterValue[TP_BL] = ADS12_ReadADPin(TP_BL);
  WhiteCenterValue[TP_BR] = ADS12_ReadADPin(TP_BR);
  GreenCenterValue[TP_C] = ADS12_ReadADPin(TP_C);
  BlackCenterValue[TP_FL] = ADS12_ReadADPin(TP_FL);
  BlackCenterValue[TP_FR] = ADS12_ReadADPin(TP_FR);
  (void)printf("\r\n BL White : %d",WhiteCenterValue[TP_BL]);
  (void)printf("\r\n BR White : %d",WhiteCenterValue[TP_BR]);
  (void)printf("\r\n C Green : %d",GreenCenterValue[TP_C]);
  (void)printf("\r\n FL Black : %d",BlackCenterValue[TP_FL]);
  (void)printf("\r\n FR Black : %d",BlackCenterValue[TP_FR]);
  (void)printf("\r\nTurn the robot 180 degrees and shift up a little (Front white, middle white, back black)");
  //Wait until the button is pressed
  while(PTS & BIT3HI){
    TapeSensorRead();
  }
  
  //Wait until the button is depressed
  while(!(PTS & BIT3HI)){
    TapeSensorRead();
  }
  WhiteCenterValue[TP_FL] = ADS12_ReadADPin(TP_FL);
  WhiteCenterValue[TP_FR] = ADS12_ReadADPin(TP_FR);
  WhiteCenterValue[TP_C] = ADS12_ReadADPin(TP_C);
  BlackCenterValue[TP_BL] = ADS12_ReadADPin(TP_BL);
  BlackCenterValue[TP_BR] = ADS12_ReadADPin(TP_BR);
  (void)printf("\r\n FL White : %d",WhiteCenterValue[TP_FL]);
  (void)printf("\r\n FR White : %d",WhiteCenterValue[TP_FR]);
  (void)printf("\r\n C White : %d",WhiteCenterValue[TP_C]);
  (void)printf("\r\n BL Black : %d",BlackCenterValue[TP_BL]);
  (void)printf("\r\n BR Black : %d",BlackCenterValue[TP_BR]);
  (void)printf("\r\nFinished Calibrating");
  while(PTS & BIT3HI){
    TapeSensorRead();
  }
  
  //Wait until the button is depressed
  while(!(PTS & BIT3HI)){
    TapeSensorRead();
  }
  PTS &= BIT2LO;
}

void TapeSensorInit(void){
 
  int i;

  (void)ADS12_Init("AAAAAAAA");  
  // initalize all states on tape sensor to be off
  // sets all the integral values to zero
  for(i=0;i<5;i++){
    TAPE_INTEGRAL_ARRAY[i] = 0;
    TAPE_STATE_ARRAY[i] = 0; 
  }

  // Initialize the tape sensor timer
  // The condition for ON is: for 100ms, the
  // tape reading must be lower than the threshold voltage.
  
 // Initialize the TIM1 timer (it may be already initialized in beacon sensors)
 
 // ----- TIMER -----------
 TIM2_TSCR1 = _S12_TEN; /* turn the timer system on */
  
 // set pre-scale to /128 or 187.5 kHz clock 
 TIM2_TSCR2 |= _S12_PR0;
 TIM2_TSCR2 |= _S12_PR1;
 TIM2_TSCR2 |= _S12_PR2; 

 //------Output compare (just for 10ms timing)------
 //----------using TC4 for output compare-----------
  
 // (--- use TIM2_TC6 (sixth port on the timer 2) ---------)
  
 //set the fourth TCx on TIM1 to output compare
  TIM2_TIOS |= _S12_IOS6;
 
 // set first event to current time 
  TIM2_TC6 = TIM2_TCNT+delay;
 
 // clear the flag.
  TIM2_TFLG1 = _S12_C6F;
  
  #ifdef RUNCALIBRATION    
  CalibrateTapeSensors();
  #endif
  
  #ifndef RUNCALIBRATION
  for(i = 0; i<5; i++){
    WhiteCenterValue[i] = WHITE_SET_CENTER;
    GreenCenterValue[i] = GREEN_SET_CENTER;
    BlackCenterValue[i] = BLACK_SET_CENTER;
  }
  
  #endif
      
  (void)printf("\r\n Tape Sensors Initialized \r\n");
  
}



void TapeSensorRead(void){

 // For each sensor, every 20ms reads the value of the corresponding AD
 // port and adds it to an integral array. 
 //
 // After 5 cycles of this (100ms) it checks the 
 // if the integral is below a certain threshold area 
 // and updates the states of each of the sensors 
  
  // counter variable.
  int i;
  
  
  // IF 20ms timer is up.   
  if((TIM2_TFLG1 & _S12_C6F) != 0) {
  
    // set up next compare
    TIM2_TC6+=delay;
    TIM2_TFLG1 = _S12_C6F;
  
    // Add all the values to the appropriate integral array.
    for(i=0;i<5;i++)      
    TAPE_INTEGRAL_ARRAY[i] += ADS12_ReadADPin(i);
  
   
    // AFTER 5 cycles!
    // IF 5 cycles have passed
    if(TAPE_INTEGRAL_STEP == 5) 
    {
      
    //FOR each tape sensor
    for(i=0;i<5;i++) {
      // IF value of the integral is lower than NO_TAPE threshold, set the corresponding 
      // state to NO_TAPE.
     
          if((TAPE_INTEGRAL_ARRAY[i] >= (WhiteCenterValue[i]-WHITE_BAND)*5) && (TAPE_INTEGRAL_ARRAY[i] <= (WhiteCenterValue[i]+WHITE_BAND)*5))
             TAPE_STATE_ARRAY[i] = WHITE;
     
          if((TAPE_INTEGRAL_ARRAY[i] >= (GreenCenterValue[i]-GREEN_BAND)*5) && (TAPE_INTEGRAL_ARRAY[i] <= (GreenCenterValue[i]+GREEN_BAND)*5))
              TAPE_STATE_ARRAY[i] = GREEN;
          
          if((TAPE_INTEGRAL_ARRAY[i] >= (BlackCenterValue[i]-BLACK_BAND)*5) && (TAPE_INTEGRAL_ARRAY[i] <= (BlackCenterValue[i]+BLACK_BAND)*5))
            TAPE_STATE_ARRAY[i] = BLACK;
    }
     
     // reset integral step to zero.
     // reset the integral array to zero.  
    for(i=0;i<5;i++)
      TAPE_INTEGRAL_ARRAY[i] = 0;
    
      TAPE_INTEGRAL_STEP = 1; 
      
      return;
    }

      // Add +1 to number of cycles.

      TAPE_INTEGRAL_STEP++;
   }
  else 
    return;
  
}


char getTapeColor(char sensor)
{
  return (char)TAPE_STATE_ARRAY[sensor];
}



#ifdef TAPE_TEST

void main(){

  long i=0;
  TapeSensorInit();
  
  
  while(1) {
    TapeSensorRead();
 
    
    if(i%20000 == 0){
      
      printf("FL: %d,%d " ,(int)getTapeColor(TP_FL), ADS12_ReadADPin(TP_FL));
      printf("  FR: %d,%d " ,(int)getTapeColor(TP_FR), ADS12_ReadADPin(TP_FR));
      printf("  C: %d,%d",(int)getTapeColor(TP_C), ADS12_ReadADPin(TP_C));
      printf("  BL: %d,%d ",(int)getTapeColor(TP_BL), ADS12_ReadADPin(TP_BL));
      printf("  BR: %d,%d",(int)getTapeColor(TP_BR), ADS12_ReadADPin(TP_BR));
      printf("\r\n");

    i = 0;
     
    }
  
    i++;
    
    
    
    }
    


}
#endif
