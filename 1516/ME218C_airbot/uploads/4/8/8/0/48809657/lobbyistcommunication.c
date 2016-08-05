/*----------------------------- Include Files -----------------------------*/
/* include header files for this state machine as well as any machines at the
   next lower level in the hierarchy that are sub-machines to this machine
*/
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "uartstdio.h"
#include "LobbyistCommunication.h"

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

#include "PWMModule.h"
#include "ProcessReceive.h"
#include "LiftFanModule.h"
#include "PropModule.h"
#include "DMCModule.h"

/*----------------------------- Module Defines ----------------------------*/
// these times assume a 1.000mS/tick timing
#define ONE_SEC 976
#define ALL_BITS (0xff<<2)

#define RX_LINE GPIO_PIN_0
#define RX_LINE_HI BIT0HI
#define RX_LINE_LO BIT0LO

#define TX_LINE GPIO_PIN_1
#define TX_LINE_HI BIT1HI
#define TX_LINE_LO BIT1LO


#define BRDI  0x104			//260
#define	BRDF  0x1B 			//27

#define MAX_DATA_SIZE 33
#define FRAME_OVERHEAD 5

#define STALL_TIME	ONE_SEC / 5

#define ENCR_KEY_HEADER 0x01
#define CTRL_HEADER 0x02
#define STATUS_HEADER 0x03

// API Identifiers
#define	API_TX_STATUS 0x89

#define BLUE_TEAM BIT5HI 
#define	RED_TEAM BIT5LO 

/*---------------------------- Module Functions ---------------------------*/
static void InitUart(void);
static void PrintReceived(void);

/*---------------------------- Module Variables ---------------------------*/
static uint8_t MyPriority;
//static uint8_t TestByte[] = {0xAA, 0x4B, 0xFF, 0x3D, 0x55, 0x00, 0x09, 0xC2};  //Random bytes for now
static uint8_t SendIndex;
static uint8_t ReceivedPacket[MAX_DATA_SIZE + FRAME_OVERHEAD]; //set up empty array for received bytes
static uint8_t ReceiveIndex;
static LobbyistComState_t CurrentState;
static uint8_t BytesLeftToReceive;
static uint8_t TotalBytes;
static uint8_t CheckSumReceive;
static uint8_t PacketToSend[MAX_DATA_SIZE + FRAME_OVERHEAD];	
static uint8_t FrameID;
static uint8_t DataByte[MAX_DATA_SIZE + FRAME_OVERHEAD];
static uint8_t SizeOfData;

/*------------------------------ Module Code ------------------------------*/

bool InitLobbyistComService(uint8_t Priority) {
		MyPriority = Priority;
		ES_Event ThisEvent;
		ThisEvent.EventType = ES_INIT;
		SendIndex = 0;
		InitUart();
		//	Enable receive mask interrupt
		HWREG(UART1_BASE+UART_O_IM) |=  UART_IM_RXIM;
		//initialize state
		CurrentState = WAITING47E;
		FrameID = 0x00;
		InitPWMModule();
		if (ES_PostToService (MyPriority, ThisEvent) == true) {
			return true;
		} else {
			return false;
		}
}

