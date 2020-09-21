#include <stdio.h>
/********************************************
/ lightSensor.c
/ The module for sensing, reading, and
/ interpreting the incoming IR signal.
/ Expects a duty cycle from the hardware
/ at a 1.25KHz frequency.
/ Author: Andrew Parra
*********************************************/

#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include "S12eVec.h"

#define PERIOD 2402			//800us
#define MAXCOUNT 65536	//3MHz Clock, 22ms for full cycle
#define OC7_PERIOD 1000	//
#define MAXTIMEFORZERODC 25

//#define TESTLIGHTS

static int dutyCycle[8];
static int lastTimeFlag[8];

/********************************************
/ void initLights(void)
/ Input: None
/ Output: None
/ Initializes the IR sensing beacons.
/ Turns on the InputCapture on TIM0_IC4
/ Turns on the OutputCompare on TIM_OC5
/ Author: Andrew Parra
********************************************/
void initLights(void){
  int i;
  
	DDRT &= 0x00;                           //Set all of Port T to be an input
	
	// Enable Timer 0
	TIM0_TSCR1 = _S12_TEN;
	
	// Set Timer 0 to a 3MHz clock rate	
	TIM0_TSCR2 = (_S12_PR1 | _S12_PR0);
	
	// Enable Timer 1
	TIM1_TSCR1 = _S12_TEN;
	
	// Set Timer 1 to a 3MHz clock rate	
	TIM1_TSCR2 = (_S12_PR1 | _S12_PR0);

 // ----- TIMER -----------
 TIM2_TSCR1 = _S12_TEN; /* turn the timer system on */
  
 // set pre-scale to /128 or 187.5 kHz clock 
 TIM2_TSCR2 |= _S12_PR0;
 TIM2_TSCR2 |= _S12_PR1;
 TIM2_TSCR2 |= _S12_PR2; 

	//Set up TIM0_IC4 as an IC
	TIM0_TIE |= _S12_C4I;                    // Enable TIM0_TC4 as an interrupt
	TIM0_TCTL3 |= (_S12_EDG4A | _S12_EDG4B); // Set TIM_0 IC4 to capture any edge
	TIM0_TFLG1 = _S12_C4F;                   // Clear TIM_0 IC4 flag
	
	//Set up TIM0_IC5 as an IC
	TIM0_TIE |= _S12_C5I;                    // Enable TIM0_TC5 as an interrupt
	TIM0_TCTL3 |= (_S12_EDG5A | _S12_EDG5B); // Set TIM_0 IC5 to capture any edge
	TIM0_TFLG1 = _S12_C5F;                   // Clear TIM_0 IC5 flag	
	
	//Set up TIM0_IC6 as an IC
	TIM0_TIE |= _S12_C6I;                    // Enable TIM0_TC6 as an interrupt
	TIM0_TCTL3 |= (_S12_EDG6A | _S12_EDG6B); // Set TIM_0 IC6 to capture any edge
	TIM0_TFLG1 = _S12_C6F;                   // Clear TIM_0 IC6 flag
	
	//Set up TIM0_IC7 as an IC
	TIM0_TIE |= _S12_C7I;                    // Enable TIM0_TC7 as an interrupt
	TIM0_TCTL3 |= (_S12_EDG7A | _S12_EDG7B); // Set TIM_0 IC7 to capture any edge
	TIM0_TFLG1 = _S12_C7F;                   // Clear TIM_0 IC7 flag

	//Set up TIM1_IC4 as an IC
	TIM1_TIE |= _S12_C4I;                    // Enable TIM1_TC4 as an interrupt
	TIM1_TCTL3 |= (_S12_EDG4A | _S12_EDG4B); // Set TIM_1 IC4 to capture any edge
	TIM1_TFLG1 = _S12_C4F;                   // Clear TIM_1 IC4 flag
	
	//Set up TIM1_IC5 as an IC
	TIM1_TIE |= _S12_C5I;                    // Enable TIM1_TC5 as an interrupt
	TIM1_TCTL3 |= (_S12_EDG5A | _S12_EDG5B); // Set TIM_1 IC5 to capture any edge
	TIM1_TFLG1 = _S12_C5F;                   // Clear TIM_1 IC5 flag	
	
	//Set up TIM1_IC6 as an IC
	TIM1_TIE |= _S12_C6I;                    // Enable TIM1_TC6 as an interrupt
	TIM1_TCTL3 |= (_S12_EDG6A | _S12_EDG6B); // Set TIM_1 IC6 to capture any edge
	TIM1_TFLG1 = _S12_C6F;                   // Clear TIM_1 IC6 flag
	
	//Set up TIM1_IC7 as an IC
	TIM1_TIE |= _S12_C7I;                    // Enable TIM1_TC7 as an interrupt
	TIM1_TCTL3 |= (_S12_EDG7A | _S12_EDG7B); // Set TIM_1 IC7 to capture any edge
	TIM1_TFLG1 = _S12_C7F;                   // Clear TIM_1 IC7 flag
	
	 // set pre-scale to /128 or 187.5 kHz clock 
  TIM2_TSCR2 |= _S12_PR0;
  TIM2_TSCR2 |= _S12_PR1;
  TIM2_TSCR2 |= _S12_PR2; 
	
	 // Set up TIM2_OC7 as an OC
   TIM2_TIE |= _S12_C7I;	            		// Enable TIM2_TC7 as an interrupt
   TIM2_TIOS |= _S12_IOS7;		    				// Set TIM2_TC7 to output capture
   //TIM2_TCTL1 &= ~(_S12_OL7 | _S12_OM7);  // There is no pin connected to TIM2_OC7
   TIM2_TC5 = TIM2_TCNT + OC7_PERIOD;  		// Set first compare for TIM2_OC7
   TIM2_TFLG1 = _S12_C7F;		    					// Clear flag for TIM2_OC7
   
   for(i=0; i<8; i++){
     dutyCycle[i] = 0;
   }
}

