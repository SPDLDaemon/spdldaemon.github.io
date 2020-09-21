//#define BUMP_TEST

#include <stdio.h>
#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "bumpSensor.h"

void initBump(void){
  DDRU &= ~(BIT1HI | BIT0HI);
}

// GetBump dummy declaration for compilation
char getBump(char sensor){
  if(sensor == BMP_F){
    if(PTU & BIT1HI){
      return BMP_OFF;
    } else{
      return BMP_ON;
    }
  } else if(sensor == BMP_B){
    if(PTU & BIT0HI){
      return BMP_OFF;
    }else{
      return BMP_ON;
    }
  }
}

#ifdef BUMP_TEST

void main() 
{

 (void)printf("Running bump test\n\r");
 initBump();
 
 while(1) {
   if(getBump(BMP_F) == BMP_ON) (void)printf("Feeling bump on front!\n\r");
   if(getBump(BMP_B) == BMP_ON) (void)printf("Feeling bump on back!\n\r");
 }
}

#endif