// Initializes the Uart
static void InitUart(void) {
	// Enable UART module 1 using the RCGCUART register
	HWREG(SYSCTL_RCGCUART) |= SYSCTL_RCGCUART_R1;
	// Wait for the UART to be ready (PRUART)
	while ((HWREG(SYSCTL_PRUART ) & SYSCTL_PRUART_R1) != SYSCTL_PRUART_R1);
	// Enable the clock to the appropriate GPIO module via the RCGCGPIO
	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	// Wait for the GPIO module to be ready (PRGPIO)
	while ((HWREG(SYSCTL_RCGCUART) & SYSCTL_RCGCGPIO_R1) != SYSCTL_RCGCGPIO_R1);
	
	
	// Configure the GPIO pins for in/out/drive-level/drive-type
	//if ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R1) != SYSCTL_PRGPIO_R1) {
	//	HWREG(SYSCTL_RCGCGPIO) |= SYSCTL_RCGCGPIO_R1;
	//	while ((HWREG(SYSCTL_RCGCGPIO) & SYSCTL_RCGCGPIO_R1) != SYSCTL_PRGPIO_R1);
	//}
	//Set Pins 0 and 1 as digital pins on port B
	HWREG(GPIO_PORTB_BASE+GPIO_O_DEN) |= (RX_LINE | TX_LINE);
	//Set receive line as input
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) &= RX_LINE_LO;
	//Set transmit line as output
	HWREG(GPIO_PORTB_BASE+GPIO_O_DIR) |= TX_LINE;
	//Set transmits line as low
	HWREG(GPIO_PORTB_BASE+(GPIO_O_DATA + ALL_BITS)) &= TX_LINE_LO;
	
	
	// Select the Alternate function for the UART pins (AFSEL)
	HWREG(GPIO_PORTB_BASE+GPIO_O_AFSEL) |= (BIT0HI | BIT1HI);
	// Configure the PMCn fields in the GPIOPCTL register to assign the UART pins (PA0 for recieve, PA1 for xmit, mux is 1)
	HWREG(GPIO_PORTB_BASE+GPIO_O_PCTL) = (HWREG(GPIO_PORTA_BASE+GPIO_O_PCTL) & 0xffffff00) + (1<<0) + (1<<4);
	// Disable the UART by clearing the UARTEN bit in the UARTCTL register.
	HWREG(UART1_BASE+UART_O_CTL) &=  ~UART_CTL_UARTEN;
	// Write the integer portion of the BRD to the UARTIBRD register.
	HWREG(UART1_BASE+UART_O_IBRD) |= BRDI;
	// Write the fractional portion of the BRD to the UARTFBRD register.
	HWREG(UART1_BASE+UART_O_FBRD) |= BRDF;
	
	// Write the desired serial parameters to the UARTLCRH register. 
	//Set in 8-bit mode
	HWREG(UART1_BASE+UART_O_LCRH) |= (UART_LCRH_WLEN_8 );
	
	// Configure the UART operation using the UARTCTL register.
	// Enable Transmit and Receive, put in EOT mode
	HWREG(UART1_BASE+UART_O_CTL) |= (UART_CTL_RXE | UART_CTL_TXE | UART_CTL_EOT);					

	// Enable the UART by setting the UARTEN bit in the UARTCTL register.
	HWREG(UART1_BASE+UART_O_CTL) |=  UART_CTL_UARTEN;
	
	// Enable the interrupt in the NVIC
	HWREG(NVIC_EN0) |= BIT6HI;
	// Enable global interrupts
	__enable_irq();
	printf("Succesfully Initialized UART\r\n");
}

bool PostLobbyistComService( ES_Event ThisEvent ){
	  //Post Event to queue
	return ES_PostToService( MyPriority, ThisEvent);
}

