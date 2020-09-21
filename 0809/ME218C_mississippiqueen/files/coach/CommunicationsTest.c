//*********************************************************************
// CommunicationsTest.c
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
#include "ADS12.H"
#include "CommunicationsTest.h"

//*********************************************************************
// Test Define
//*********************************************************************
//#define COMMUNICATIONS_TEST

//*********************************************************************
// Constant Defines
//*********************************************************************

#define StartDelimiter     0x7E

#define TX_LengthMSB       0x00
#define TX_LengthLSB       0x07
#define TX_Identifier      0x01 
#define TX_FrameID         0x83
#define TX_AddressMSB      0xAF
#define TX_Options         0x01

//*********************************************************************
// Variable Definitions
//*********************************************************************
// Declare receive variables
static unsigned char RX_LengthMSB;
static unsigned char RX_LengthLSB;
static unsigned char RX_Identifier;
static unsigned char RX_Byte5;
static unsigned char RX_Byte6;
static unsigned char RX_Byte7;
static unsigned char RX_Byte8;
static unsigned char RX_Byte9;
static unsigned char RX_Byte10;
static unsigned char RX_CheckSum;

// Software Flags
static char Opcode_Flag = 0;
static char Parameter_Flag = 0;
static char TowrpID_Flag = 0;

static char State = 0; 
static char count = 1;

//*********************************************************************
// Functions
//*********************************************************************

//*********************************************************************
// Initialization Function
//*********************************************************************

void SCI_INIT(void){
  
  // Set the baud rate to 9600
  SCI1BD = 156;
  
  // 8-bit mode, both pins in use, always awake
  SCI1CR1 = 0x00;
  
  // Enable both transmit and receive, with interrupts on receive
  SCI1CR2 = 0x2C;
  
  // Transmit out, standard break length
  SCI1SR2 = 0x02;
 
}

//*********************************************************************
// Send Data Packet Function
//*********************************************************************

void SendPacket(char TX_AddressLSB, char TX_MsgMSB, char TX_MsgLSB) {

      char TX_CheckSum;
      int TX_Sum;
      
      // Wait for the transmit data register to be available
      // This code is intentionally blocking
      while(!(SCI1SR1 & BIT7HI)) {}
  
      //SEND DATA PACKET
      
      SCI1DRL = StartDelimiter;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_LengthMSB;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_LengthLSB;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_Identifier;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_FrameID;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_AddressMSB;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_AddressLSB;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_Options;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_MsgMSB;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      SCI1DRL = TX_MsgLSB;
      while(!(SCI1SR1 & BIT7HI)) {}
      
      // Calculate the sum of the data frame
      TX_Sum = TX_Identifier+TX_FrameID+TX_AddressMSB+TX_AddressLSB+TX_Options+TX_MsgMSB+TX_MsgLSB;
      
      // Calculate the check sum by subtracting the lower byte of the sum from 0xFF
      TX_CheckSum = 0xFF - (TX_Sum & 0x00FF);
      
      SCI1DRL = TX_CheckSum;   
        
}


//*********************************************************************
// Receive Data Packet Interrupt
//*********************************************************************

