#ifndef ToggleLEDService_H
#define ToggleLEDService_H

#include "ES_Configure.h"
#include "ES_Types.h"


// Public Function Prototypes
bool InitToggleLED( uint8_t Priority );
bool PostToggleLED( ES_Event ThisEvent );
ES_Event RunToggleLED( ES_Event ThisEvent );
void EOTResponse(void);

#endif /* LobbyistCommunication_H */
