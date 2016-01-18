/***********************************************************************

Module
    motors.h

Description
    Header file for motors
    
Notes
    Requires Board Connections as noted in the lab writeup
    
***********************************************************************/

#ifndef MOTORS_H
#define MOTORS_H

void InitMotors            ( void);
void SimpleMove         (float LeftDuty, float RightDuty);
void ControlLawMove     (char LeftRPMTarget, char RightRPMTarget);
void SetPWMOffset(unsigned char Offset);

#endif

/*-------------------------- End of file -----------------------------*/