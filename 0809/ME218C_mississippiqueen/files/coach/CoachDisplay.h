#ifndef COACHDISPLAY_H
#define COACHDISPLAY_H

#define Red   0
#define Blue  1

// Alerts the user that a mutiny is in progress
void MutinyAlertLED(void);
void MutinyAlertLEDOff(void);

// Plays Mississippi Queen
void PlayMississippiQueen(void);

// Alerts the user that they are not paired
void NotPairedAlert(void);
void NotPairedAlertOff(void);

// Finds out what team the user is on (red or blue)
unsigned char GetTeamColor(void);


#endif