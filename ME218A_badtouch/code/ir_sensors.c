/* ir_sensors.c
 * ============
 */

#include <stdio.h>
#include "our_c32.h"
#include "ir_sensors.h"

//#define DEBUG

//Port/pin constants
#define LED_PORT  T
#define LED_PIN   0

//Threshold constants
#define COVERED_THRESHOLD 360

//Prototypes
static void ToggleLeds(char state);

//Implementation

/* IrSensorsInit
 * =============
 * See header file for usage.
 */
void IrSensorsInit(void) {
  char i;

  WritePort(LED_PORT, LED_PIN, ON);
  SetDDR(LED_PORT, LED_PIN, WRITE);
  
  for(i = 0; i <= 7; i++) {
    SetDDR(AD, i, ANALOG);
  }
}

/* ToggleLeds
 * ==========
 * Changes the state of the LEDs. Can specify either on,
 * off, or toggle.
 *
 * Args:
 *   state: A constant specifying what state to make the
 *          LEDs. Takes ON, OFF, or TOGGLE.
 */
static void ToggleLeds(char state) {
  char currState;
  char newState;
  
  switch(state) {
    case ON:
      newState = ON;
      break;
    case OFF:
      newState = OFF;
      break;
    case TOGGLE:
      currState = (char)ReadPort(LED_PORT, LED_PIN);
      newState = !(currState && ON);
      break;
  }

  WritePort(LED_PORT, LED_PIN, newState);
}

/* GetSensorValue
 * ==============
 * See header file for usage..
 */
char GetSensorValue(char sensor) {
  short reading;

  reading = ReadPort(AD, sensor);
  if(reading < COVERED_THRESHOLD) {
    return COVERED;
  }
  return UNCOVERED;
}

//Test harness
#ifdef DEBUG

void main(void) {
  char dummy;
  int i;

  IrSensorsInit();

  /*
  //Test light toggling
  ToggleLeds(ON);
  printf("ON\r\n");
  dummy = getchar();
  ToggleLeds(OFF);
  printf("OFF\r\n");
  dummy = getchar();
  ToggleLeds(TOGGLE);
  printf("TOGGLE (on)\r\n");
  dummy = getchar();
  ToggleLeds(TOGGLE);
  printf("TOGGLE (off)\r\n");
  dummy = getchar();
  */

  
  //Test sensor reading
  printf("Left Hand, left: %d\r\n", GetSensorValue(LL_SENSOR));
  dummy = getchar();
  printf("Left Hand, back: %d\r\n", GetSensorValue(LB_SENSOR));
  dummy = getchar();
  printf("Left Hand, right: %d\r\n", GetSensorValue(LR_SENSOR));
  dummy = getchar();
  printf("Left Hand, front: %d\r\n", GetSensorValue(LF_SENSOR));
  dummy = getchar();
  printf("Right Hand, front: %d\r\n", GetSensorValue(RF_SENSOR));
  dummy = getchar();
  printf("Right Hand, left: %d\r\n", GetSensorValue(RL_SENSOR));
  dummy = getchar();
  printf("Right Hand, back: %d\r\n", GetSensorValue(RB_SENSOR));
  dummy = getchar();
  printf("Right Hand, right: %d\r\n", GetSensorValue(RR_SENSOR));
  dummy = getchar();
  
  
  /*
  //Determine pins
  for(i = 0; i <= 7; i++) {
    while(1) {	 
      printf("%d: %d\r\n", i, GetSensorValue((char)i));
      dummy = getchar();
      if(dummy == 'q')
        break;
    }
  }
  */
  
  /*
  //Calibrate light
  while(1) {
    printf("LED off: %d\r\n", ReadPort(AD, LL_SENSOR));
    dummy = getchar();
    ToggleLeds(ON);
    for(i = 0; i < 10000; i++) {
      dummy = i;
    }
    
    printf("LED on: %d\r\n", ReadPort(AD, LL_SENSOR));
    dummy = getchar();
    ToggleLeds(OFF);
    for(i = 0; i < 10000; i++) {
      dummy = i;
    }
  }
  */
}

#endif