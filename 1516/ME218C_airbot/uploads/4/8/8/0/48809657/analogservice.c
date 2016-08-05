
/****************************************************************************
 Module
   AnalogService.c

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
#include "PACService.h"
#include "ReceivingService.h"
#include "ProcessDataModule.h"
#include "AnalogService.h"
#include <stdlib.h>
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
#define ONE_SEC 100
#define HALF_SEC ONE_SEC/2
#define TENTH_SEC ONE_SEC/10
#define THREE_SEC ONE_SEC*3
#define FIFTH_SEC ONE_SEC/5
#define ACQUISITION_TIME 1
#define INITIALIZATION_TIME 50

#define SPBRGL_CONSTANT 0x33
#define XBEE_OVERHEAD 9
#define MAX_DATA_SIZE 33
#define MAX_PACKET_SIZE (XBEE_OVERHEAD+MAX_DATA_SIZE)

#define CHANNEL_MASK    0b10000011
#define SELECT_CHANNEL  0b00100000  // AN8
#define DRIVE_CHANNEL   0b00110000  // AN10 Drive sign will be flipped (Rotated 90 degrees clockwise)
#define STEER_CHANNEL   0b00101000  // AN12

#define STEER_OFFSET (125-44)
#define DRIVE_OFFSET (123-44)
#define TOLERANCE 120
#define SIGN_MASK 0x80
/*---------------------------- Module Functions ---------------------------*/
/* prototypes for private functions for this service.They should be functions
   relevant to the behavior of this service
*/
static void InitAnalogModule(void);
/*---------------------------- Module Variables ---------------------------*/
// with the introduction of Gen2, we need a module level Priority variable
static uint8_t MyPriority;
//static uint8_t Data2Send[] = {0x21,0x18,0xFF,0x7E,0x12,0x05};
//static uint8_t *Data2Send;
//static uint8_t Packet2Process[MAX_PACKET_SIZE];
//static uint8_t Packet2Send[MAX_PACKET_SIZE];
//static uint8_t API_Identifier = 0x01;
//static uint16_t DestinationAddress = 0x2181;
//static uint8_t FrameID = 0x01;
//static uint8_t CheckSum = 0;
//static uint8_t *Packet2Send;
//static uint8_t Packet2SendSize;
//static uint8_t SendPacketIndex = 0;
static AnalogState_t CurrentState;
static int8_t SteerReading = 0;
static int8_t DriveReading = 0;
static uint8_t LobbyistReading = 0;
static bool JoystickInitializedSteer = false;
static bool JoystickInitializedDrive = false;
static int8_t SteerOffset = -78;
static int8_t DriveOffset = 0;
//static bool SendingTimeoutFlag = false;
//static bool PacketReadyFlag = false;



/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
     InitAnalogService

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
bool InitAnalogService ( uint8_t Priority )
{
    ES_Event ThisEvent;

    MyPriority = Priority;
    /********************************************
     in here you write your initialization code
     *******************************************/
    InitAnalogModule();
    CurrentState = Initializing;
    // Enable sending timer
    //ES_Timer_InitTimer(ANALOG_TIMER, ACQUISITION_TIME);
    ES_Timer_InitTimer(ANALOG_INIT_TIMER, INITIALIZATION_TIME);
  
    // post the initial transition event
    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService( MyPriority, ThisEvent) == true) {
        return true;
    } else {
        return false;
    }
}

static void InitAnalogModule(void) {
    // Set RB0 and RB1 to inputs
    //TRISB0 = 1;
    //TRISB1 = 1;
    
    // Set RB0 and RB1 to analog
    //ANSB0 = 1;
    //ANSB1 = 1;
    
    // Set AD clock to FOSC/64, VDD and VSS to ref voltages and sign-magnitude format
    ADCON1 = 0b01100000;
    
    ADFM = 1;
    ADRMD = 0;
    
    // Turn ADC Module on
    ADON = 1;
    
    // Set initial channel to sample steering input
    ADCON0 &= CHANNEL_MASK;
    ADCON0 |= STEER_CHANNEL;
    
    // Clear ADC Interrupt Flag
    ADIF = 0;
    
    // Enable Interrupts
    ADIE = 1;
    PEIE = 1;  

}

