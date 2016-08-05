//#define DEBUG 
/****************************************************************************
 Module
    ProcessDataModule.c

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

#define LOBBYIST_NUMBER 5
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void GenerateKey(void);
static void EncryptControlData(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t ReceivedFrameDataSize;
//static uint8_t SizeOfReceivedPacket;
//static uint8_t ReceivedPacket[MAX_PACKET_SIZE];
static uint8_t ReceivedFrameData[MAX_DATA_SIZE + FRAME_DATA_OVERHEAD];
static uint16_t DestinationAddress = 0x2081;
static uint8_t FrameID = 0x00;
static uint8_t CheckSum = 0;
static uint8_t EncryptedCheckSum = 0;
//static uint8_t ReceiveDataIndex = 0;
uint16_t SourceAddress;
uint8_t SignalStrength;
uint8_t ReceivedOptionsByte;
uint8_t ReceivedPacketType;
uint8_t CurrentEncryptionKey[ENCRYPTION_KEY_SIZE];
uint8_t KeyIndex = 0;
//static ReceivingState_t CurrentState;
static uint8_t Packet2Send[MAX_PACKET_SIZE];
static uint8_t Packet2SendSize = 0;
static uint8_t Data2Send[MAX_DATA_SIZE];
//static uint8_t Data2Send[] = {0x00,0x83};
static uint8_t SizeOfData = 0;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
    CopyReceivedPacket

 Parameters
     uint8_t *: PacketSource - pointer to the address of the array storing the 
       last packet sent

 Returns
    none

 Description
    Copies array of recently received packet into more permanent storage for 
       processing.
 Notes

 Authors
    Will Findlay
    Jordan A. Miller, 05/7/16, 16:37
****************************************************************************/
void CopyFrameData( uint8_t *PacketSource, uint8_t PacketSize )
{
    memcpy(ReceivedFrameData, PacketSource, PacketSize);
    ReceivedFrameDataSize = PacketSize;
}

void ProcessData(void) {

    uint8_t ByteIndex = 0;
    uint8_t API_ID = ReceivedFrameData[ByteIndex++];
    uint8_t ReceivedFrameID;
    
    switch (API_ID) {
        case XBEE_RESPONSE :
            ReceivedFrameID = ReceivedFrameData[ByteIndex++];
            uint8_t Status = ReceivedFrameData[ByteIndex++];
            if (Status != 0x00) {
                // Post packet ready event to Sending Service
                ES_Event Event2Post;
                Event2Post.EventType = PACKET_READY;
                //PostPACService(Event2Post);
            }
        break;
        
        case INCOMING_PACKET :
            SourceAddress = (ReceivedFrameData[ByteIndex++] << 8);
            SourceAddress += ReceivedFrameData[ByteIndex++];
            SignalStrength = ReceivedFrameData[ByteIndex++];
            ReceivedOptionsByte = ReceivedFrameData[ByteIndex++];
            
            if (ReceivedOptionsByte != 0x00) { // If received message was broadcast, ignore
                break;
            }
            
            ReceivedPacketType = ReceivedFrameData[ByteIndex++];
            ES_Event PACEvent;
            
            if (ReceivedPacketType == STATUS) {              
                uint8_t Status = ReceivedFrameData[ByteIndex++];  
                uint8_t AckCheckSum = ReceivedFrameData[ByteIndex++];
                //if (Status == 0x01) RA1 = 1;
                //else RA1 = 0;
                PACState_t CurrentPACState = QueryPACState();
                
                
                
                if (CurrentPACState == Unpaired) {

                    if ((Status & PAIR_MASK) == PAIRED) {
                        PACEvent.EventType = PAIRED_EVENT;
                        PostPACService(PACEvent);
                        if (RA2 == 1) {
                            RA2 = 0;
                        } else {
                            RA2 = 1;
                        }                          
                    } 
                }
                else if (CurrentPACState == Paired) {
                    
                    // If pair bit is clear, unpair
                    if ((Status & PAIR_MASK) == UNPAIRED) {
                        /*
                        if (RA1 == 1) {
                            RA1 = 0;
                        } else {
                            RA1 = 1;
                        } */  
                        //EncryptedCheckSum = 0;
                        PACEvent.EventType = UNPAIRED_EVENT;
                        PostPACService(PACEvent);
                    }
                    
                    // If Ack was for the wrong packet, ignore
                    else if (!GetEncryptionFlag() && (AckCheckSum != EncryptedCheckSum)) { 
                        if (RA1 == 1) {
                            RA1 = 0;
                        } else {
                            RA1 = 1;
                        }   
                        break;
                    }
                    
                    // If pair bit is set, make sure encryption is good
                    else if ((Status & PAIR_MASK) == PAIRED) {
                        if ((Status & ENCRYPTION_MASK) == ENCRYPTION_SUCCESS) {
                            PACEvent.EventType = PAIRED_EVENT;
                            PostPACService(PACEvent);
                            if (RA2 == 1) {
                                RA2 = 0;
                            } else {
                                RA2 = 1;
                            }                             
                        }
                        else if ((Status & ENCRYPTION_MASK) == ENCRYPTION_ERROR) {
                            PACEvent.EventType = UNPAIRED_EVENT;
                            PostPACService(PACEvent);
                        }
                    }
                    
                    /*
                    // If pair bit is clear, unpair
                    else if ((Status & PAIR_MASK) == UNPAIRED) {
                        
                        if (RA1 == 1) {
                            RA1 = 0;
                        } else {
                            RA1 = 1;
                        }  
                        PACEvent.EventType = UNPAIRED_EVENT;
                        PostPACService(PACEvent);
                    }
                    */
                }
            }
            
            // If we receive a packet that isn't a status packet from our LOBBYIST, unpair
            else if (ReceivedPacketType != STATUS) {   
                PACEvent.EventType = UNPAIRED_EVENT;
                //PostPACService(PACEvent);
            }
        break;
    }
}

