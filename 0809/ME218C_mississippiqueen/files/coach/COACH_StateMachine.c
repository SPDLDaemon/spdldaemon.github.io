//*********************************************************************
// COACH_StateMachine.c
//NOTE: Includes the Project Main
//*********************************************************************
// Stuart Calder; ME218C
// Mississippi Queen Project: 05/09
//*********************************************************************

//*********************************************************************
// Header File Includes
//*********************************************************************

#include <hidef.h>
#include <mc9s12e128.h>
#include <S12E128bits.h>
#include <bitdefs.h>
#include <stdio.h>
#include "S12eVec.h"
#include "TimerS12.h"
#include "COACH_StateMachine.h"
#include "CommunicationsTest.h"
#include "CoachEvents.h"
#include "CoachDisplay.h"
#include "encoder.h"
#include "seg7_display.h"

//*********************************************************************
// Constant Defines
//*********************************************************************

#define Half_second     31
#define One_second      61  
#define Three_seconds   183
#define Five_seconds    305

#define NO_CODE         0xFF

//LEVEL 0
//Master States
#define StartUp     0
#define Paired      1
#define NotPaired   2

//LEVEL 1
//Paired SM States
#define NormalOperations  0
#define MutinousCrew      1

//LEVEL 2
//MutinousCrew SM substates
#define WaitForSuppression  0
#define WaitingForTOWRP     1

//LEVEL 1
//Notpaired SM States
#define WaitingForIAmAvailable  0
#define AssertingCommand        1

//LEVEL 2
//AssertingComand SM substates
#define WaitingForAssertionOfCommandAction  0
#define WaitingForAssertionOfCommandResult  1


//*********************************************************************
// Variable Definitions
//*********************************************************************

//STATE VARIABLES
static unsigned char MasterState = StartUp;

static unsigned char Paired_Substate = NormalOperations;
static unsigned char Notpaired_Substate = WaitingForIAmAvailable;

static unsigned char Mutiny_Substate = WaitForSuppression;
static unsigned char AssertingCommand_Substate = WaitingForAssertionOfCommandAction;

//COMMUNICATION PARAMETERS
static unsigned char Opcode;
static unsigned char Parameter;
static unsigned char TOWRP_ID;

static unsigned char Controlling_TOWRP;
static unsigned char TOWRP_AvailableID;

static unsigned char TeamColor;

//TIMERS
unsigned int AOC_StartTime = 0;
unsigned int NP_StartTime = 0;
unsigned int Velocity_StartTime = 0;
unsigned int Morale_StartTime = 0;
unsigned int Mutiny_StartTime = 0;
unsigned int Available_StartTime = 0;
unsigned int Music_StartTime = 0;

//*********************************************************************
// Function Definitions
//*********************************************************************

void Run_NormalConditionsSM(void);
void Run_MutinousCrewSM(void);
void Run_WaitingForIAmAvailableSM(void);
void Run_AssertingCommandSM(void);


//*********************************************************************
// Functions
//*********************************************************************

//*********************************************************************
// Master State Machine (Level 0)
//*********************************************************************

