/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "uartstdio.h"
#include "ProcessReceive.h"

// the common headers for C99 types 
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

// the headers to access the GPIO subsystem
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_gpio.h"
#include "inc/hw_sysctl.h"
#include "inc/hw_pwm.h"
#include "inc/hw_Timer.h"
#include "inc/hw_nvic.h"
#include "inc/hw_ssi.h"
#include "inc/hw_memmap.h"
#include "inc/hw_uart.h"

// the headers to access the TivaWare Library
#include "driverlib/sysctl.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/interrupt.h"

#include "LobbyistCommunication.h"
#include "LiftFanModule.h"
#include "PropModule.h"
#include "DMCModule.h"
#include "ToggleLEDService.h"
#include "ADMulti.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define ALL_BITS (0xff<<2)

#define REQ_PAIR_HEADER 0x00
#define ENCR_KEY_HEADER 0x01
#define CTRL_HEADER 0x02
#define STATUS_HEADER 0x03

// API Identifiers
#define	API_TX_STATUS 0x89
#define API_RX_PACKET 0x81

#define MAX_DATA_SIZE 33
#define FRAME_OVERHEAD 5

#define COLOR_MASK 0x80
#define BOT_NUM_MASK 0x7F
#define BREAK_MASK 0x01
#define UNPAIR_MASK 0x02

#define OWN_NUMBER 0x05

#define BLUE 1
#define RED 0

#define KEY_ROLLOVER 0x1F

#define TX_TIME ONE_SEC
#define PAIRING_TIME ONE_SEC*45

#define CTRL_LENGTH 5
#define ONE_K_READING 3976
#define	TWO_K_READING 3012
#define THREE_K_READING	2435
#define	FOUR_K_READING 2040