ES_Event RunLobbyistComService( ES_Event ThisEvent ){
	if (ThisEvent.EventType == ES_NEW_KEY) {
		if (ThisEvent.EventParam == 's') {
			printf("s key pressed\r\n");
			uint8_t Byte[] = {0xAA, 0x55, 0xFF};
			//uint8_t Byte[] = {0x00, 0x83}; 
			SizeOfData = sizeof(Byte);
			memset(DataByte, 0, sizeof(DataByte));
			for (uint8_t index=0; index < SizeOfData; index++) {
				DataByte[index] = Byte[index];
			}
			PrepareDataPacket(0x20, 0x81);
			SendFirstByte();
		}
		if (ThisEvent.EventParam == 'x'){
			printf("r key pressed\r\n");
			PrintReceived();
		}
		if (ThisEvent.EventParam == 'h'){
			//uint8_t Byte[] = {0x7E, 0x00, 0x07, 0x81, 0x21, 0x81, 0x28, 0x00, 0x00, 0x83, };
	
		}
		if (ThisEvent.EventParam == 'q') {
			printf("Stopped\r\n");
			SetDutyLeft(1, FORWARD);
			SetDutyRight(1, FORWARD);
		}
		if (ThisEvent.EventParam == 'f') {
			printf("Forward\r\n");
			GoForward(90);
		}
		if (ThisEvent.EventParam == 'b') {
			printf("Reverse\r\n");
			GoBackward(90);
		}
		if(ThisEvent.EventParam == 'l'){
			printf("Left Turn\r\n");
		//	TurnLeft(100);
			TurnLeft(90, 90, REVERSE, FORWARD);
		}
		if(ThisEvent.EventParam == 'r'){
			printf("Right Turn\r\n");
			TurnRight(90, 90, FORWARD, REVERSE);
		}
		if(ThisEvent.EventParam == 'm'){
			printf("Left Motor forward\r\n");
			SetDutyLeft(95, FORWARD);
		}
		if(ThisEvent.EventParam == 'n'){
			printf("Right Motor forward\r\n");
			SetDutyRight(95, FORWARD);
		}
		if(ThisEvent.EventParam == 'u'){
			printf("Lift Fan On\r\n");
			//StartLiftFan();
			SetDutyLift(50);
		}
		if(ThisEvent.EventParam == 'd'){
			printf("Lift Fan Off\r\n");
			//StopLiftFan();
			SetDutyLift(1);
		}
		if(ThisEvent.EventParam == '1'){
			printf("Blue team chosen\r\n");
			SetDMCPaired();
			SetDMCColor(1);
		}
		if(ThisEvent.EventParam == '0'){
			printf("Red team chosen\r\n");
			SetDMCPaired();
			SetDMCColor(0);
		}
	} else if (ThisEvent.EventType == ERROR_MESSAGE) {
		printf("Bad Check Sum\r\n");
	}
	//	if we are in Waiting47E state
	if(CurrentState == WAITING47E){
			//if a byte was recieved
			if(ThisEvent.EventType == BYTE_RECEIVED){
				//if 7E is received
				if(ThisEvent.EventParam == 0x7E){
					//	Reset receive storage array
					memset(ReceivedPacket, 0, sizeof(ReceivedPacket));
					CheckSumReceive = 0;
					//Go to Waiting4MSB state
					ES_Timer_InitTimer(STALL_TIMER, STALL_TIME);
					CurrentState = WAITING4MSB;
				}
			}
	}else if(CurrentState == WAITING4MSB){
			//if StallTimer times out
			if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == STALL_TIMER){
				//Go to waiting47E state
				CurrentState = WAITING47E;
			//Else if a byte was received
			}else if(ThisEvent.EventType == BYTE_RECEIVED){
				//if 0x00 is received for MSB
				if(ThisEvent.EventParam == 0x00){
					ES_Timer_InitTimer(STALL_TIMER, STALL_TIME);
					//Go to waiting4LSB state
					CurrentState = WAITING4LSB;
				}else{
					//Go to waiting47E state
					CurrentState = WAITING47E;
				}
			}
	}else if(CurrentState == WAITING4LSB){
		//if StallTimer times out
		if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == STALL_TIMER){
				//Go to waiting47E state
			CurrentState = WAITING47E;
		}else if(ThisEvent.EventType == BYTE_RECEIVED){
			ES_Timer_InitTimer(STALL_TIMER, STALL_TIME);
			//Set BytesLeftToReceive
			BytesLeftToReceive = ThisEvent.EventParam; //check this 
			TotalBytes = ThisEvent.EventParam;
			//Go to SuckUpPacket state
			CurrentState = SUCKUPPACKET;
		}
	} else if(CurrentState == SUCKUPPACKET) {
		//if StallTimer times out
		if(ThisEvent.EventType == ES_TIMEOUT && ThisEvent.EventParam == STALL_TIMER) {
				//Go to waiting47E state
				CurrentState = WAITING47E;
		//Else if a byte was received
		}else if(ThisEvent.EventType == BYTE_RECEIVED) {
			ES_Timer_InitTimer(STALL_TIMER, STALL_TIME);
			//If BytesLefttoReceive is not equal to 0
			if(BytesLeftToReceive != 0) {
				//Store the receive byte into array[index]
				ReceivedPacket[TotalBytes - BytesLeftToReceive] = HWREG(UART1_BASE+UART_O_DR);
				//Decrement BytesLeftToReceive
				BytesLeftToReceive--;
				//Add byte that was received to CheckSum
				CheckSumReceive += HWREG(UART1_BASE+UART_O_DR);
			//else
			} else{
				//ReceivedPacket[TotalBytes+1] = HWREG(UART1_BASE+UART_O_DR);
				//Check the checksum
				CheckSumReceive = 0xFF - CheckSumReceive;
				//printf("CheckSum is %x \r\n", CheckSumReceive);
				ES_Event Event2Post;
				//If checksum is good
				if (HWREG(UART1_BASE+UART_O_DR) == CheckSumReceive) { 
					ES_Timer_StopTimer(STALL_TIMER);
					// Copy frame data to process
					CopyFrameData(ReceivedPacket, sizeof(ReceivedPacket));
					//Go to waiting47E
					CurrentState = WAITING47E;					
					//post MessageGood event
					Event2Post.EventType = GOOD_MESSAGE;
					ES_PostAll(Event2Post);							
					//	PostLobbyistComService(Event2Post);
				// Else
				} else{
					//Go to waiting47E
					CurrentState = WAITING47E;
					//Post error message
					Event2Post.EventType = ERROR_MESSAGE;
				}
			}
		}
	}
	ES_Event ReturnVal;
	ReturnVal.EventType = ES_NO_EVENT;
	return ReturnVal;
}

