

/****************************************************************************
 Module
   SendingService.c

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
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 100
#define HALF_SEC ONE_SEC/2
#define TENTH_SEC ONE_SEC/10
#define THREE_SEC ONE_SEC*3
#define TEN_MILLI 1
#define SPBRGL_CONSTANT 0x33
#define NON_FRAME_DATA_OVERHEAD 4
#define FRAME_DATA_OVERHEAD 4
#define MAX_DATA_SIZE 33
#define MAX_PACKET_SIZE (NON_FRAME_DATA_OVERHEAD + FRAME_DATA_OVERHEAD + MAX_DATA_SIZE)

/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitReceivingEUSART(void);
static void ResetPackets(void);

/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
//static uint8_t Data2Send[16]; 

static uint8_t SizeOfFrameData;
static uint8_t SizeOfReceivedPacket;
static uint8_t ReceivedPacket[MAX_PACKET_SIZE];
static uint8_t FrameDataPacket[MAX_DATA_SIZE + FRAME_DATA_OVERHEAD];
//static uint8_t API_Identifier;
//static uint16_t DestinationAddress;
//static uint8_t FrameID;
static uint8_t CheckSum = 0;
static uint8_t ReceiveDataIndex = 0;
static ReceivingState_t CurrentState;


/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitReceivingService

 Parameters
     uint8_t : the priority of this service

 Returns
     bool, false if error in initialization, true otherwise

 Description
     Saves away the priority, and does any 
     other required initialization for this service
 Notes

 Author
     J. Edward Carryer, 01/16/12, 10:00
****************************************************************************/
bool InitReceivingService ( uint8_t Priority )
{
  ES_Event ThisEvent;

  MyPriority = Priority;
  /********************************************
   in here you write your initialization code
   *******************************************/
  // Initialize the EUSART to receive
  InitReceivingEUSART();
  // Initialize the current state to WaitingForStart
  CurrentState = WaitingForStart;
  // post the initial transition event
  ThisEvent.EventType = ES_INIT;
  if (ES_PostToService( MyPriority, ThisEvent) == true)
  {
      return true;
  }else
  {
      return false;
  }
}

static void InitReceivingEUSART(void) {
    // Set desired baud rate
    BRGH = 0;
    BRG16 = 0;
    SPBRGH = 0;
    SPBRGL = SPBRGL_CONSTANT;
    
    // RECEIVING EUSART
    // Set pin as input
    //TRISB7 = 1;
    
    // Set RX pin to RB7
    RXSEL = 1;
    // Enable SPEN bit
    SPEN = 1;
    // Enable receiving interrupts
    RCIE = 1;
    PEIE = 1;
    // 8 bit receiving
    RX9 = 0;
    // Enable reception by setting CREN bit
    CREN = 1;
}

/****************************************************************************
 Function
     PostReceivingService

 Parameters
     EF_Event ThisEvent ,the event to post to the queue

 Returns
     bool false if the Enqueue operation failed, true otherwise

 Description
     Posts an event to this state machine's queue
 Notes

 Author
     J. Edward Carryer, 10/23/11, 19:25
****************************************************************************/
bool PostReceivingService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunReceivingService

 Parameters
   ES_Event : the event to process

 Returns
   ES_Event, ES_NO_EVENT if no error ES_ERROR otherwise

 Description
   add your description here
 Notes
   
 Author
   J. Edward Carryer, 01/15/12, 15:23
****************************************************************************/
ES_Event RunReceivingService( ES_Event ThisEvent )
{
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors

    if (ThisEvent.EventType == ES_TIMEOUT) {
        if (ThisEvent.EventParam == RECEIVE_TIMEOUT_TIMER) {
            CurrentState = WaitingForStart;

        }
    }
    return ReturnEvent;
}

/***************************************************************************
 private functions
 ***************************************************************************/