#define	TOLERANCE 100
/*---------------------------- Module Functions ---------------------------*/
static void SendACK();
static void InitHardware(void);
static void Decrypt(void);
static void ProcessCTL(void);
static bool RepeatedFrame(void);
static void GetBotNum(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
static uint8_t FrameData[MAX_DATA_SIZE + FRAME_OVERHEAD];
static uint8_t DecryptedFrameData[MAX_DATA_SIZE + FRAME_OVERHEAD];
static RXProcessState_t CurrentState;
static bool Paired;
static bool Decrypt_Error;
static uint8_t Color;
static uint8_t Encryption_Key[32];
static uint8_t Bytes2Send[3];
static uint8_t AddressLSB;
static uint8_t AddressMSB;

static uint8_t LastAddressLSB;
static uint8_t LastAddressMSB;

static uint8_t KeyIdx;
static uint8_t TestCount;
static uint8_t LastCommandFrame[MAX_DATA_SIZE];
static uint8_t EncryptedCheckSum;
static uint8_t BotNumber;
static uint32_t RawReading[2];

/*------------------------------ Module Code ------------------------------*/

bool InitProcessReceiveService(uint8_t Priority) {
	MyPriority = Priority;
	ES_Event ThisEvent;
	ThisEvent.EventType = ES_INIT;
	// Initialize DMC hardware
	InitDMCHardware();
	//Initialize PE0 and PE1 as analog inputs
	 ADC_MultiInit(2);
	//Get Bot Number
	GetBotNum();
	// Initialize Lift Fan hardware
	InitLiftFan();
	CurrentState = WAITING4PAIR;
	Paired = false;
	Decrypt_Error = false;
	EncryptedCheckSum = 0x00;
	//ES_Timer_InitTimer(TestTimer, ONE_SEC);
	if (ES_PostToService (MyPriority, ThisEvent) == true) {
			return true;
		} else {
			return false;
		}
}

static void GetBotNum(void){
//Define RawReading
	//Read A/D pin into variable RawReading
	ADC_MultiRead(RawReading);
	//Scale to number from 0 to 3 (use RawReading[1])
	printf("Raw Reading is %u \r\n", RawReading[1]);
	if (RawReading[1] > ONE_K_READING-TOLERANCE && RawReading[1] < ONE_K_READING + TOLERANCE) {
		BotNumber = 0;
	} else if (RawReading[1] > TWO_K_READING-TOLERANCE && RawReading[1] < TWO_K_READING + TOLERANCE) {
		BotNumber = 1;
	} else if (RawReading[1] > THREE_K_READING-TOLERANCE && RawReading[1] < THREE_K_READING + TOLERANCE) {
		BotNumber = 2;
	} else if (RawReading[1] > FOUR_K_READING-TOLERANCE && RawReading[1] < FOUR_K_READING + TOLERANCE) {
		BotNumber = 3;
	}
	printf("BotNumber is %u\r\n", BotNumber);
}

bool PostProcessReceiveService( ES_Event ThisEvent ){
	  //Post Event to queue
	return ES_PostToService( MyPriority, ThisEvent);
}

ES_Event RunProcessReceiveService( ES_Event ThisEvent ){
	ES_Event ReturnEvent;
	if (ThisEvent.EventType == ES_NEW_KEY) {
		if (ThisEvent.EventParam == 'p') {
			for (uint8_t index; index < 32; index++) {
				printf("Encryption Key at index %u is %x \r\n", index, Encryption_Key[index]);
			}
		}
	}
	// If there is a timeout
	if (ThisEvent.EventType == ES_TIMEOUT) {
		//If timer is XMIT_TIMER 
		if (ThisEvent.EventParam == TX_TIMER) {
			// Deactivate Lift Fan and props
			StopLiftFan();
			StopProps();
			// Indicate available for pairing on DMC //Need to do this still
			ClearDMCPaired();
			// Disable the 45s pairing timer
			ES_Timer_StopTimer(PAIRING_TIMER);
			// Disable the TX Timer
			ES_Timer_StopTimer(TX_TIMER);
			printf("TX Timer timed out\r\n");
			// Transmit Status with pairing bit cleared
			Paired = false;
			ES_Event Event2Post;
			Event2Post.EventType = UNPAIR_EVENT;
			PostToggleLED(Event2Post);
			
			//SendACK();
			// Change state to WAITING4PAIR
			//CurrentState = WAITING4PAIR;
		}else if (ThisEvent.EventParam == PAIRING_TIMER) {
			printf("Pairing TIMER Timed out \r\n");
			// Deactivate Lift Fan and props
			StopLiftFan();
			StopProps();
			// Indicate available for pairing on DMC //Need to do this still
			ClearDMCPaired();
			// Disable the 45s pairing timer
			ES_Timer_StopTimer(PAIRING_TIMER);
			// Disable the TX Timer
			ES_Timer_StopTimer(TX_TIMER);
			// Transmit Status with pairing bit cleared
			Paired = false;
			ES_Event Event2Post;
			Event2Post.EventType = UNPAIR_EVENT;
			PostToggleLED(Event2Post);
			//SendACK();
			// Change state to WAITING4PAIR
			//CurrentState = WAITING4PAIR;
		}
	}else if (ThisEvent.EventType == DECRYPTION_ERROR) { 
			// Deactivate Lift Fan and props
			StopLiftFan();
			StopProps();
			// Indicate available for pairing on DMC
			ClearDMCPaired();
			// Disable the 45s pairing timer
			ES_Timer_StopTimer(PAIRING_TIMER);
			// Disable the TX Timer
			ES_Timer_StopTimer(TX_TIMER);
			// Transmit Status with pairing bit cleared and decryption error bit set
			Paired = false;
			ES_Event Event2Post;
			Event2Post.EventType = UNPAIR_EVENT;
			PostToggleLED(Event2Post);
			Decrypt_Error = true;
			//SendACK();
			// Change state to WAITING4PAIR
			//CurrentState = WAITING4PAIR;
	}else if (ThisEvent.EventType == GOOD_MESSAGE) {
			if (FrameData[0] == API_TX_STATUS) {
				if (FrameData[2] != 0) {			//If not sucess, resend ACK
					SendACK();
				} 
			} else if (FrameData[0] == API_RX_PACKET) { 
				//printf("Frame Data 0 is %x \r\n", FrameData[0]);
				switch(CurrentState) {
					case WAITING4PAIR:
						//If not paired
						if (Paired == false) {
							//If we get a REQ_PAIR_HEADER in FrameData[5]
							if (FrameData[5] == REQ_PAIR_HEADER) {
								EncryptedCheckSum = 0;
								// If lobbyist number in Request To Pair matches own number 
								if (LastAddressLSB != FrameData[2] || LastAddressMSB != FrameData[1]) {
									if ((FrameData[6] & BOT_NUM_MASK) == BotNumber) {
										printf("Pair Number is %x \r\n", FrameData[6]);
										// Turn on paired output to DMC
										//HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= PAIR_OUTPUT_HI;
										SetDMCPaired();
										// Indicate succesfful pairing on DMC
										// Indicate team color specified by Request To Pair on DMC
										if ((FrameData[6]& COLOR_MASK) == BIT7HI) {
											Color = BLUE;
											//HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) &= COLOR_OUTPUT_LO;
											SetDMCColor(Color);
										} else {
											Color = RED;																							// DO DMC stuff with color
											//HWREG(GPIO_PORTA_BASE+(GPIO_O_DATA + ALL_BITS)) |= COLOR_OUTPUT_HI;
											SetDMCColor(Color);
										}
										ES_Event Event2Post;
										Event2Post.EventType = PAIR_EVENT;
										PostToggleLED(Event2Post);
										//save away address into two variables
										AddressLSB = FrameData[2];
										AddressMSB = FrameData[1];
										LastAddressLSB = AddressLSB;
										LastAddressMSB = AddressMSB;
										printf("Paired \r\n");
										// Activates lift fan
										StartLiftFan();									
										// Send back ACK with pair bit set
										Paired = true;
										KeyIdx = 0;
										//	Start toggling LEDS
										SendACK();
										// Start 45 sec Pairing timer
										ES_Timer_InitTimer(PAIRING_TIMER, PAIRING_TIME);
										// Start 1s transmit timeout timer
										ES_Timer_InitTimer(TX_TIMER, TX_TIME);
										// Change the current state to WAITING4ENCR
										CurrentState = WAITING4ENCR;
									}
								}
							}
						}
					break;
					case WAITING4ENCR:
						if (Paired == false) {
							SendACK();
							CurrentState = WAITING4PAIR;
						} else {
							if(FrameData[1] == AddressMSB && FrameData[2] == AddressLSB){
								// If we get an ENCR_KEY_HEADER in FrameData[5]
								if (FrameData[5] == ENCR_KEY_HEADER) {
									printf("ENCR Header received \r\n");
									// Save the encrption key locally 
									for (uint8_t index = 0; index < 32; index++) {
										Encryption_Key[index] = FrameData[6+index];
										//printf(" Current Key at index %u is %x \r\n", index, Encryption_Key[index]);
									}
									// Restart 1s transmit timeout timer
									ES_Timer_InitTimer(TX_TIMER, TX_TIME);
									// Transmit Status with pairing bit set
									//Paired = true;
									SendACK();
									//	Change the current state to WAITING4CTL
									CurrentState = WAITING4CTL;
								}
							}
						}
					break;
					case WAITING4CTL:
						if (Paired == false) {
							SendACK();
							CurrentState = WAITING4PAIR;
						} else {
							// If the command is addresed to us
							if(FrameData[1] == AddressMSB && FrameData[2] == AddressLSB){	
								// restart XMit timeout
								ES_Timer_InitTimer(TX_TIMER, TX_TIME);
								// Set the Encrypted CheckSum
								EncryptedCheckSum = FrameData[FRAME_OVERHEAD + 4];
								// If the Command frame is not repeated
								if (RepeatedFrame() == false) {
									//Decrypt
									Decrypt();
									//printf("Decrypting\r\n");
									// If we get a CTRL_HEADER
									if (DecryptedFrameData[0] == CTRL_HEADER) {
										uint8_t ChkSum = 0;
										// Sum byte 0, 1, 2, and 3 from control byte
										for (uint8_t index = 0; index < 4; index++) {
											ChkSum += DecryptedFrameData[index];
										}
										// If the sum matches the chksum byte
										if (ChkSum == DecryptedFrameData[4]) {
											//TestCount++;
										//	printf("Control count is %u \r\n", TestCount);
											// Transmit status with pairing bit set
											SendACK();
											// Process CTL command
											ProcessCTL();
										// Else 
										} else {
											// Post DECRYPTION_ERROR event
											ES_Event Event2Post;
											Event2Post.EventType = DECRYPTION_ERROR;
											PostProcessReceiveService(Event2Post);
										}
									}
								} else {
									SendACK();
								}
								// Save Last Command Frame
								for (uint8_t i = 0; i<sizeof(LastCommandFrame); i++) {
									LastCommandFrame[i] = FrameData[i + FRAME_OVERHEAD];
								}
							} else {
								printf("Diff Bot\r\n");
							}
						}
				break;
			} 
		}  
	} 
	ReturnEvent.EventType = ES_NO_EVENT;
	return ReturnEvent;
}
// Copies the last data that was received into the FrameData
void CopyFrameData (uint8_t* ReceivedData, uint8_t PacketSize) {
	memcpy(FrameData, ReceivedData, PacketSize);
}

static void SendACK(void) {	
	//set first byte in response to status header
	Bytes2Send[0] = STATUS_HEADER;
	
	//if we are paired, set bit 0 in second byte
	//clear bit 0 if we are not paired
	if(Paired){
		Bytes2Send[1] |= BIT0HI;
	}else{
		Bytes2Send[1] &= BIT0LO;
	}
	
	//if decryption error bit is set, set bit 1 in second byte
	//clear bit 1 if decryption error bit is cleared
	if(Decrypt_Error){
		Bytes2Send[1] |= BIT1HI;
	}else{
		Bytes2Send[1] &= BIT1LO;
	}
	Bytes2Send[2] = EncryptedCheckSum;
	SetDataByte(Bytes2Send, 3);
	PrepareDataPacket(AddressMSB, AddressLSB);
	SendFirstByte();
}

static void Decrypt(){
	//get the length of the data 
	uint8_t DataLength = CTRL_LENGTH;
	//iterate over the frame data, decrypt bytes, and save into new frame data array
	for(uint8_t i = 0;i < DataLength;i ++){
		DecryptedFrameData[i] = FrameData[i + FRAME_OVERHEAD] ^ Encryption_Key[(KeyIdx++ & KEY_ROLLOVER)];
		
		//rollover encryption index at 32
		//KeyIdx = KeyIdx & KEY_ROLLOVER;
	}	
}

static bool RepeatedFrame(void) {
	for (uint8_t i=0; i < sizeof(LastCommandFrame); i++) {
		if (LastCommandFrame[i] != FrameData[i + FRAME_OVERHEAD]) {
			return false;
		}
	}
	return true;
}

static void ProcessCTL() {
	//printf("Hex X value is %x \r\n", DecryptedFrameData[1]);
	int8_t Y_Pos_Int =  (int8_t) DecryptedFrameData[1];
	int8_t X_Pos_Int = (int8_t) DecryptedFrameData[2];
	float Y_Pos = (float) Y_Pos_Int;
	float X_Pos = (float) X_Pos_Int;
	printf("X position is %f and Y position is %f \r\n", X_Pos, Y_Pos);
	//printf("X Position: %f\t",X_Pos);
	//printf("Y Position: %f\r\n",Y_Pos);
	ProcessPositions(X_Pos, Y_Pos);

	// Check if breaking is engaged
	//printf(" DecryptedFrameData[3] is %x\r\n", DecryptedFrameData[3]);
	if ((DecryptedFrameData[3] & BREAK_MASK) == BREAK_MASK) {
		StopProps();
	}
	if ((DecryptedFrameData[3] & UNPAIR_MASK) == UNPAIR_MASK) {
		//printf("UNPAIR MASK");
		Paired = false;
		ClearDMCPaired();
	}
	
}