/********************************************
/ void GetDutyCycle(void)
/ Input: None
/ Output: Integer Duty Cycle (Value between 0-100)
/ Returns the currently read duty cycle from
/ the IR sensor.  Note, the IR sensor is
/ looking for a 1.25KHz IR light.
/ Author: Andrew Parra
********************************************/
char getIRDutyCycle(char sensor){
	return (char)dutyCycle[sensor];
}

void interrupt _Vec_tim0ch4 ICInterrupt0(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM0_TC4;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT0HI){
		lastTime = currTime;
		lastTimeFlag[0] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[0] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM0_TFLG1 = _S12_C4F;	//Reset the interrupt flag
}

void interrupt _Vec_tim0ch5 ICInterrupt1(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM0_TC5;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT1HI){
		lastTime = currTime;
		lastTimeFlag[1] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[1] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM0_TFLG1 = _S12_C5F;	//Reset the interrupt flag
}

void interrupt _Vec_tim0ch6 ICInterrupt2(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM0_TC6;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT2HI){
		lastTime = currTime;
		lastTimeFlag[2] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[2] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM0_TFLG1 = _S12_C6F;	//Reset the interrupt flag
}

void interrupt _Vec_tim0ch7 ICInterrupt3(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM0_TC7;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT3HI){
		lastTime = currTime;
		lastTimeFlag[3] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[3] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM0_TFLG1 = _S12_C7F;	//Reset the interrupt flag
}

void interrupt _Vec_tim1ch4 ICInterrupt4(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM1_TC4;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT4HI){
		lastTime = currTime;
		lastTimeFlag[4] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[4] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM1_TFLG1 = _S12_C4F;	//Reset the interrupt flag
}

void interrupt _Vec_tim1ch5 ICInterrupt5(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM1_TC5;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT5HI){
		lastTime = currTime;
		lastTimeFlag[5] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[5] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM1_TFLG1 = _S12_C5F;	//Reset the interrupt flag
}

void interrupt _Vec_tim1ch6 ICInterrupt6(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM1_TC6;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT6HI){
		lastTime = currTime;
		lastTimeFlag[6] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[6] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM1_TFLG1 = _S12_C6F;	//Reset the interrupt flag
}

void interrupt _Vec_tim1ch7 ICInterrupt7(void){
	int currTime;
	int sawDutyCycle;
	static int lastTime = 0;
	static int integrateTimes = 0;
	static int integrateAvg = 0;
	
	currTime = TIM1_TC7;		//Record the current time
	//If we are on a rising edge
	if(PTT & BIT7HI){
		lastTime = currTime;
		lastTimeFlag[7] = 0;
	}
	//If we are on a falling edge
	else{
	  //If an overcount occured
		if(lastTime > currTime){
		  sawDutyCycle = 100-(((currTime<<1) - (lastTime+currTime))/(PERIOD/100));
			integrateAvg += sawDutyCycle; 
		}
		// else we have a non overcount situation
		else{
		  sawDutyCycle = 100-((currTime-lastTime)/(PERIOD/100));
			integrateAvg += sawDutyCycle;
		}
		//Check for irregularities on the duty cycle
		if(sawDutyCycle <= 0 || sawDutyCycle > 100){
		  integrateAvg -= sawDutyCycle;
		  integrateTimes--;
		}
		//Increment the amount of times we saw a full duty cycle
		integrateTimes++;
	}
	if(integrateTimes == 200){
	  dutyCycle[7] = integrateAvg/200;
	  integrateAvg = 0;
	  integrateTimes = 0;
	}
	TIM1_TFLG1 = _S12_C7F;	//Reset the interrupt flag
}

void interrupt _Vec_tim2ch7 OutputCompareInterrupt(void){
	int i;
	
	//PTU ^= BIT5HI;
	
	for(i=0; i<8; i++){
		lastTimeFlag[i]++;
  	if(lastTimeFlag[i] == MAXTIMEFORZERODC){
	  	dutyCycle[i] = 0;
	  }
	}

	// Clear the flag
	TIM2_TFLG1 = _S12_C7F;

	// Set the next output compare
	TIM2_TC7 += OC7_PERIOD;
	
}

#ifdef TESTLIGHTS
void main(){
  char i;
  char duty;
	initLights();
	EnableInterrupts;
	(void)printf("\r\n");
	while(1){
	  for(i=0; i<8; i++){
	    duty = getIRDutyCycle(i);
		  (void)printf("DC%d: ", i);
		  (void)printf("%d    ",duty);
	  }
	  (void)printf("\r\n");
	}
}
#endif