void interrupt _Vec_sci1 ReceivePacket(void){
  	
  	int Sum;
  	int temp;
 
  	// Clear interrupt flag
  	temp = SCI1SR1;
  	
  	// Enter a new case depending on where you are in the receive process  
    switch(State) 
    {
    
    // The first byte will be the start delimiter  
    case 0: 
      
      // If the start delimiter has been received then continue to receive next byte
      if(SCI1DRL == StartDelimiter) {  
      
      // Re-set all variables for the incoming receive
        RX_LengthMSB = 0;
        RX_LengthLSB = 0;
        RX_Identifier = 0;
        RX_Byte5 = 0;
        RX_Byte6 = 0;
        RX_Byte7 = 0;
        RX_Byte8 = 0;
        RX_Byte9 = 0;
        RX_Byte10 = 0;
        RX_CheckSum = 0;
      
      State = State+1;
      count = 1;
      } else {
        
        // The start delimiter was not received so ignore incoming transmission
        State = 0;
      }
      break;
    
    // The second/third bytes will be the length of the data frame
    case 1:      
      RX_LengthMSB = SCI1DRL;
      State = State+1;
      break;
    
    case 2:      
      RX_LengthLSB = SCI1DRL;
      State = State+1;
      break;
    
    // The fourth - (n-1) bytes will be the data package
    case 3:
      
      // Enter a new case depending once the previous frame byte has been received
      // The maximum frame length is 7 bytes, so 7 cases are handled here
      
      switch(count) 
      {
        
        // For a receive, the first byte is the identifier
        case 1:
                
        RX_Identifier = SCI1DRL;
        count = count+1;
        
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      
        // Continue for each of the 7 bytes of frame data
        // Enter new state after successful receipt
        
        case 2:
        
        RX_Byte5 = SCI1DRL;
        count = count+1;
        
        // Check the length of the frame data has not exceeded the indicated length
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      
      
        case 3:
        
        RX_Byte6 = SCI1DRL;
        count = count+1;
        
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      
      
        case 4:
        
        RX_Byte7 = SCI1DRL;
        count = count+1;
        
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      
      
        case 5:
        
        RX_Byte8 = SCI1DRL;
        count = count+1;
        
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      
      
        case 6:
        
        RX_Byte9 = SCI1DRL;
        count = count+1;
        
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      
        case 7:
        
        RX_Byte10 = SCI1DRL;
        count = count+1;
        
        if(count>RX_LengthLSB) {
          State = State+1;
        }
        break;
      }
      break;
    
      // The last byte is the check sum 
      case 4:
      
        RX_CheckSum = SCI1DRL;
                        
        State = 0;
        
        // Compute the sum of bytes 4-10
        Sum = ((RX_Identifier+RX_Byte5+RX_Byte6+RX_Byte7+RX_Byte8+RX_Byte9+RX_Byte10) & 0xFF);
        
        // Check if the check sum is valid, and process data
        if((Sum+RX_CheckSum) == 0xFF) { 
                 
          TowrpID_Flag = 1;
        }
        
        // The check sum must be wrong, so indiacte corrupt data
        else {
          
          (void)printf("There was an corrupt package");
        }

        break;
    
    
    default:
     
      (void)printf("False\n\r");
     
      break;
    
    }	
			
	return;
}

//*********************************************************************
// Get TOWRP Identification
//*********************************************************************

unsigned char GetTowrpID(void) {
  
  //Test to see if there is an ID ready
  if(TowrpID_Flag == 1) {
    
    // Set opcode flag
    Opcode_Flag = 1;  
    
    TowrpID_Flag = 0;
    //(void)printf("The TOWRP ID is:%x \n\r",RX_Byte6);
    
    return RX_Byte6;
  } else{
    
    return 0xEF;
  }
}    
    

//*********************************************************************
// Get Operation Code Function
//*********************************************************************

unsigned char GetOpcode(void) {
  
  // If there has been a receive
  // Switch through the possible cases to identify op code
  if(Opcode_Flag == 1) {
  
    // Set parameter flag
    Parameter_Flag = 1;  
    
    //(void)printf("The opcode is: %x \n\r",RX_Byte9);
    
    switch (RX_Byte9) {
      
      case MutinyInProgress:
        
        Opcode_Flag = 0;
        return MutinyInProgress;
        break;
        
      case IAmAvailable:
        
        Opcode_Flag = 0;
        return IAmAvailable;
        break;
        
      case AssertionOfCommandResult:
         
        Opcode_Flag = 0;
        return AssertionOfCommandResult;
        break;
        
      case UpdateMorale:
        
        Opcode_Flag = 0;
        return UpdateMorale;
        break;
        
      default:
      
        Opcode_Flag = 0;
        return No_Opcode;
        break;
    }
  } else{
    
    return No_Opcode;
  }
  
}

//*********************************************************************
// Get Operation Code Function
//*********************************************************************

unsigned char GetParameter(void) {
  
  // Test to see if there is a parameter ready
  if(Parameter_Flag == 1) {
    
    Parameter_Flag = 0;
    //(void)printf("The parameter is: %x \n\r",RX_Byte10);
    
    return RX_Byte10;
  }
}
 

//*********************************************************************
// Test Harness #1
//*********************************************************************

#ifdef COMMUNICATIONS_TEST

  void main (void){
  
    char TX_MsgMSB;
    char TX_MsgLSB;
    
    unsigned char O;
    unsigned char P;
    unsigned char ID;
    
    static char last_trig = 0;
    char curr_trig;
    
    EnableInterrupts;
  
    // Set P ports to be inputs
    DDRP = 0x00;
  
    SCI_INIT();
  
    (void)printf("Hardware reset\n\r");
  
    while(1){
    
      // Check for a new trigger command from our node
      curr_trig = PTP & BIT2HI;
      if(curr_trig != last_trig) {
      
        TX_MsgMSB = 0x00;
        // Determine data packet
        TX_MsgLSB = PTP & 0x3;
      
        (void) printf("Changed state\n\r");
        last_trig = curr_trig; 
      
        SendPacket(0x05, TX_MsgMSB, TX_MsgLSB); 
      }
      
      O = GetOpcode();
      P = GetParameter();
      ID = GetTowrpID();
            
    }
  }  
  
#endif
    
    
    
    
    
    
    
    
    
    
    
    
    
    
  
  