void MasterStateMachine(void){
  
  char Jettison = 0;
  
  
  switch (MasterState) {
    
    case StartUp:
       
      // Check for new data
      TOWRP_ID = GetTowrpID();
      Opcode = GetOpcode();
      Parameter = GetParameter();
            
      if((Opcode == AssertionOfCommandResult)&&(Parameter == MississippiQueen)) 
      {
        
        // Indicate our TOWRP is under control
        Controlling_TOWRP = MississippiQueen;
        
        // Enter paired state
        MasterState = Paired;
        
        // Play the music
        PlayMississippiQueen();
        
        // Display which TOWRP we are now controlling
        (void)dispNum(convNum((char)Controlling_TOWRP), TOWRP_DISP);
        
        //START TIMERS
        
        // Start the morale timer
        Morale_StartTime = TMRS12_GetTime();
        
        // Start the velocity command timer
        Velocity_StartTime = TMRS12_GetTime();
        
        (void)printf("Mississippi Queen is under our control\n\r");
      } 
      
      // Check if the Assertion of Command timer has expired
      else if((TMRS12_GetTime() - AOC_StartTime)>(One_second))
      {
        
        // Resend the Assertion of Command
        TeamColor = GetTeamColor();
    
        SendPacket(MississippiQueen,AssertionOfCommand,TeamColor);
        
        // Reset timer
        AOC_StartTime = TMRS12_GetTime();
        
        (void)printf("Resent Assertion of Command \n\r");
        
      } 
      
      else if((TMRS12_GetTime() - NP_StartTime)>(Five_seconds))
      {
        // Can't communicate with own TOWRP, enter not paired
        MasterState = NotPaired;
        
        // Turn on the Not Paired LED
        NotPairedAlert();
        
        (void)printf("Can't find Mississippi Queen - entering notpaired \n\r");
        
      } 
      
                        
      break;
    
    
    case Paired:
      
      //STEP 1 
      
      TOWRP_ID = GetTowrpID();
      
      // Check if the message is from the TOWRP we are controlling
      if(TOWRP_ID == Controlling_TOWRP) {
        
        // Check for new data
        Opcode = GetOpcode();
        Parameter = GetParameter();
      } 
      
      // Otherwise, make sure Opcode and Parameter are not valid
      else {
      
        Opcode = NO_CODE;
        Parameter = NO_CODE;
      }
        
      
      //STEP 2
      
      // Check if the crew has been jettisoned
      Jettison = CrewJettison();
      // if so, switch state to 'Not Paired'
      if(Jettison == 1) {
        
        Jettison = 0;
        MasterState = NotPaired;
        Notpaired_Substate = WaitingForIAmAvailable;
        
        // Turn on the Not Paired LED
        NotPairedAlert();
        
        // Turn off the mutiny lamp, in case it was on
        MutinyAlertLEDOff();
        
        // Blank the seven-segment displays
        (void)dispNum(NTHNG, BOTH);
        
        // Send Jettison the Crew message to TOWRP
        SendPacket(Controlling_TOWRP, JettisonTheCrew,0);
        
        (void)printf("There was a jettison of the crew\n\r");
        
        // Set the music bit low
        PTP &= ~BIT1HI;
      }
      
      
      
      //STEP 3
      
      // Check the morale timer
      else if((TMRS12_GetTime() - Morale_StartTime)>Five_seconds) {
        
        // Enter not paired state and initialize correct substate
        MasterState = NotPaired;
        Notpaired_Substate = WaitingForIAmAvailable;
        
        // Turn on the Not Paired LED
        NotPairedAlert();
        
        // Blank the seven-segment displays
        (void)dispNum(NTHNG, BOTH);
        
        // Turn off the mutiny lamp, in case it was on
        MutinyAlertLEDOff();
        
        (void)printf("The morale timer expired \n\r");
        
        // Set the music bit low
        PTP &= ~BIT1HI;
      }
      
      
      //STEP 4
      
      // Check if there is an 'I Am Available' command from our TOWRP
      // and enter not paired state if there is
      else if((Opcode == IAmAvailable)&&(TOWRP_ID == Controlling_TOWRP)) {
        
        // Enter not paired master state
        MasterState = NotPaired;
        
        // Ensure correct substates are entered
        Notpaired_Substate = AssertingCommand;
        AssertingCommand_Substate = WaitingForAssertionOfCommandAction;
        
        TOWRP_AvailableID = Controlling_TOWRP;
        
        // Turn on the Not Paired LED
        NotPairedAlert();
        
        // Display the available TOWRP number on both displays
        (void)dispNum(convNum(-1*TOWRP_AvailableID), BOTH);
        
        // Turn off the mutiny lamp, in case it was on
        MutinyAlertLEDOff();
        
        // Start the I Am Available timer
        Available_StartTime = TMRS12_GetTime();
        
        (void)printf("An 'I Am Available' Message was received from the TWORP under control\n\r");
        (void)printf("I am Available timer started. \n\r");
        
        // Set the music bit low
        PTP &= ~BIT1HI;
      }

      //STEP 5
      else{
        
        // Run lower-level state machine
        switch(Paired_Substate) {
          
          case NormalOperations:
        
            Run_NormalConditionsSM();
            break;
          
          case MutinousCrew:
        
            Run_MutinousCrewSM();
            break;
          
        }
      }
      
      break;
      
    
    case NotPaired:
    
      //STEP 1
      
      // Check for new data
      TOWRP_ID = GetTowrpID();
      Opcode = GetOpcode();
      Parameter = GetParameter();
      
      //STEP 2
    
      // Run lower-level state machine
      switch(Notpaired_Substate) {
      
        case WaitingForIAmAvailable:
      
          Run_WaitingForIAmAvailableSM();
          break;
        
        case AssertingCommand:
          
          if((Opcode == AssertionOfCommandResult)&&(Parameter != MississippiQueen)) {
            
            // Somebody else must have paired with the TOWRP before us
            Notpaired_Substate = WaitingForIAmAvailable;
            
            // Blank the seven-segment displays
            (void)dispNum(NTHNG, BOTH);
            
            (void)printf("Someone was able to control the TOWRP before us \n\r");
          } 
          
          else if((TMRS12_GetTime() - Available_StartTime)>(Three_seconds)) 
          {
            
            // Set notpaired substate
            Notpaired_Substate = WaitingForIAmAvailable;
            
            // Blank the seven-segment displays
            (void)dispNum(NTHNG, BOTH);
          }
            
          else {
        
          Run_AssertingCommandSM();
          }
          
          break;
      }
      
      break;
      
  }
  
}
  
  
//*********************************************************************
// Normal Operations Sub-State Machine (Level 1)
//*********************************************************************
  