void SendFirstByte(void){
	// If txfe is set (room to transmit a byte)
	if ((HWREG(UART1_BASE+UART_O_FR) & UART_FR_TXFE) == UART_FR_TXFE) {
		// Write the new data to register (UARTDR)
		HWREG(UART1_BASE+UART_O_DR) = PacketToSend[SendIndex];
		SendIndex++;
		//	Enable TXIM
		HWREG(UART1_BASE+UART_O_IM) |=  UART_IM_TXIM;
		//return success
	}//Else
		//return error
}

static void PrintReceived(void){
	//loop over array and print bytes
	for(uint8_t i = 0;i < sizeof(ReceivedPacket); i ++){
		printf("Byte #%u: %x\r\n",i,ReceivedPacket[i]);
	}
}


//	Sets up the data packet that will be send 
void PrepareDataPacket(uint8_t MSB, uint8_t LSB) {
 	uint8_t CheckSum = 0;
	// Put 0x7E into the packet to send
	PacketToSend[0] = 0x7E;
	// Put 0x00 (MSB of length) into the packet to send
	PacketToSend[1] = 0x00;
	// Put length of paket (LSB of length) into the packet to send
	PacketToSend[2] = SizeOfData + 5;				
	// Put 0x01 (API) into packet to send
	PacketToSend[3] = 0x01;																										
	CheckSum += PacketToSend[3];
	// Put frame ID into XBeePacket and add to checksum
	PacketToSend[4] = FrameID;
	//FrameID++; 																
	//if (FrameID == 0) {
//		FrameID = 0x01;
	//}
	CheckSum += PacketToSend[4];
	// Put Address MSB into paket to send and add to check sum
	PacketToSend[5] = MSB;
	CheckSum += PacketToSend[5];
	// Put Address LSB into packet to send and add to check sum
		PacketToSend[6] = LSB;
	CheckSum += PacketToSend[6];
	// Put options byte (0x00) into packet to send and add to check sum
	PacketToSend[7] = 0x00;
	CheckSum += PacketToSend[7];
	// for 0 to size of Data Byte
	for (uint8_t index = 0; index < SizeOfData; index++) {
			// Put DataByte[index] into packet to send and add to checksum
		PacketToSend[index+8] = DataByte[index];
		CheckSum += PacketToSend[index+8];
	}
	CheckSum = 0xFF - CheckSum;
	PacketToSend[SizeOfData + 8] = CheckSum;
	// Put check sum into packet to send
}

