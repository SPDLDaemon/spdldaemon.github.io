#ifndef LobbysistCommunication_H
#define LobbyistCommunication_H

#include "ES_Configure.h"
#include "ES_Types.h"

//define possible states
typedef enum {WAITING47E,WAITING4MSB,WAITING4LSB,SUCKUPPACKET} LobbyistComState_t;

// Public Function Prototypes
bool InitLobbyistComService( uint8_t Priority );
bool PostLobbyistComService( ES_Event ThisEvent );
ES_Event RunLobbyistComService( ES_Event ThisEvent );
void EOTResponse(void);

void PrepareDataPacket(uint8_t MSB, uint8_t LSB);
void SendFirstByte(void);
void SetDataByte(uint8_t* SendData, uint8_t PacketSize);

#endif /* LobbyistCommunication_H */
