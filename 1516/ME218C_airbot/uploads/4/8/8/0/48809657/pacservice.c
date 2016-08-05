
/****************************************************************************
 Module
   PACService.c

 Revision
   1.0.1

 Description
   

 Notes

 History
 When           Who     What/Why
 -------------- ---     --------
 05/08/16 09:58 waf      began creating PACService from SendingService template
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
#include "AnalogService.h"
#include "AccelerometerModule.h"
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 100
#define HALF_SEC ONE_SEC/2
#define TENTH_SEC ONE_SEC/10
#define THREE_SEC ONE_SEC*3
#define FIFTH_SEC ONE_SEC/5
#define SPBRGL_CONSTANT 0x33

#define SENDING_TIME 20
#define RESEND_TIME 10
#define INACTIVE_TIME 100
#define XBEE_OVERHEAD 9
#define MAX_DATA_SIZE 33
#define MAX_PACKET_SIZE (XBEE_OVERHEAD+MAX_DATA_SIZE)

#define BRAKE RC5    // Reverse because active low
#define UNPAIR RC6   // Reverse becauase active low
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitSendingEUSART(void);
static void InitColorSwitchInput(void);
static void ResetSendTimers(void);
static void SendPacketAndResetFlags(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
static PACState_t CurrentState;
static PACColor_t LobbyistColor = Blue;
static uint8_t LobbyistNumber = 5;
static uint8_t *Packet2Send;
static uint8_t Packet2SendSize;
static uint8_t SendPacketIndex = 0;
static bool SendingTimeoutFlag = false;
static bool PacketReadyFlag = false;
static bool EncryptionPacketFlag = false;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitPACService

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
bool InitPACService ( uint8_t Priority )
{
    ES_Event ThisEvent;

    MyPriority = Priority;
    /********************************************
     in here you write your initialization code
     *******************************************/
    CurrentState = Unpaired;
    InitSendingEUSART();
    InitAccelerometer();
    //InitColorSwitchInput();
    ES_Timer_InitTimer(SENDING_TIMER, SENDING_TIME);
    
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

static void InitColorSwitchInput(void) {
    // Set RC7 to input
    TRISC7 = 1;
    // Set RC7 to digital
    ANSC7 = 0;
    
}

static void InitSendingEUSART(void) {
    // Set desired baud rate
    BRGH = 0;
    BRG16 = 0;
    SPBRGH = 0;
    SPBRGL = SPBRGL_CONSTANT;
    
    // SENDING EUSART
    // Enable 8-bit mode
    TX9 = 0;
    // Enable asynchronous serial port by clearing SYNC bit and setting SPEN bit
    SYNC = 0;
    SPEN = 1;
    // Enable transmission by setting TXEN bit
    TXEN = 1;
    // Set TX pin to RB6
    TXSEL = 1;
    // Make sure interrupt is disabled
    TXIE = 0;   

}