/***************************************************************************
 public functions
 ***************************************************************************/

// Getter function to get packet to send
uint8_t *GetPacket2Send(void) {
    return Packet2Send;
}

// Getter function to get packet to send
uint8_t GetPacket2SendSize(void) {
    return Packet2SendSize;
}

/****************************************************************************
 Function
    ConstructPacket

 Parameters
     

 Returns
     

 Description
    Prepares the appropriate packet with previously prepared data. 
 Notes
 * This function must be called after a Construct_InsertHere_Data function has 
   already been called to prepare the data appropriate for the next packet to send

 Author
    William A. Findlay, 5/7/2016, 20:11
    Jordan A. Miller
 ***************************************************************************/
void ConstructPacket( void ) {
    // Increment FrameID
    FrameID++;
    // If frame ID is 0, set it to 1
    if (FrameID == 0) {
        FrameID++;
    }
    
    FrameID = 0;
    
    // Clear Packet2Send
    memset(Packet2Send,0,sizeof(Packet2Send));
    // Set CheckSum equal to 0
    CheckSum = 0;
    // Start Delimiter
    Packet2Send[0] = START_DELIMITER;
    // MSB of length
    Packet2Send[1] = MSB_LENGTH;
    // LSB of length
    Packet2Send[2] = SizeOfData + FRAME_DATA_OVERHEAD;
    // API Identifier
    Packet2Send[3] = API_ID_ToSend;
    // Add to CheckSum
    CheckSum += Packet2Send[3];
    // Frame ID
    Packet2Send[4] = FrameID;
    // Add to CheckSum
    CheckSum += Packet2Send[4];
    // Destination Address LSB
    Packet2Send[6] = DestinationAddress & 0xFF;
    // Add to CheckSum
    CheckSum += Packet2Send[6];
    // Destination Address MSB
    Packet2Send[5] = (DestinationAddress >> 8);
    // Add to CheckSum
    CheckSum += Packet2Send[5];
    // Options Byte
    Packet2Send[7] = SENDING_OPTIONS_BYTE;
    // Add to CheckSum
    CheckSum += Packet2Send[7];
    // Actual Data
    for (uint8_t index = 0; index < SizeOfData; index++) {
        Packet2Send[(XBEE_OVERHEAD-1) + index] = Data2Send[index];
        CheckSum += Packet2Send[(XBEE_OVERHEAD-1) + index];
    }
    // Calculate Checksum
    CheckSum = 0xFF - CheckSum;
    // Add CheckSum to Packet
    Packet2Send[(XBEE_OVERHEAD-1) + SizeOfData] = CheckSum;
    // Calculate size of Packet
    Packet2SendSize = XBEE_OVERHEAD + SizeOfData;
    // Post packet ready event to Sending Service
    ES_Event Event2Post;
    Event2Post.EventType = PACKET_READY;
    PostPACService(Event2Post);
}

