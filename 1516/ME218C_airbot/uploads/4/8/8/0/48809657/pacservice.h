/* 
 * File:   PACService.h
 * Author: wfindlay
 *
 * Created on May 8, 2016, 8:35 PM
 */

#ifndef PACSERVICE_H
#define	PACSERVICE_H

#include "ES_Types.h"


// typedefs for the state
typedef enum { Paired, Unpaired } PACState_t ;
typedef enum {Red, Blue} PACColor_t;

// Public Function Prototypes

bool InitPACService ( uint8_t Priority );
bool PostPACService( ES_Event ThisEvent );
ES_Event RunPACService( ES_Event ThisEvent );
PACState_t QueryPACState( void );
void TXIF_ISR(void);
bool GetEncryptionFlag( void );

#endif	/* PACSERVICE_H */

