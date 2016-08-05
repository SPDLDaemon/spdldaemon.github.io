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
#include "LiftFanModule.h"
#include "PWMModule.h"

#define ALL_BITS (0xff<<2) 


#define LIFT_PIN GPIO_PIN_4
#define LIFT_PIN_HI	BIT4HI
#define	LIFT_PIN_LO BIT4LO

/*----------------------------- Module Code ----------------------------*/
void InitLiftFan(void) {
		if ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R1) != SYSCTL_PRGPIO_R1) {
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
		while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R1) != SYSCTL_PRGPIO_R1);
	}
	//Set Pin 4 as digital pin on port B
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= LIFT_PIN;
	//Set lift fan line as output
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= LIFT_PIN;
	//Set lift fan line as low
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= LIFT_PIN_LO;
}

void StopLiftFan(void) {
	//HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= LIFT_PIN_LO;
	SetDutyLift(1);
}

void StartLiftFan(void) {
	//HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) |= LIFT_PIN_HI;
	SetDutyLift(40);
}
