/*---------------- Include Files -----------------------------*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_Timer.h"
#include "inc/hw_nvic.h"

// the header to get the timing functions
#include "ES_Port.h"

/* include header files for the framework and this service
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ES_DeferRecall.h"

#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"	// Define PART_TM4C123GH6PM in project
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "PWMModule.h"
#include "PropModule.h"
#include <math.h>

/*----------------------------- Module Defines ----------------------------*/
#define MAX_SPEED_POS 127
#define MAX_SPEED_NEG 128

#define PI 3.14
#define FULL_DUTY 95
#define MID_DUTY 75
#define	MIN_DUTY	50
#define BTURN_HI_DUTY 95
#define BTURN_LO_DUTY 85
#define STURN_HI_DUTY 75
#define STURN_LO_DUTY 65

#define MIN_DISTANCE 43
#define MID_DISTANCE 65
#define	MAX_DISTANCE 128

/*----------------------------- Module Variables ----------------------------*/
static float Distance;
static float Angle;
static uint8_t Direction;

/*------------------------------ Module Code ------------------------------*/
void GoForward(uint8_t DutyCycle) {
	//uint8_t DutyCycle = (Speed * (MAX_DUTY - MIN_DUTY)) / (MAX_SPEED_POS) + MIN_DUTY;
	SetDutyLeft(DutyCycle, FORWARD);
	SetDutyRight(DutyCycle, FORWARD);
}

void GoBackward(uint8_t DutyCycle) {
	//uint8_t DutyCycle = (Speed * (MAX_DUTY - MIN_DUTY)) / (MAX_SPEED_NEG) + MIN_DUTY;
	SetDutyLeft(DutyCycle, REVERSE);
	SetDutyRight(DutyCycle, REVERSE);
}

void StopProps(void) {
	SetDutyLeft(1, FORWARD);
	SetDutyRight(1, FORWARD);

}

void TurnRight(uint8_t DutyLeft, uint8_t DutyRight, uint8_t DirLeft, uint8_t DirRight){
	//uint8_t DutyCycle = (Speed * (MAX_DUTY - MIN_DUTY)) / (MAX_SPEED_POS) + MIN_DUTY;
	SetDutyLeft(DutyLeft, DirLeft);
	SetDutyRight(DutyRight, DirRight);
}
void TurnLeft(uint8_t DutyLeft, uint8_t DutyRight, uint8_t DirLeft, uint8_t DirRight){
	//uint8_t DutyCycle = (Speed * (MAX_DUTY - MIN_DUTY)) / (MAX_SPEED_POS) + MIN_DUTY;
	SetDutyLeft(DutyLeft, DirLeft);
	SetDutyRight(DutyRight, DirRight);
}
	
void ProcessPositions(float X, float Y) {
	Distance = sqrt(X*X + Y*Y);
	if (X==0 && Y==0) {
		Angle = 0;
	} else {
		Angle = atan2(Y, X);
	}
	float AngleInDeg = Angle * 180 / PI;
	//printf("Angle is %f \r\n", AngleInDeg);
	uint8_t DutyCycle;
	DutyCycle = (FULL_DUTY - MIN_DUTY) * (Distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE) + MIN_DUTY;
	printf("Duty Cycle is %u\r\n", DutyCycle);
	// If Joystick is near center
	if (Distance < MIN_DISTANCE ) {
		StopProps();
	} else if (AngleInDeg >= -23 && AngleInDeg < 23) {
		// Turns Right in place
		//if (Distance < MID_DISTANCE ) {
			//TurnRight(MID_DUTY, MID_DUTY, FORWARD, REVERSE);
		//} else {
			//TurnRight(FULL_DUTY, FULL_DUTY, FORWARD, REVERSE);
		//}
		TurnRight(DutyCycle, DutyCycle, FORWARD, REVERSE);
	} else if (AngleInDeg >= 23 && AngleInDeg < 68) {
		// Turns right and moves forward
		//if (Distance < MID_DISTANCE ) {
			//TurnRight(STURN_HI_DUTY, STURN_LO_DUTY, FORWARD, FORWARD);
		//} else {
			//TurnRight(BTURN_HI_DUTY, BTURN_LO_DUTY, FORWARD, FORWARD);
		//}
		TurnRight(DutyCycle, 0.90*DutyCycle, FORWARD, FORWARD);
	} else if (AngleInDeg >=68 && AngleInDeg <112) {
		// Goes straight forward
		//if (Distance < MID_DISTANCE) {
			//GoForward(MID_DUTY);
		//} else {
			//printf("MAX FORWARD\r\n");
			//GoForward(FULL_DUTY);
	//	}
		GoForward(DutyCycle);
	} else if (AngleInDeg >=112 && AngleInDeg < 157) {
// Turns left and moves forward		
		//if (Distance < MID_DISTANCE) {
			//TurnLeft(STURN_LO_DUTY, STURN_HI_DUTY, FORWARD, FORWARD);
		//} else {
			//TurnLeft(BTURN_LO_DUTY, BTURN_HI_DUTY, FORWARD, FORWARD);
		//}
		TurnLeft(0.90*DutyCycle, DutyCycle, FORWARD, FORWARD);
	} else if (AngleInDeg >=157 || AngleInDeg < -157) {
		// Turns Left in place
		//if (Distance < MID_DISTANCE) {
			//TurnLeft(MID_DUTY, MID_DUTY, REVERSE, FORWARD);
		//} else {
			// Turns Left at max speed
			//TurnLeft(FULL_DUTY, FULL_DUTY, REVERSE, FORWARD);
		//}
		TurnLeft(DutyCycle, DutyCycle, REVERSE, FORWARD);
	} else if (AngleInDeg >=-157 && AngleInDeg < -112) {
		// Turn left and goes backward
		//if (Distance < MID_DISTANCE) {
			//TurnLeft(STURN_LO_DUTY, STURN_HI_DUTY, REVERSE, REVERSE);
		//} else {
			//TurnLeft(BTURN_LO_DUTY, BTURN_HI_DUTY, REVERSE, REVERSE);
		//}
		TurnLeft(0.90*DutyCycle, DutyCycle, REVERSE, REVERSE);
	} else if (AngleInDeg >= -112 && AngleInDeg < -68) {
		// Goes straight backwards
		//if (Distance < MID_DISTANCE) {
			//GoBackward(MID_DUTY);
		//} else {
			//GoBackward(FULL_DUTY);
		//}
		GoBackward(DutyCycle);
	} else if (AngleInDeg >= -68 && AngleInDeg < -23) {
		// Goes right and backwards
	//	if (Distance < MID_DISTANCE) {
		//	TurnRight(STURN_HI_DUTY, STURN_LO_DUTY, REVERSE, REVERSE);		
		//} else {
			//TurnRight(BTURN_HI_DUTY, BTURN_LO_DUTY, REVERSE, REVERSE);
		//}
		TurnRight(DutyCycle, 0.90*DutyCycle, REVERSE, REVERSE);
	}
}
