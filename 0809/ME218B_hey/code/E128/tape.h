/***********************************************************************

Module
    tape.h

Description
    Header file for tape detection
    
Notes
    Requires Board Connections as noted in the lab writeup
    
***********************************************************************/

#ifndef TAPE_H
#define TAPE_H

#define FRONT_LEFT       0
#define FRONT_RIGHT      2
#define REAR             1

#define WHITE 0
#define GREEN 1
#define BLACK 2
#define GREEN_TAPE 3

int GetTapeValue(char sensor);
char GetTapeColor(char sensor);

#endif

/*-------------------------- End of file -----------------------------*/