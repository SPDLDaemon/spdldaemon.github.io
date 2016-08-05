
/*----------------------------- Include Files -----------------------------*/
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

#define ALL_BITS (0xff<<2) 

#define LEFT_INPUT1 BIT2HI
#define LEFT_INPUT1_HI BIT2HI
#define LEFT_INPUT1_LO BIT2LO
#define LEFT_PWM_PIN BIT6HI
#define LEFT_PWM_NUM 6

#define RIGHT_INPUT1 BIT3HI
#define RIGHT_INPUT1_HI BIT3HI
#define RIGHT_INPUT1_LO BIT3LO
#define RIGHT_PWM_PIN BIT7HI
#define RIGHT_PWM_NUM 7

#define LIFT_PWM_PIN BIT4HI
#define LIFT_PWM_NUM 4

#define PERIOD 100   /* 100us is 10kHz */

/*----------------------------- Module Defines ----------------------------*/
#define PeriodInMicroS PERIOD
#define PeriodForServo 20000
#define PWMTicksPerMS 40000/32
#define BitsPerNibble 4

/*----------------------------- Module Variables ----------------------------*/
uint8_t LastDutyCycleLeft;
uint8_t LastDutyCycleRight;
uint8_t LastDutyCycleLift;

/*----------------------------- Module Functions ----------------------------*/
static void InitializePorts(void);
static void InitPWM(void);
static void InitLiftPWM(void);

/*----------------------------- Module Code ----------------------------*/

//Function to Initialize Portsqq
void InitPWMModule(void) {
	InitializePorts();
	InitPWM();
	InitLiftPWM();
}

static void InitializePorts(void) {
	//Enable Port B
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R1) != SYSCTL_PRGPIO_R1);
	
	
	//Enable Port E
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R4;
	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R4) != SYSCTL_PRGPIO_R4);
	
	//Set Motor Input Pins on Port B to digital
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (LEFT_INPUT1 | RIGHT_INPUT1);
	//Set Motor Input Pins on Port B as outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (LEFT_INPUT1 | RIGHT_INPUT1);


	//Initially set Motor Input Pins as low to begin 
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= (LEFT_INPUT1_LO & RIGHT_INPUT1_LO);
	//End InitializePorts
}

static void InitPWM(void) {
	//Define volatile Dummy variable
 volatile uint32_t Dummy; 
	//Enable the clock to the PWM Module (PWM0)
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	//Enable the clock to Port B
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	//Select the PWM clock as System Clock/32
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
	(SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	//Make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0);
	//Disable the PWM while initializing
	HWREG( PWM0_BASE+PWM_O_0_CTL ) = 0;
	//Program generator A to go to 1 at rising compare A, 0 on falling compare A
	HWREG( PWM0_BASE+PWM_O_0_GENA) = (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO );
	//Program generator B to go to 1 at rising compare B, 0 on falling compare B
	HWREG( PWM0_BASE+PWM_O_0_GENB) = (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO );
	// Set the PWM period
	HWREG( PWM0_BASE+PWM_O_0_LOAD) = ((PeriodInMicroS * PWMTicksPerMS / 1000) -1)>>1;
	//Enable the PWM outputs
	HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM1EN | PWM_ENABLE_PWM0EN);
	//Configure the Port B pins to be PWM outputs
	//Start by selecting the alternate function for PB6 & PB7
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (LEFT_PWM_PIN | RIGHT_PWM_PIN);
	//Choose to map PWM to those pins, this is a mux value of 4 that we
	//want to use for specifying the function on bits 4, 6, and 7
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) =
	(HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0x00ffffff) + (4<<(LEFT_PWM_NUM*BitsPerNibble)) + (4<<(RIGHT_PWM_NUM*BitsPerNibble));
	//Enable PWM pins on Port B for digital I/O
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (LEFT_PWM_PIN | RIGHT_PWM_PIN);
	//Make PWM pins on Port B into outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (LEFT_PWM_PIN | RIGHT_PWM_PIN);
	
	//Set the initial Duty Cycle to 1 for both motors and lift fan
	SetDutyLeft(1, FORWARD);
	SetDutyRight(1, FORWARD);
	
	//Set the up/down count mode, enable the PWM generator and make
	//both generator updates locally synchronized to zero count
	HWREG(PWM0_BASE+ PWM_O_0_CTL) = (PWM_0_CTL_MODE | PWM_0_CTL_ENABLE |
	PWM_0_CTL_GENAUPD_LS | PWM_0_CTL_GENBUPD_LS);

}

