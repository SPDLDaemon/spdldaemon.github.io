/***********************************************************************

Module
    beacons.h

Description
    Header file for beacon detection
    
Notes
    Requires Board Connections as noted in the lab writeup
    
***********************************************************************/

#ifndef BEACONS_H
#define BEACONS_H

#define LEFT_TOP_BEACON   4
#define RIGHT_TOP_BEACON  5
#define LEFT_FLAG_BEACON  6
#define RIGHT_FLAG_BEACON 7

#define LEFT_FLAG_LEVEL   3
#define RIGHT_FLAG_LEVEL  4

#define RIGHT		  7
#define LEFT              6

typedef struct {
  char side;
  unsigned char own;
  unsigned char opposing;
}CornerBeacons_t;

void InitBeacons(void);
void InitSide(void);
short GetBeaconLevel(char BeaconPin);
CornerBeacons_t GetSide(void);
char DetectFlash          ( void);
char BeaconDuty           ( char);
unsigned char Orientation(void);
void BeaconControlMove(void);
char IsGateBlocked(void);

#endif

/*-------------------------- End of file -----------------------------*/