void Run_NormalConditionsSM() {
  
  char Free_Analog = 0;
  char Free_Digital = 0;
  char V = 0;
  char e = 0;
  char CurrentSpeed = 0;
  
  //STEP 1
      
  // Send velocity command if 1s has elapsed since last sent
  if((TMRS12_GetTime() - Velocity_StartTime)>(Half_second)) {
    
    V = GetTranslationalVelocity();
    e = getSteering();    
    
    CurrentSpeed = (V + e);
        
    SendPacket(Controlling_TOWRP, Velocity, CurrentSpeed);
    (void)printf("Sent velocity command: %x \n\r",CurrentSpeed);
    
    // Restart velocity timer
    Velocity_StartTime = TMRS12_GetTime();
            
  }
  
    
  //STEP 2
  
  // Check if the free digital command has been activated
  Free_Digital = FreeDigitalActivate();
  
  // if so, send free analog command
  if(Free_Digital == 1) {
        
    Free_Digital = 0;
    
    // Send digital command    
    SendPacket(Controlling_TOWRP, FreeDigital, 0);
    
    (void)printf("The free digital command was activated\n\r");
  }
  
  
  //STEP 3
  
  // Check if the free analog command has been activated
  Free_Analog = FreeAnalogActivate();
  
  // If so, send free analog command
  if(Free_Analog == 1) {
        
    Free_Analog = 0;
    
    SendPacket (Controlling_TOWRP, FreeAnalog, GetFreeAnalogParameter());

    (void)printf("The free analog command was activated\n\r");
  }
  
  
  //STEP 4
  
  // Check if there has been an update morale package from our TOWRP
  if(Opcode == UpdateMorale) {
    
    // Display the new morale on the seven-segment displays
    (void)dispNum(convNum((char)Parameter), MORALE_DISP);
    
    Morale_StartTime = TMRS12_GetTime();
    
    (void)printf("The current morale is: %x \n\r",Parameter);
  }
  
  
  //STEP 5
  
  // Check if there has been a mutiny in progress package
  else if(Opcode == MutinyInProgress) {
    
    // If there has, change the paired substate
    Paired_Substate = MutinousCrew;
    
    // Set entry state
    Mutiny_Substate = WaitForSuppression;
    
    // Alert user by setting LED
    MutinyAlertLED();
    
    // Update the morale display to -10
    (void)dispNum(NEG_A, MORALE_DISP);
  }
}
    

//*********************************************************************
// Mutinous Crew Sub-State Machine (Level 1)
//*********************************************************************    
    
void Run_MutinousCrewSM(void) {
  
  char Check_Mutiny = 0;
  
  switch(Mutiny_Substate) {
    
    case WaitForSuppression:
    
      // Check if the suppress mutiny command has been activated
      Check_Mutiny = CheckMutinySuppression();
  
      // if so, send free analog command
      if(Check_Mutiny == 1) {
        
        Check_Mutiny = 0;
        
        // Send suppress mutiny command to TWORP
        SendPacket(Controlling_TOWRP,SuppressMutiny,0x00);
        
        // Change the state to wait for the TOWRP response
        Mutiny_Substate = WaitingForTOWRP;
        
        // Start the mutiny timer
        Mutiny_StartTime = TMRS12_GetTime();
          
        (void)printf("The mutiny was suppressed\n\r");
      }
      
      break;
      
    case WaitingForTOWRP:
      
      // If the morale status is -5
      if((Opcode==UpdateMorale)&&(Parameter==0xFB)) {
               
        // Reset paired substate to normal operations
        Paired_Substate = NormalOperations;
        
        // Start velocity timer
        Velocity_StartTime = TMRS12_GetTime();
        
        // Turn off the mutiny lamp
        MutinyAlertLEDOff();
        
        // Change morale display to -5
        (void)dispNum(NEG_5, MORALE_DISP);
        
        (void)printf("The mutiny was put down \n\r");
        
      } 
      
      else if((TMRS12_GetTime() - Mutiny_StartTime)>(One_second))
      {
        // Send suppress mutiny command to TWORP
        SendPacket(Controlling_TOWRP,SuppressMutiny,0x00);
        
        // Restart the mutiny timer
        Mutiny_StartTime = TMRS12_GetTime();
        
        (void)printf("The mutiny timer has expired \n\r");
      
      }
      
      break;
  }
}
    
    
//*********************************************************************
// Waiting For "I Am Available" Sub-State Machine (Level 1)
//*********************************************************************    
    
