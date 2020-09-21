#ifndef CLAW_H
#define CLAW_H 

/*
 * These are all the public functions necessary to move
 * the stepper motors/claws 
 */

/* equals to 80ms with 16ms initialization */
#define step_delay 2000
#define DirectionBit BIT4HI
#define FULL 1
#define WAVE 2
#define HALF 3
#define MICRO 4
#define FORWARD 0
#define BACKWARD 1

/* claw states */
#define DR_OPEN 0
#define DR_CLSD 1
#define DR_CLSING 2
#define DR_OPENING 3

/* claw parameters */
#define DR_FRONT 0
#define DR_BACK 1 
#define STEPS_TO_CLOSE 30
#define STEPS_TO_OPEN 30


/* public functions used in the state machine */
void ClawInit(void);
char getDoor(char door);
void commandDoor(char door, char action);
void CloseClaw1(void);
void OpenClaw1(void);
void CloseClaw2(void);
void OpenClaw2(void);


/* used internally */

void ResetToOpen1(void);
void ResetToClosed1(void);
void IncrementDriveStep1(void);
void DecrementDriveStep1(void);


void ResetToOpen2(void);
void ResetToClosed2(void);
void IncrementDriveStep2(void);
void DecrementDriveStep2(void);


void FirstStepHandle1(void);
void StepOutput1(void);
void OutputToPTP_W_inverter1(int arr[6]);

void FirstStepHandle2(void);
void StepOutput2(void);
void OutputToPTP_W_inverter2(int arr[6]);

#endif