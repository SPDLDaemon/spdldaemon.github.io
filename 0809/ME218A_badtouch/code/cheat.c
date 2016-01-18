#include "our_c32.h"
#include "cheat.h"
#include <stdio.h>
#include <timers12.h>

//#define DEBUG
#define Refresh 20

//Port/pin constants
#define FAN_PORT M
#define FAN_PIN 0

void InitCheat(void) {
	WritePort(FAN_PORT, FAN_PIN ,LO);
	SetDDR(FAN_PORT, FAN_PIN, WRITE);
}

void ExecuteCheat(char state){
    if(state == ON)
      WritePort(FAN_PORT, FAN_PIN, HI);
    else
      WritePort(FAN_PORT, FAN_PIN, LO);
	}			

#ifdef DEBUG

//Test Harness for ExecuteCheat Function
void main(void){
  char dummy;
  char state;

  InitCheat();
  TMRS12_Init(TMRS12_RATE_1MS);
  while(1){
    dummy = 'b';
	  while(dummy == 'b') {
			state=ON;
			ExecuteCheat(state);
			dummy = getchar();	
		}
    while(dummy == 'v') {
      state = OFF;
      ExecuteCheat(state);
      dummy = getchar();
    }
  }
  return;
}
#endif
	
	
	
	
