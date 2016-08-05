#ifndef PropModule_H
#define PropModule_H

#include "ES_Configure.h"
#include "ES_Types.h"

void ProcessPositions(float X, float Y);
void GoForward(uint8_t DutyCycle);
void GoBackward(uint8_t DutyCycle);
void StopProps(void);
void TurnRight(uint8_t DutyLeft, uint8_t DutyRight, uint8_t DirLeft, uint8_t DirRight);
void TurnLeft(uint8_t DutyLeft, uint8_t DutyRight, uint8_t DirLeft, uint8_t DirRight);

#endif /* PropModule_H */