static void ResetPackets(void) {
    // Clear ReceivedPacket
    memset(ReceivedPacket,0,sizeof(ReceivedPacket));
    // Clear FrameDataPacket
    memset(FrameDataPacket,0,sizeof(FrameDataPacket));
    // Clear CheckSum
    CheckSum = 0;
    // Reset ReceiveDataIndex
    ReceiveDataIndex = 0;
    // Reset SizeOfFrameData and SizeOfReceivedPacket
    SizeOfFrameData = 0;
    SizeOfReceivedPacket = 0;
}
/***************************************************************************
 public functions
 ***************************************************************************/

// Getter function to get entire received packet
uint8_t *GetReceivedPacket(void) {
    return ReceivedPacket;
}

// Getter function to get size of entire received packet
uint8_t GetReceivedPacketSize(void) {
    return SizeOfReceivedPacket;
}

// Getter function to get just the frame data
uint8_t *GetFrameDataPacket(void) {
    return FrameDataPacket;
}

// Getter function to get the size of the frame data
uint8_t GetFrameDataPacketSize(void) {
    return SizeOfFrameData;
}

// Function called by the ISR when a byte is received
bool RCIF_ISR() {
    bool ReturnVal = false;
    // Store the newly received byte in a function variable
    uint8_t ByteReceived = RCREG;
    // Put the byte into the received packet array at the next available index
    ReceivedPacket[ReceiveDataIndex] = ByteReceived;
    // Create a ReceivingState_t NextState
    ReceivingState_t NextState;
    // Set next state equal to the current state
    NextState = CurrentState;
    // Switch statement for the state machine
    switch (CurrentState) {
        // WaitingForStart state
        case WaitingForStart:
            ResetPackets();
            if (ByteReceived == 0x7E) {
                ReceivedPacket[ReceiveDataIndex] = ByteReceived;
                ReceiveDataIndex++;
                NextState = WaitingForMSB;
                ES_Timer_InitTimer(RECEIVE_TIMEOUT_TIMER,TENTH_SEC);
            } else {
                ResetPackets();
            }
            break;
        
        // WaitingForMSB state (MSB of length of data)
        case WaitingForMSB:
            
            if (ByteReceived == 0x00) {
                ReceiveDataIndex++;
                NextState = WaitingForLSB;
                ES_Timer_InitTimer(RECEIVE_TIMEOUT_TIMER,TENTH_SEC);
            } else {
                NextState = WaitingForStart;
            }
            break;
            
        // WaitingForLSB state (LSB of length of data)
        case WaitingForLSB:
            SizeOfFrameData = ByteReceived;
            SizeOfReceivedPacket = SizeOfFrameData + NON_FRAME_DATA_OVERHEAD;
            ReceiveDataIndex++;
            CheckSum = 0;
            NextState = ReceivingFrameData;
            
            ES_Timer_InitTimer(RECEIVE_TIMEOUT_TIMER,TENTH_SEC);
            
            break;

        // ReceivingFrameData state 
        case ReceivingFrameData:
            
            if (ReceiveDataIndex < SizeOfReceivedPacket - 1) {
                CheckSum += ByteReceived;
                FrameDataPacket[ReceiveDataIndex - (NON_FRAME_DATA_OVERHEAD - 1)] = ByteReceived;
            }
            ReceiveDataIndex++;
            if (ReceiveDataIndex == (SizeOfReceivedPacket - 1)) {
                NextState = WaitingForCheckSum;
            }
            ES_Timer_InitTimer(RECEIVE_TIMEOUT_TIMER,TENTH_SEC);
            break;

        // WaitingForCheckSum state
        case WaitingForCheckSum:
            ES_Timer_StopTimer(RECEIVE_TIMEOUT_TIMER);
            
            if ((0xFF - CheckSum) == ByteReceived) {
                CopyFrameData(FrameDataPacket,SizeOfFrameData);
                ReturnVal = true;
            } else {
                ResetPackets();
            }
            NextState = WaitingForStart;
            break;
            
    // End switch statement                
    }
    
    // Set the current state equal to the next state 
    CurrentState = NextState;
    
    // Just in case I messed up somewhere, set the index to 0 if trying to 
    // access outside the preallocated memory of array
    if (ReceiveDataIndex == sizeof(ReceivedPacket)) {
        ReceiveDataIndex = 0;
        ResetPackets();
    }
    
    return ReturnVal;
}


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/


