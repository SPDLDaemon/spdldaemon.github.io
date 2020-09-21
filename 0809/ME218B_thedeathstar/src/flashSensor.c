/***********************************
/ flashSensor.c
/ Functions for detecting a camera
/ flash signal.  The output of the
/ camera flash hardware should be
/ connected to PT1.
***********************************/

//#define FLASH_TEST

#include <stdio.h>
#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>

#include "flashSensor.h"

/***********************************
/ void InitFlash(void)
/ Inputs: None
/ Outputs: None
/ Initializes the Flash sensor
/ module
***********************************/
void initFlash(void){
  DDRU &= ~BIT6HI;
}

/***********************************
/ unsigned char GetFlash(void)
/ Inputs: None
/ Outputs: unsigned char (0 or 1)
/ Returns the state of the flash
/ sensor.
***********************************/
char getFlash(void){
  if(PTU & BIT6HI) return FLSH_ON;
  else return FLSH_OFF;
}

#ifdef FLASH_TEST

void main() 
{

 (void)printf("Running flash test\r\n");
 initFlash();
 
 while(1) {
   if(getFlash() == FLSH_ON) {
    (void)printf("Saw flash!\n\r");
   }
 }

}

#endif