/****************************************************************************
 Function
     PostPACService

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
bool PostPACService( ES_Event ThisEvent )
{
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunPACService

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
ES_Event RunPACService( ES_Event ThisEvent )
{
    PACState_t NextState = CurrentState;
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    /********************************************
     in here you write your service code
     *******************************************/
    switch (CurrentState) {
        case Unpaired:
        
            if (ThisEvent.EventType == PAIR_TRIGGER) {
                // Request a pair on button trigger (will be from Accel Input Capture)
                if (RC7 = 0) {
                    LobbyistColor = Red;
                } else {
                    LobbyistColor = Blue;
                }
                ConstructPairRequestPacket(LobbyistColor, GetLobbyistNumber());  
                
                if (RA0)
                    RA0 = 0;
                else
                    RA0 = 1;                
            }

            else if (ThisEvent.EventType == PAIRED_EVENT) {
                // If unpaired and we get a paired event, send encryption key
                EncryptionPacketFlag = true;
                ConstructEncryptionPacket();
                ES_Timer_InitTimer(ACTIVE_PAIR_TIMER,INACTIVE_TIME);
                NextState = Paired;
                IgnorePair();
            }    
            
            else if (ThisEvent.EventType == UNPAIRED_EVENT) {                
                CheckForPair();
            }
            
            else if (ThisEvent.EventType == ES_TIMEOUT) {
                if(ThisEvent.EventParam == SENDING_TIMER) {
                    SendingTimeoutFlag = true;
                    if (PacketReadyFlag) {
                        SendPacketAndResetFlags();                        
                    }
                }
                
                else if (ThisEvent.EventParam == ACTIVE_PAIR_TIMER) {
                    ES_Event Event2Post;
                    Event2Post.EventType = UNPAIRED_EVENT;
                    //PostPACService(Event2Post);
                }                
            } 
            
            else if (ThisEvent.EventType == PACKET_READY) {
                PacketReadyFlag = true;
                if (SendingTimeoutFlag) {
                    SendPacketAndResetFlags();
                }
            }
            
        break;
        
        case Paired:
            
            if (ThisEvent.EventType == UNPAIRED_EVENT) {
                NextState = Unpaired;
                CheckForPair();
            }

            else if (ThisEvent.EventType == PAIRED_EVENT) {
                int8_t AccelInput = GetDriveCommand();
                int8_t SteerInput = GetSteeringCommand();
                bool SpecAction = false;
                bool Unpair = false;
                if (UNPAIR == 0) Unpair = true;
               
                bool EBrake = false;       
                if (BRAKE == 0) EBrake = true;
                
                EncryptionPacketFlag = false;
                ConstructControlPacket(AccelInput, SteerInput, SpecAction, Unpair, EBrake);
            } 
            
            else if (ThisEvent.EventType == ES_TIMEOUT) {
                if(ThisEvent.EventParam == SENDING_TIMER) {
                    SendingTimeoutFlag = true;
                    if (PacketReadyFlag) {
                        SendPacketAndResetFlags();
                    } 
                }
                
                else if (ThisEvent.EventParam == RESEND_TIMER) {
                    if (!EncryptionPacketFlag) {
                        SendPacketAndResetFlags();
                    } 
                }
                
                else if (ThisEvent.EventParam == ACTIVE_PAIR_TIMER) {
                    ES_Event Event2Post;
                    Event2Post.EventType = UNPAIRED_EVENT;
                    PostPACService(Event2Post);
                    //ES_Timer_InitTimer(SENDING_TIMER,SENDING_TIME);
                }
            } 
            
            else if (ThisEvent.EventType == PACKET_READY) {
                PacketReadyFlag = true;
                if (SendingTimeoutFlag) {
                    SendPacketAndResetFlags();
                }
            }

        break;
        
    }
    
    CurrentState = NextState;
    return ReturnEvent;
}

void TXIF_ISR() {
    // Write next byte to TXREG register
    TXREG = Packet2Send[SendPacketIndex];
    // Increment DataIndex
    SendPacketIndex++;
    // If write is done
    if (SendPacketIndex == Packet2SendSize) {
        // Disable TXIE interrupts
        TXIE = 0;        
        
        // Reset Flags
        //PacketReadyFlag = false;
        //SendingTimeoutFlag = false;
    }   
}

/***************************************************************************
 public functions
 ***************************************************************************/

// Function that returns the current state of the PAC (paired or unpaired)
PACState_t QueryPACState( void ) {
    return CurrentState;
}

bool GetEncryptionFlag( void ) {
    return EncryptionPacketFlag;
}
/***************************************************************************
 private functions
 ***************************************************************************/
static void SendPacketAndResetFlags(void) {
    // Get Packet2Send
    Packet2Send = GetPacket2Send();
    // Get Packet2SendSize
    Packet2SendSize = GetPacket2SendSize();
    // Set DataIndex equal to 0
    SendPacketIndex = 0;
    
    
    // Restart appropriate timers
    if (PacketReadyFlag) { // If new packet is ready, its not a resend so reset all timers
        ES_Timer_InitTimer(SENDING_TIMER,SENDING_TIME);
        ES_Timer_InitTimer(RESEND_TIMER,RESEND_TIME);
        ES_Timer_InitTimer(ACTIVE_PAIR_TIMER,INACTIVE_TIME);
        
        // Reset Flags
        PacketReadyFlag = false;
        SendingTimeoutFlag = false;
     
    }
    else { // If new packet isn't ready, its a resend to restart just that timer
        ES_Timer_InitTimer(RESEND_TIMER,RESEND_TIME);
    }
    
    
    // Enable interrupt by setting TXIE interrupt enable bit (PIE1 register)
    TXIE = 1;

}
/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/