/****************************************************************************
 Function
    ConstructEncryptionPacket

 Parameters
     

 Returns
     

 Description
    Prepares the appropriate bytes of data, which in this case should be an encryption key
 Notes

 Author
    William A. Findlay, 5/7/2016, 20:11
    Jordan A. Miller
 ***************************************************************************/
void ConstructEncryptionPacket(void) {
    memset(Data2Send,0,sizeof(Data2Send));
    
    GenerateKey();
    DestinationAddress = SourceAddress;
    Data2Send[0] = ENCR_KEY;
    memcpy(&Data2Send[1],CurrentEncryptionKey,ENCRYPTION_KEY_SIZE);
    
    SizeOfData = ENCR_DATA_SIZE;
    ConstructPacket();
}


/****************************************************************************
 Function
    ConstructPairRequestData

 Parameters
     

 Returns
     

 Description
    Prepares the appropriate packet
 Notes

 Author
    William A. Findlay, 5/7/2016, 20:11
    Jordan A. Miller
 ***************************************************************************/
void ConstructPairRequestPacket(PACColor_t Color, uint8_t LobbyistNumber) {
    memset(Data2Send,0,sizeof(Data2Send));
    DestinationAddress = BROADCAST;
    Data2Send[0] = REQ_PAIR;
    if (Color == Red) {
        Data2Send[1] &= RED_MASK;
    } else {
        Data2Send[1] |= BLUE_MASK;
    }
    
#ifdef DEBUG
    LobbyistNumber = LOBBYIST_NUMBER;
#endif
    
    Data2Send[1] |= LobbyistNumber;
    SizeOfData = REQ_PACKET_DATA_SIZE;
    ConstructPacket();
}


/****************************************************************************
 Function
    ConstructControlPacket

 Parameters
     

 Returns
     

 Description
    Prepares the appropriate packet
 Notes

 Author
    William A. Findlay, 5/7/2016, 20:11
    Jordan A. Miller
 ***************************************************************************/
void ConstructControlPacket(int8_t AccelInput, int8_t SteerInput, bool SpecAction, bool Unpair, bool EBrake) {
    memset(Data2Send,0,sizeof(Data2Send));
    // THIS SHOULD BE CHANGED LATER
    DestinationAddress = SourceAddress;
    Data2Send[0] = CTRL;
    Data2Send[1] = (uint8_t)AccelInput;
    Data2Send[2] = (uint8_t)SteerInput;
    if (SpecAction) {
        Data2Send[3] |= BIT2HI;
    }
    if (Unpair) {
        Data2Send[3] |= BIT1HI;
    }
    if (EBrake) {
        Data2Send[3] |= BIT0HI;
    }
    Data2Send[4] = Data2Send[0] + Data2Send[1] + Data2Send[2] + Data2Send[3];
    SizeOfData = CTRL_DATA_SIZE;
    
    EncryptControlData();
    ConstructPacket();
}

/***************************************************************************
 private functions
 ***************************************************************************/
static void GenerateKey(void) {
    srand(ES_Timer_GetTime());
    for (int i=0;i<ENCRYPTION_KEY_SIZE;i++) {
        CurrentEncryptionKey[i] = (uint8_t)(rand() & 0xFF);
    }
    
    // Reset the key index every time we create and send a new key
    KeyIndex = 0;
}

static void EncryptControlData(void) {
    for (int i=0;i<CTRL_DATA_SIZE;i++) {
        Data2Send[i] = Data2Send[i] ^ CurrentEncryptionKey[(KeyIndex++ & MODULO_32)];
        //if (KeyIndex == 32) {
        //    KeyIndex = 0; // increment modulo 32
        //}
    }
    EncryptedCheckSum = Data2Send[CTRL_DATA_SIZE-1]; // Save away the encrypted check sum for Ack Checking
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/



