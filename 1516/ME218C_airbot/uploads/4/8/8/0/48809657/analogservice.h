/* 
 * File:   AnalogService.h
 * Author: jordanm1
 *
 * Created on May 9, 2016, 8:02 PM
 */

#ifndef ANALOGSERVICE_H
#define	ANALOGSERVICE_H

typedef enum {Initializing, Sampling, Waiting} AnalogState_t;

int8_t GetDriveCommand(void);
int8_t GetSteeringCommand(void);
uint8_t GetLobbyistNumber(void);

bool InitAnalogService ( uint8_t Priority );
bool PostAnalogService( ES_Event ThisEvent );
ES_Event RunAnalogService( ES_Event ThisEvent );

void AnalogISR(void);

#endif	/* ANALOGSERVICE_H */

