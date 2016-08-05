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
#include "driverlib/pin_map.h"	
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "DMCModule.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define ALL_BITS (0xff<<2)

#define PAIR_OUTPUT GPIO_PIN_4
#define PAIR_OUTPUT_HI	BIT4HI
#define	PAIR_OUTPUT_LO BIT4LO

#define COLOR_OUTPUT GPIO_PIN_3
#define COLOR_OUTPUT_HI	BIT3HI
#define	COLOR_OUTPUT_LO BIT3LO

#define BLUE 1
#define RED 0

/*----------------------------- Module Variables ----------------------------*/

/*----------------------------- Module Functions ----------------------------*/
static void WagTail(void);
static void StopTail(void);

/*------------------------------ Module Code ------------------------------*/
void InitDMCHardware(void) {
		if ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R0) != SYSCTL_PRGPIO_R0) {
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
		while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R0) != SYSCTL_PRGPIO_R0);
	}
	//Set Pin 2 and 3 as digital pins on port A
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= (PAIR_OUTPUT | COLOR_OUTPUT);
	//Set pins 2 and 3 as outputs
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= (PAIR_OUTPUT | COLOR_OUTPUT);
	//Set pins 2 and 3 as low
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= (PAIR_OUTPUT_LO | COLOR_OUTPUT_LO);
	printf("DMC Hardware Init Successful\r\n");
}

void SetDMCPaired(void){
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= PAIR_OUTPUT_HI;
	StopTail();
}

void ClearDMCPaired(void){
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= PAIR_OUTPUT_LO;
	WagTail();
}

void SetDMCColor(uint8_t Color){
	if(Color == BLUE){
		HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= COLOR_OUTPUT_HI;
	}else if(Color == RED){
		HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= COLOR_OUTPUT_LO;
	}
}

void WagTail(void){
	printf("Ima wag ma tail\r\n");
}

void StopTail(void){
	printf("Ima not wag ma tail\r\n");
}