static void InitLiftPWM(void) {
		//Define volatile Dummy variable
 volatile uint32_t Dummy; 
	//Enable the clock to the PWM Module (PWM0)
	HWREG(SYSCTL_RCGCPWM) |= SYSCTL_RCGCPWM_R0;
	//Enable the clock to Port B
	if ((HWREG(SYSCTL_RCGCGPIO) | SYSCTL_RCGCGPIO_R1) != SYSCTL_RCGCGPIO_R1) {
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	}
	//Select the PWM clock as System Clock/32
	HWREG(SYSCTL_RCC) = (HWREG(SYSCTL_RCC) & ~SYSCTL_RCC_PWMDIV_M) |
	(SYSCTL_RCC_USEPWMDIV | SYSCTL_RCC_PWMDIV_32);
	//Make sure that the PWM module clock has gotten going
	while ((HWREG(SYSCTL_PRPWM) & SYSCTL_PRPWM_R0) != SYSCTL_PRPWM_R0);
	//Disable the PWM while initializing
	HWREG( PWM0_BASE+PWM_O_1_CTL ) = 0; 
	//Program generator A to go to 1 at rising compare A, 0 on falling compare A
	HWREG( PWM0_BASE+PWM_O_1_GENA) = (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO );
	// Set the PWM period
	HWREG( PWM0_BASE+PWM_O_1_LOAD) = ((PeriodInMicroS * PWMTicksPerMS) / 1000 -1)>>1;
	//Enable the PWM outputs
	HWREG( PWM0_BASE+PWM_O_ENABLE) |= (PWM_ENABLE_PWM2EN);
	//Configure the Port pins to be PWM outputs
	//Start by selecting the alternate function for PB4 
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (LIFT_PWM_PIN);
	//Choose to map PWM to those pins, this is a mux value of 4 that we
	//want to use for specifying the function on bits 4
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) =
	(HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) & 0xfff0ffff) + (4<<(LIFT_PWM_NUM*BitsPerNibble));
	//Enable PWM pins on Port B for digital I/O
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |=  (LIFT_PWM_PIN);
	//Make PWM pins on Port B into outputs
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= (LIFT_PWM_PIN);
	
	//Set the initial Duty Cycle to 1 for the flywheel
	SetDutyLift(1);
	
	//Set the up/down count mode, enable the PWM generator and make
	//both generator updates locally synchronized to zero count
	HWREG(PWM0_BASE+ PWM_O_1_CTL) = (PWM_1_CTL_MODE | PWM_1_CTL_ENABLE |
	PWM_1_CTL_GENAUPD_LS | PWM_1_CTL_GENBUPD_LS);
	
	printf("Successfully initialized lift fan PWM\r\n");
}

//Function to set duty cycle for the left motor
void SetDutyLeft(uint8_t DutyCycleLeft, uint8_t DirectionLeft){
	//If "FOWARD" paramter is given
	if (DirectionLeft == FORWARD) {
		//Set input 1 pin for the left motor on Port B as low
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= LEFT_INPUT1_LO;
	//Else if "REVERSE" parameter is given	
	} else if (DirectionLeft == REVERSE) {
		//Set Input 1 pin for the left motor on Port B as high
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) |= LEFT_INPUT1_HI;
		//Set DutyCycle to 100-DutyCycle
		DutyCycleLeft = 100 - DutyCycleLeft;
	}
	
	//If DutyCycle is 100 and LastDutyCycle was not 100
	if (DutyCycleLeft == 100 && LastDutyCycleLeft != 100) {
		//Set both Up and Down events to high
		HWREG( PWM0_BASE+PWM_O_0_GENA) = (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ONE);
	//Else if LastDutyCycle was 100 and the current duty cycle is not 100	
	} else if (LastDutyCycleLeft == 100 && DutyCycleLeft != 100) {
		//Set Up event to high and down event to low
		HWREG( PWM0_BASE+PWM_O_0_GENA) = (PWM_0_GENA_ACTCMPAU_ONE | PWM_0_GENA_ACTCMPAD_ZERO);
		//Set Duty Cycle by setting CMPA
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = HWREG( PWM0_BASE+PWM_O_0_LOAD) * (100-DutyCycleLeft) / 100;
	} else {
		//Set Duty cycle by setting CMPA
		HWREG( PWM0_BASE+PWM_O_0_CMPA) = HWREG( PWM0_BASE+PWM_O_0_LOAD) * (100-DutyCycleLeft) / 100;
		//Endif
	}
	//Set LastDutyCycleLeft to current duty cycle
	LastDutyCycleLeft = DutyCycleLeft;
	//End SetDutyLeft
}

