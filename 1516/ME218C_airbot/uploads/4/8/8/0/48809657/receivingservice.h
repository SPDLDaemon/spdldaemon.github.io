/* 
 * File:   ReceivingService.h
 * Author: wfindlay
 *
 * Created on May 7, 2016, 8:42 AM
 */

#ifndef RECEIVINGSERVICE_H
#define	RECEIVINGSERVICE_H

#include "ES_Types.h"

// typedefs for the state
typedef enum { WaitingForStart, WaitingForMSB, WaitingForLSB, ReceivingFrameData, WaitingForCheckSum } ReceivingState_t ;

// Public Function Prototypes

bool InitReceivingService ( uint8_t Priority );
bool PostReceivingService( ES_Event ThisEvent );
ES_Event RunReceivingService( ES_Event ThisEvent );
bool RCIF_ISR(void);
uint8_t *GetReceivedPacket(void);
uint8_t GetReceivedPacketSize(void);
uint8_t *GetFrameDataPacket(void);
uint8_t GetFrameDataPacketSize(void);


#endif	/* RECEIVINGSERVICE_H */

