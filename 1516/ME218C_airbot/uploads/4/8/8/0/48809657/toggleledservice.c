/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "uartstdio.h"
#include "LobbyistCommunication.h"

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_Timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ssi.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "ToggleLEDService.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define ALL_BITS (0xff<<2)

#define LED_PIN GPIO_PIN_5
#define	LED_PIN_HI BIT5HI
#define LED_PIN_LO BIT5LO

#define DECREMENT_TIME 11
/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static uint16_t ToggleTime;

/*---------------------------- Module Functions ---------------------------*/
static void InitLED(void);
/*------------------------------ Module Code ------------------------------*/

bool InitToggleLED(uint8_t Priority) {
		MyPriority = Priority;
		InitLED();
		ToggleTime = ONE_SEC;
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_INIT;
		if (ES_PostToService (MyPriority, ThisEvent) == true) {
			printf("LED initialized sucessfully\r\n");
			return true;
		} else {
			return false;
		}
}

static void InitLED(void) {
		if ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R0) != SYSCTL_PRGPIO_R0) {
		HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R0;
		while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R0) != SYSCTL_PRGPIO_R0);
	}
	//Set Pin 5 as digital pins on port A
	HWREG(GPIO_PORTA_BASE+GPIO_O_DEN) |= LED_PIN;
	//Set pins 5 as outputs
	HWREG(GPIO_PORTA_BASE+GPIO_O_DIR) |= LED_PIN;
	//Set pins 5 as low
	HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= LED_PIN_LO;
}

bool PostToggleLED( ES_Event ThisEvent ){
	  //Post Event to queue
	return ES_PostToService( MyPriority, ThisEvent);
}

ES_Event RunToggleLED( ES_Event ThisEvent) {
	if (ThisEvent.EventType == PAIR_EVENT) {
		ToggleTime = ONE_SEC;
		ES_Timer_InitTimer(TOGGLE_TIMER, ToggleTime);
		printf("Pair Event \r\n");
	}
	if (ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam== TOGGLE_TIMER) {
		//printf("Toggle timer timed out\r\n");
		HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) ^= LED_PIN_HI;
		ToggleTime = ToggleTime - DECREMENT_TIME;
		ES_Timer_InitTimer(TOGGLE_TIMER, ToggleTime);
	} 
	if (ThisEvent.EventType == UNPAIR_EVENT) {
		HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= LED_PIN_LO;
		ES_Timer_StopTimer(TOGGLE_TIMER);
		printf("Unpair event \r\n");
	}
	ES_Event ReturnEvent;
	ReturnEvent.EventType = ES_NO_EVENT;
	return ReturnEvent;
}

