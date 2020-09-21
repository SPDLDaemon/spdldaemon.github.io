#ifndef COACHEVENTS_H
#define COACHEVENTS_H

// Initialize the COACH ports etc.
void COACH_INIT(void);

// Find out if there has been a jettison crew action
char CrewJettison(void);

// Find out if the mutiny has been suppressed
char CheckMutinySuppression(void);

// Check for an assertion of command action
char CheckAssertionCommand(void);

// Find out if the free digital command has been triggered
char FreeDigitalActivate(void);

// Find out if the free analog command has been triggered
char FreeAnalogActivate(void);

// Find the upper nibble of the velocity command
char GetTranslationalVelocity(void);

// Get Free Analog Parameter Function
char GetFreeAnalogParameter(void);

#endif