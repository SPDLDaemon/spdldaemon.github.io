#ifndef PWMModule_H
#define PWMModule_H

// Event Definitions
#include "ES_Configure.h" /* gets us event definitions */
#include "ES_Types.h"     /* gets bool type for returns */
#include "ES_Events.h"

#define FORWARD 1
#define REVERSE 0

// Public Function Prototypes

void InitPWMModule(void);
void SetDutyLeft(uint8_t DutyCycleLeft, uint8_t DirectionLeft);
void SetDutyRight(uint8_t DutyCycleRight, uint8_t DirectionRight);
void SetDutyLift(uint8_t DutyCycleLift);

#endif /* PWMModule_H */