void SetDutyRight(uint8_t DutyCycleRight, uint8_t DirectionRight) {
	//If "FORWARD" paramter is give
	if (DirectionRight == FORWARD) {
		//Set bit 5 on Port B as low
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= RIGHT_INPUT1_LO;
	//Else if "REVERSE" parameter is given	
	} else if (DirectionRight == REVERSE) {
		//Set bit 5 on Port B as high
		HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) |= RIGHT_INPUT1_HI;
		//Set DutyCycle to 100-DutyCycle
		DutyCycleRight = 100 - DutyCycleRight;
		//Endif
	}
	
	//If DutyCycle is 100 and LastDutyCycle was not 100
	if (DutyCycleRight == 100 && LastDutyCycleRight != 100) {
		//Set both Up and Down events to high
		HWREG( PWM0_BASE+PWM_O_0_GENB) = (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ONE);
		//HWREG( PWM0_BASE+PWM_O_0_CMPB) = HWREG( PWM0_BASE+PWM_O_0_LOAD) * 50 / 100;
	//Else if LastDutyCycle was 100 and the current duty cycle is not 100	
	} else if (LastDutyCycleRight == 100 && DutyCycleRight != 100) {
		//Set Up event to high and down event to low
		HWREG( PWM0_BASE+PWM_O_0_GENB) = (PWM_0_GENB_ACTCMPBU_ONE | PWM_0_GENB_ACTCMPBD_ZERO);
		//Set Duty Cycle by setting CMPB
		HWREG( PWM0_BASE+PWM_O_0_CMPB) = HWREG( PWM0_BASE+PWM_O_0_LOAD) * (100-DutyCycleRight) / 100;
	//Else	
	} else {
		//Set Duty cycle by setting CMPB
		HWREG( PWM0_BASE+PWM_O_0_CMPB) = HWREG( PWM0_BASE+PWM_O_0_LOAD) * (100-DutyCycleRight) / 100;
		//Endif
	}
	//Set LastDutyCycleRight to current duty cycle
	LastDutyCycleRight = DutyCycleRight;
	//End SetDutyRight
}

void SetDutyLift(uint8_t DutyCycleLift){
	//If DutyCycle is 100 and LastDutyCycle was not 100
	if (DutyCycleLift == 100 && LastDutyCycleLift != 100) {
		//Set both Up and Down events to high
		HWREG( PWM0_BASE+PWM_O_1_GENA) = (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ONE);
	//Else if LastDutyCycle was 100 and the current duty cycle is not 100	
	} else if (LastDutyCycleLift == 100 && DutyCycleLift != 100) {
		//Set Up event to high and down event to low
		HWREG( PWM0_BASE+PWM_O_1_GENA) = (PWM_1_GENA_ACTCMPAU_ONE | PWM_1_GENA_ACTCMPAD_ZERO);
		//Set Duty Cycle by setting CMPA
		HWREG( PWM0_BASE+PWM_O_1_CMPA) = HWREG( PWM0_BASE+PWM_O_1_LOAD) * (100-DutyCycleLift) / 100;
	//Else	
	} else {
		//Set Duty cycle by setting CMPA
		HWREG( PWM0_BASE+PWM_O_1_CMPA) = HWREG( PWM0_BASE+PWM_O_1_LOAD) * (100-DutyCycleLift) / 100;
		//Endif
	}
	//Set LastDutyCycleLeft to current duty cycle
	LastDutyCycleLift = DutyCycleLift;
	
}

