/****************************************************************************
 Copyright (C) Creative Applications Engineering 2004.
 Permission granted to freely use this file as long as this header remains
 intact.

 Description
         ads12.h is the header file for the embedded a/d converter library.
         This header should be included by all applications programs wishing
         to call library routines.
 Notes
         All A/D channels must be in a single contiguios group of pins.

 History
 When           Who	What/Why
 -------------- ---	--------
 06/22/04 20:07 jec  First pass
****************************************************************************/

#ifndef ADS12_H
#define ADS12_H

typedef enum  {  ADS12_Err,
                 ADS12_OK
} ADS12ReturnTyp;

/*
   function prototypes for the library
*/
ADS12ReturnTyp ADS12_Init(char * modeString);
short ADS12_ReadADPin( unsigned char pinNumber);

#endif /* ADS12_H */