void Run_WaitingForIAmAvailableSM(void) {
  
  if(Opcode == IAmAvailable){
    
    // Store the ID of the TOWRP that is available
    TOWRP_AvailableID = TOWRP_ID;
    
    // Display the available TOWRP number on both displays
    (void)dispNum(convNum(-1*TOWRP_AvailableID), BOTH);
    
    // Change the state to assert command over the TOWRP
    Notpaired_Substate = AssertingCommand;
    
    // Reset the Asserting Command substate
    AssertingCommand_Substate = WaitingForAssertionOfCommandAction;
    
    // Start the I am Available timer
    Available_StartTime = TMRS12_GetTime();
    
    (void)printf("The available TOWRP is: %x \n\r",TOWRP_AvailableID);
  }   
}


//*********************************************************************
// Asserting Command Sub-State Machine (Level 1)
//*********************************************************************    
    
void Run_AssertingCommandSM(void) {
 
  char Check_Assertion = 0;
  
  switch(AssertingCommand_Substate) {
    
    case WaitingForAssertionOfCommandAction:
         
      // Check if the assert command has been activated
      Check_Assertion = CheckAssertionCommand();
  
      // if so, send assertion of command
      if(Check_Assertion == 1) {
        
        Check_Assertion = 0;
        
        // Send assertion of command to TWORP
        
        TeamColor = GetTeamColor();
    
        SendPacket(TOWRP_AvailableID,AssertionOfCommand,TeamColor);
        
        // Change the state to wait for the TOWRP response
        AssertingCommand_Substate = WaitingForAssertionOfCommandResult;
        
        // Start the assertion of command timer
        AOC_StartTime = TMRS12_GetTime();
          
        (void)printf("Command was asserted\n\r");
      } 
      
      else if((Opcode == IAmAvailable)&&(TOWRP_ID == TOWRP_AvailableID)) {
        
        // Reset I Am Available timer 
        Available_StartTime = TMRS12_GetTime();
        
        (void)printf("The same TOWRP has sent an I Am Available\n\r");
      }
      
      break;
      
    case WaitingForAssertionOfCommandResult:
    
      if((Opcode == AssertionOfCommandResult)&&(Parameter == MississippiQueen)) {
        
        // Go into paired master state
        MasterState = Paired;
        // Enter normal operations
        Paired_Substate = NormalOperations;
        
        // Turn off the Not Paired LED
        NotPairedAlertOff();
                
        // Update the TOWRP control number
        Controlling_TOWRP = TOWRP_AvailableID;
        
        // Play the music, only if it is our TOWRP
        if(TOWRP_AvailableID == MississippiQueen){
          PlayMississippiQueen();
        }
        
        // Display which TOWRP we are now controlling
        (void)dispNum(convNum((char)Controlling_TOWRP), TOWRP_DISP);
        
        // Start the morale timer
        Morale_StartTime = TMRS12_GetTime();
        
        // Start the velocity timer
        Velocity_StartTime = TMRS12_GetTime();
        
        (void)printf("The TOWRP number is now: %x \n\r",TOWRP_AvailableID);
        
      }
      
      // Check the assertion of command timer
      else if((TMRS12_GetTime() - AOC_StartTime)>(One_second))
      {
        // Send assertion of command to TWORP
        TeamColor = GetTeamColor();
        
        SendPacket(TOWRP_AvailableID,AssertionOfCommand,TeamColor);
        
        // Restart the assertion of command timer
        AOC_StartTime = TMRS12_GetTime();  
        
        (void)printf("The assertion of command timer was reset \n\r");
        
      }  
      
      break;
    
  }
}
 
  
//*********************************************************************
// Project Main
//*********************************************************************

 void main (void){
    
    EnableInterrupts;
    
    TMRS12_Init(TMRS12_RATE_16MS);
    
    COACH_INIT();
  
    SCI_INIT();
  
    (void)printf("Hardware reset\n\r");
    
    // Start the assertion of command timer
    AOC_StartTime = TMRS12_GetTime();
    
    // Start the not paired timer
    NP_StartTime = TMRS12_GetTime();
    
    TeamColor = GetTeamColor();
    
    SendPacket(MississippiQueen,AssertionOfCommand,TeamColor);
  
    while(1){
      MasterStateMachine();
            
    }
 }
  
    
    