void SetDataByte(uint8_t* SendData, uint8_t PacketSize){
	
	//clear databyte
	memset(DataByte,0, sizeof(DataByte));
	
	//Copy SendData to DataByte
	memcpy(DataByte, SendData, PacketSize);
	
	//Set DataSize to PacketSize
	SizeOfData = PacketSize;
	//set local data bytes to the passed parameter
//	DataByte[0] = bytes[0];
//	DataByte[1] = bytes[1];
//	SizeOfData = 2;
	

}


void EOTResponse(void) {
		
		//	TRANSMIT PROCESS
		// Read the Masked Interupt status (UARTMIS)
		// If TXMIS is set
		if ((HWREG(UART1_BASE+UART_O_MIS) & UART_MIS_TXMIS) == UART_MIS_TXMIS) {
			// Set TXIC in UARTICR (clear int)
			HWREG(UART1_BASE+UART_O_ICR) |= UART_ICR_TXIC;
			//printf("TX\r\n");	
			//	If this was the last byte in message block (used to say sizeof(PackageToSend)
			if (SendIndex >= (SizeOfData+9)) {
					//Disable TXIM
					HWREG(UART1_BASE+UART_O_IM) &=  ~UART_IM_TXIM;
					//	Reset Index
					SendIndex = 0;
			} else {
				// Write the new data to register (UARTDR)
				HWREG(UART1_BASE+UART_O_DR) = PacketToSend[SendIndex++];
				//SendIndex++;
			}
		} else {
		//You are done (not an TX interrupt)
			
		//	RECEIVE PROCESS
		// Read the Masked Interrupt Status (UARTMIS)
		// If RXMIS is set
		if ((HWREG(UART1_BASE+UART_O_MIS) & UART_MIS_RXMIS) == UART_MIS_RXMIS) {	
			// Read the data register (UARTDR)
			// If no error bits set
			//check for overrun error
			if((HWREG(UART1_BASE+UART_O_DR) & UART_DR_OE) == UART_DR_OE){
				printf("Overrun Error \r\n");
				
				//check for break error
			}else if((HWREG(UART1_BASE+UART_O_DR) & UART_DR_BE) == UART_DR_BE){
				printf("Break Error\r\n");
								
				//check for parity error 
			}else if((HWREG(UART1_BASE+UART_O_DR) & UART_DR_PE) == UART_DR_PE){
				printf("Parity Error\r\n");
								
				//check for framing error
			}else if((HWREG(UART1_BASE+UART_O_DR) & UART_DR_FE) == UART_DR_FE){
				printf("Framing Error\r\n");
				 
			}else{		
				//save new data byte and increment index
					//ReceivedByte[ReceiveIndex++] = HWREG(UART1_BASE+UART_O_DR);
					ES_Event Event2Post;
					Event2Post.EventType = BYTE_RECEIVED;
					Event2Post.EventParam = HWREG(UART1_BASE+UART_O_DR);
					PostLobbyistComService(Event2Post);
					//If recieve index is greater or equal to the size of the received byte array
					//if(ReceiveIndex >= sizeof(ReceivedByte)) {
						//Set receive index to 0
					//	ReceiveIndex = 0;
				//}
			}
		}
	}
}
