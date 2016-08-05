
 /****************************************************************************
 Module
    AccelerometerModule.c

 Revision
   1.0.1

 Description
   This is a template file for implementing a simple service under the 
   Gen2 Events and Services Framework.

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 01/16/12 09:58 jec      began conversion from TemplateFSM.c
****************************************************************************/
/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "ReceivingService.h"
#include "ProcessDataModule.h"
#include "PACService.h" 
#include <string.h>
#include <stdlib.h>

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 100
#define HALF_SEC ONE_SEC/2
#define TENTH_SEC ONE_SEC/10
#define THREE_SEC ONE_SEC*3

#define SPBRGL_CONSTANT 0x33

#define NON_FRAME_DATA_OVERHEAD 4
#define FRAME_DATA_OVERHEAD 5
#define XBEE_OVERHEAD (NON_FRAME_DATA_OVERHEAD + FRAME_DATA_OVERHEAD)
#define MAX_DATA_SIZE 33
#define MAX_PACKET_SIZE (NON_FRAME_DATA_OVERHEAD + FRAME_DATA_OVERHEAD + MAX_DATA_SIZE)
#define ENCRYPTION_KEY_SIZE 32

#define API_ID_ToSend 0x01
#define START_DELIMITER 0x7E
#define MSB_LENGTH 0x00

#define XBEE_NACK 0x01
#define CCA_FAIL 0x02

#define PAIR_MASK 0x01
#define ENCRYPTION_MASK 0x02

#define PAIRED 0x01
#define UNPAIRED 0x00

#define ENCRYPTION_ERROR 0x02
#define ENCRYPTION_SUCCESS 0x00

#define SENDING_OPTIONS_BYTE 0x00

#define BROADCAST 0xFFFF

#define REQ_PAIR 0x00
#define ENCR_KEY 0x01
#define CTRL 0x02

#define REQ_PACKET_DATA_SIZE 2
#define ENCR_DATA_SIZE 33
#define CTRL_DATA_SIZE 5

#define RED_MASK 0x7F
#define BLUE_MASK 0x80

#define MODULO_32 0x1F



/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/


/*---------------------------- Module Variables ---------------------------*/

/*------------------------------ Module Code ------------------------------*/
/***************************************************************************
 public functions
 ***************************************************************************/

void InitAccelerometer( void )
{
    // Set to look for falling edges
    IOCCN = 0b00000011;
    
    // Enable IOC interrupt
    IOCIE = 1;
        
}

bool Accel_IOC_ISR(void) {

    // Turn off IOC interrupt until later
    //IOCIE = 0;
    //IOCCN = 0b00000000;
    
    if (RA0)
        RA0 = 0;
    else
        RA0 = 1;
    // Clear the flag for both input pins
    IOCCF = 0b00000000;
    
    return true;    
}

void CheckForPair(void) {
    // Enable IOC for pairing
    IOCIE = 1;
    IOCCN = 0b00000011;
}

void IgnorePair(void) {
    IOCIE = 0;
    IOCCN = 0b00000000;
}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/



