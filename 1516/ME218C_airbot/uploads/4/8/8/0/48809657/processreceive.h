#ifndef ProcessReceive_H
#define ProcessReceive_H


#include "ES_Configure.h"
#include "ES_Types.h"

typedef enum {WAITING4PAIR, WAITING4ENCR, WAITING4CTL} RXProcessState_t;

// Public Function Prototypes
bool InitProcessReceiveService( uint8_t Priority );
bool PostProcessReceiveService( ES_Event ThisEvent );
ES_Event RunProcessReceiveService( ES_Event ThisEvent );

void CopyFrameData (uint8_t* ReceivedData, uint8_t PacketSize);

#endif /* ProcessReceive_H */
