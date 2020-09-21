#ifndef COMMUNICATIONS_TEST_H
#define COMMUNICATIONS_TEST_H

//TOWRP NUMBERS
#define MississippiQueen  0x05

//OPCODE DEFINITIONS
#define SuppressMutiny              0x00
#define AssertionOfCommand          0x01
#define JettisonTheCrew             0x02
#define Velocity                    0x03
#define FreeDigital                 0x04
#define FreeAnalog                  0x05


#define MutinyInProgress            0x20
#define IAmAvailable                0x21
#define AssertionOfCommandResult    0x22
#define UpdateMorale                0x23

#define No_Opcode                   0xFF


// Initialize the asynchronous communications
void SCI_INIT(void);

// Send a packet of data
void SendPacket(char, char, char);
       
// Find out what TOWRP is sending the command                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                           
unsigned char GetTowrpID(void);

// Find the opcode of a message
unsigned char GetOpcode (void);

// Find the parameter of the command
unsigned char GetParameter (void);

#endif