/****************************************************************************
 Function
     PostAnalogService

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
bool PostAnalogService( ES_Event ThisEvent ) {
  return ES_PostToService( MyPriority, ThisEvent);
}

/****************************************************************************
 Function
    RunAnalogService

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
ES_Event RunAnalogService( ES_Event ThisEvent ) {
    AnalogState_t NextState = CurrentState;
    ES_Event ReturnEvent;
    ReturnEvent.EventType = ES_NO_EVENT; // assume no errors
    /********************************************
     in here you write your service code
     *******************************************/
    if (CurrentState == Initializing) {
        if (ThisEvent.EventType == ES_TIMEOUT) {
            if (ThisEvent.EventParam == ANALOG_INIT_TIMER) {
                NextState = Waiting;
                ES_Timer_InitTimer(ANALOG_TIMER, ACQUISITION_TIME);
            }
        }
    }
    else if (CurrentState == Waiting) {
        if (ThisEvent.EventType == ES_TIMEOUT) {
            if(ThisEvent.EventParam == ANALOG_TIMER) {
                GO = 1; // Start conversion
                NextState = Sampling;
                /*if (RA1 == 1) {
                    RA1 = 0;
                } else if (RA1 == 0) {
                    RA1 = 1;
                } */  
                //ES_Timer_InitTimer(ANALOG_TIMER, ACQUISITION_TIME);
            }
        }
    }
    
    CurrentState = NextState;
    return ReturnEvent;
}

/***************************************************************************
 public functions
 ***************************************************************************/
int8_t GetDriveCommand(void) {
    return (-1 * DriveReading);
}

int8_t GetSteeringCommand(void) {
    return SteerReading;
}

uint8_t GetLobbyistNumber(void) {
    return ( LobbyistReading / (256/4) );
}

void AnalogISR(void) {
    if (RA0 == 1)  {
      //  RA0 = 0;
    }
    else {
       // RA0 = 1;
    }
    
    int16_t Reading = ((ADRESH << 8) + (ADRESL));
    int8_t ShiftedReading;
    // Determine which channel to read
    switch (ADCON0 & ~(CHANNEL_MASK)) {
        case STEER_CHANNEL:
            ShiftedReading = (Reading >> 4) - SteerOffset;
            if (!JoystickInitializedSteer) {
                SteerOffset = (ShiftedReading);
                RA0 = 1;
                JoystickInitializedSteer = true;
            }
            
            // If value changes drastically from an overflow, just keep old value
            if (((SteerReading & SIGN_MASK) == (ShiftedReading & SIGN_MASK)) ||
                 (abs(ShiftedReading) < TOLERANCE)) { 
                SteerReading = ShiftedReading;
            }
            ADCON0 &= CHANNEL_MASK;
            ADCON0 |= DRIVE_CHANNEL;   

        break;
        
        case DRIVE_CHANNEL:
            ShiftedReading = (Reading >> 4) - DriveOffset;
            if (!JoystickInitializedDrive) {
                DriveOffset = (ShiftedReading);
                JoystickInitializedDrive = true;
                //RA0 = 0;
            }
            // If value changes drastically from an overflow, just keep old value
            if (((DriveReading & SIGN_MASK) == (ShiftedReading & SIGN_MASK)) ||
                 (abs(ShiftedReading) < TOLERANCE)) {
                if (ShiftedReading != -128) {
                    DriveReading = ShiftedReading;
                }
            }
            ADCON0 &= CHANNEL_MASK;
            ADCON0 |= SELECT_CHANNEL;
        break;        
        
        case SELECT_CHANNEL:
            ShiftedReading = (Reading >> 4);
    
            // If value changes drastically from an overflow, just keep old value
           // if (((DriveReading & SIGN_MASK) == (ShiftedReading & SIGN_MASK)) ||
           //      (abs(ShiftedReading) < TOLERANCE)) { 
                LobbyistReading = (uint8_t)ShiftedReading;
           // }
            ADCON0 &= CHANNEL_MASK;
            ADCON0 |= STEER_CHANNEL;
        break;    
    }
    


    ES_Timer_InitTimer(ANALOG_TIMER,ACQUISITION_TIME);
    CurrentState = Waiting;
    
    // Clear flag for the interrupt
    ADIF = 0;
    
    
}

/***************************************************************************
 private functions
 ***************************************************************************/


/*------------------------------- Footnotes -------------------------------*/
/*------------------------------ End of file ------------------------------*/



