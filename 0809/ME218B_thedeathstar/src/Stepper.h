#ifndef STEPPER_H
#define STEPPER_H 

/*
 * These are all the public functions necessary to move
 * the stepper motors/claws 
 */

// Claw states
#define DR_OPEN 0
#define DR_CLSD 1

// Claw names
#define DR_F 0
#define DR_B 1

// GetDoor dummy declaration for compilation
char getDoor(char sensor);

void CloseDoor();
void OpenDoor();
