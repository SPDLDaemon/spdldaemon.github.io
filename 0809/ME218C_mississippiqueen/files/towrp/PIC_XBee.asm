;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PIC_XBee.asm
;; PIC Assembly code for the main PIC of ME218C's
;; Spring 09 quarter project.
;; Written by: Andrew Parra
;; SetVelocity and PWM written by David Hoffert
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    list 	P=PIC16F690
    #include 	"p16F690.inc"
    __config 	(_CP_OFF & _WDT_OFF & _PWRTE_ON & _HS_OSC & _MCLRE_OFF)

TEMP_1		equ	0x20
TEMP_2      equ 0x21
TX_DATA		equ 0x22
RX_DATA     equ 0x23
STATUS_TEMP	equ 0x24
W_TEMP		equ 0x25
DEST_ADDRESS_MSB    equ 0x26
DEST_ADDRESS_LSB    equ 0x27
RF_DATA_MSB equ 0x28
RF_DATA_LSB equ 0x29
CHECKSUM    equ 0x2A
PACKET_STATE	equ	0x2B
PACKET_ARRAY_I  equ 0x2C
LENGTH_OF_PACKET    equ 0x2D
RX_SOURCE_ADDRESS_MSB  equ 0x2E
RX_SOURCE_ADDRESS_LSB  equ 0x2F
TX_STATUS   equ 0x30
FWD_VEL_2 equ 0x31
ROT_VEL_2 equ 0x32
RX_SIGNAL_STRENGTH equ 0x40
RX_OPTIONS  equ 0x41
TX_SPI_DATA equ 0x42
RX_SPI_DATA equ 0x43
RX_OPCODE	equ	0x44
RX_PARAMETER	equ	0x45
COMM_SUPER_STATE	equ	0x46
COMM_STATE	equ	0x47
TEAM_COLOR	equ	0x48
MORALE	equ	0x49
BROADCAST	equ	0x4A
FWD_VELOCITY	equ	0x4B
ROT_VELOCITY	equ	0x4C
TIMER0_COUNTER	equ	0x4D
AVAILABLE_TIMER	equ	0x4E
VELOCITY_TIMEOUT_TIMER	equ	0x4F
MORALE_UPDATE_TIMER	equ	0x50
MUTINY_TIMER	equ	0x51
MUTINY_MESSAGE_TIMER	equ	0x52
VELOCITY_TIMER	equ	0x53
ADD_VELOCITY    equ 0x54
SUB_VELOCITY    equ 0x55
DUTY_CYCLE      equ 0x56
DUTY_CYCLE_2    equ 0x57
PACKET_ARRAY	equ	0x58

#define MY_ID   0xAF05
#define START_DELIMITER 0x7E
#define MSB_LENGTH  0x00
#define LSB_LENGTH  0x07
#define API_ID_TX   0x01
#define FRAME_ID    0x83
#define OPTIONS     0x01

#define SSPCON_CONFIG   b'00110010'         ;Bit 7: (0) No write collision detect bit
                                            ;Bit 6: (0) No Receive overflow bit
                                            ;Bit 5: (1) In SPI Mode
                                            ;Bit 4: (1) Idle state for clock is a low level
                                            ;Bit 3-0: (0010) SPI Master Mode, clock Fosc/64

#define SSPSTAT_CONFIG  b'00000000'         ;Bit 7: (0) Input data sampled at middle of data output time
                                            ;Bit 6: (0) Data transmitted rising edge of SCK

#define TRISB_CONFIG    b'00111111'         ;Bit 7: (0) Output Tx
                                            ;Bit 6: (0) Output SCK

#define TRISC_CONFIG    b'00000000'         ;Bit 7: (0) Output SDO
                                            ;Bit 6: (0) Output Not Used
                                            ;Bit 5: (0) Output PWM to servo
                                            ;Bit 4: (0) Output SS Right
                                            ;Bit 3: (0) Output SS Left
                                            ;Bit 2: (0) Output Kicker On
                                            ;Bit 1: (0) Output Red LED
                                            ;Bit 0: (0) Output Blue LED

#define TRISA_CONFIG    b'00000000'         ;Bit 5: (0) Output OSC1
                                            ;Bit 4: (0) Output OSC2

#define OPTION_REG_CONFIG	b'01000111'		;Bit 7: (0) Weak pull ups enabled on individual ports
											;Bit 6: (1) Intedge trigger on high
											;Bit 5: (0) Use the Fosc/4 for the TIMR0 source
											;Bit 4: (0) Transition on the low to high
											;Bit 3: (0) Prescalar assigned to TIMR0
											;Bit 2-0: (100) Use a divide by 32 prescalar

#define	T2CON_CONFIG	b'00000111'	; Bit 7 Unimplemented
					; Bits 6-3 TMR2 Postscaler (Unused for PWM)
					; Bit 2 TMR2 is on
					; Bits 1-0 TMR2 Pre-Scaler is 16
#define	CCP1CON_FWD	b'00001100'	; Bits 7-6 Single-Output PWM
					; Bits 5-4 We only need whole value duty cycles
					; Bits 3-0 Every channel active high
#define CCP1CON_REV	b'00001111'	; Bits 7-6 Single-Ouput PWM
					; Bits 5-4 We only need whole value duty cycles
					; Bits 3-0 Every channel active low
#define	CCPR1L_CONFIG	b'00000000'	; Initialize to a zero duty cycle
#define	PR2_CONFIG	b'11111001'	; PR2 = 249 --> 625 Hz PWM Period

#define RIGHT_SS        4
#define LEFT_SS         3

#define KICKER_MOTOR_BIT    b'00000100'

#define TIMER0_200MS	d'8'				;61*32*2^8/(2.5MHz) = 61
#define TIMER0_1S		d'5'				;5*200ms = 1second
#define TIMER0_2S       d'10'
#define TIMER0_3S		d'15'				;15*200ms = 3seconds											
#define TIMER0_5S		d'25'				;25*200ms = 5seconds
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; COMM STATE MACHINE #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;										
#define NOT_PAIRED 0x01
#define PAIRED 0x02
#define NORMAL 0x03
#define MUTINOUS 0x04
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; End COMM STATE MACHINE #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; COMM OPCODE #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
#define SUPPRESS_MUTINY	0x00
#define ASSERTION_OF_COMMAND	0x01
#define JETTISON_THE_CREW	0x02
#define VELOCITY	0x03
#define FREE_DIGITAL	0x04
#define FREE_ANALOG	0x05
#define MUTINY_IN_PROGRESS	0x20
#define I_AM_AVAILABLE	0x21
#define ASSERTION_OF_COMMAND_RESULT	0x22
#define UPDATE_MORALE	0x23
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; END COMM OPCODE #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TEST #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;#define TEST_WITH_PUSH_BUTTON
;#define TEST_SPI_COMM
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; END TEST #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

            org	0
            GOTO INIT
            org	4
            GOTO ISR

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; VELOCITY TABLE
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
VELOCITY_TABLE:
	ADDWF	PCL, F
	RETLW	d'0'    ;0000
	RETLW	d'15'   ;0001
	RETLW	d'29'   ;0010
	RETLW	d'45'   ;0011
	RETLW	d'59'   ;0100
	RETLW	d'75'   ;0101
	RETLW	d'89'   ;0110
	RETLW	d'100'  ;0111
	RETLW	d'0'    ;1000
	RETLW	0xF1    ;-15 in 2's complement (1001)
	RETLW	0xE3    ;-29
	RETLW	0xD3    ;-45
	RETLW	0xC5    ;-59
	RETLW	0xB5    ;-75
	RETLW	0xA7    ;-89
	RETLW	0x9C    ;-100
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; END VELOCITY TABLE
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Interrupt Response Routine
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
ISR:
    banksel PORTA
	MOVWF	W_TEMP
	SWAPF	STATUS, W
	MOVWF	STATUS_TEMP
ISR_BEGIN:
	banksel	INTCON
#ifdef TEST_WITH_PUSH_BUTTON
	BTFSC	INTCON, INTF	;The push button occured
	CALL	PUSH_BUTTON_OCCURED
#endif
	BTFSC	INTCON, T0IF	;A timer 0 overflow occured
	CALL	TIMER0_OVERFLOW
	banksel	PIR1
	BTFSC	PIR1, RCIF	;We received a new message
	CALL	HANDLE_MSG
END_ISR:
    banksel PIE1
    BSF     PIE1, RCIE          ;Should not need this, but hopefully it will fix our code
    banksel PORTA
    BSF     RCSTA, CREN
	SWAPF	STATUS_TEMP, W
	MOVWF	STATUS
	SWAPF	W_TEMP, F
	SWAPF	W_TEMP, W
	RETFIE

TIMER0_OVERFLOW:
    banksel PORTA
	INCF	TIMER0_COUNTER, F
	MOVLW	TIMER0_200MS			;With a prescalar of /32, we need to come in here 61 times before 200ms has occured
	SUBWF	TIMER0_COUNTER, W
	BTFSS	STATUS, Z
	GOTO	RETURN_FROM_TIMER0						;We arn't at 61 yet, no need to continue
	INCF	AVAILABLE_TIMER, F
	INCF	VELOCITY_TIMEOUT_TIMER, F
	INCF	MORALE_UPDATE_TIMER, F
	INCF	MUTINY_TIMER, F
	INCF	MUTINY_MESSAGE_TIMER, F
	INCF	VELOCITY_TIMER, F
	MOVLW	NOT_PAIRED			;Check what state we are currently in
	SUBWF	COMM_SUPER_STATE, W
	BTFSC	STATUS, Z
	GOTO	TIMER0_NOT_PAIRED	;If we are not paired, still check the timer event
	GOTO	TIMER0_PAIRED				;Otherwise we are paired
RETURN_FROM_HANDLE_TIMER0:
    CLRF    TIMER0_COUNTER      ;Reset the timer 0 counter
    BCF		INTCON, T0IF		;Clear the timer 0 interrupt flag
    CLRF    TMR0
    RETURN

RETURN_FROM_TIMER0:
    banksel TMR0
    BCF     INTCON, T0IF        ;But we still need to clear the interrupt
    CLRF    TMR0
    RETURN

TIMER0_NOT_PAIRED:
    banksel PORTA
    BSF     PORTC, 2                    ;Turn off the kicker motor, in case it is on.
    NOP
    BCF     PORTC, 0
    NOP
    BCF     PORTC, 1                    ;Clear both of the lights, just in case as well
    NOP
	MOVLW	TIMER0_1S
	SUBWF	AVAILABLE_TIMER, W			;Check to see if the available timer has expired
	BTFSS	STATUS, Z
	GOTO	RETURN_FROM_HANDLE_TIMER0	;If the available timer has not expired, then return
	MOVLW	I_AM_AVAILABLE
	MOVWF	RF_DATA_MSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
    MOVLW   0x00
    MOVWF   RF_DATA_LSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	BSF		BROADCAST, 0
	CALL	SEND_MSG
	BCF		BROADCAST, 0
	CLRF	AVAILABLE_TIMER				;Reset the available timer
	GOTO	RETURN_FROM_HANDLE_TIMER0

TIMER0_PAIRED:
    banksel PORTA
	MOVLW	TIMER0_5S			;First things first, check if the velocity timeout occured
	SUBWF	VELOCITY_TIMEOUT_TIMER, W
	BTFSC	STATUS, Z
	GOTO	HANDLE_PAIRED_VELOCITY_TIMER_EXP
	MOVLW	NORMAL				;Find out what sub-state we are in
	SUBWF	COMM_STATE, W
	BTFSC	STATUS, Z
	GOTO	CHECK_NORMAL_TIMER0
CHECK_MUTINOUS_TIMER0:
    banksel PORTA
	MOVLW	TIMER0_3S			;Check if the 3 seconds has expired when we were mutinous
	SUBWF	MUTINY_TIMER, W
	BTFSC	STATUS, Z
	GOTO	HANDLE_MUTINY_TIMER_EXPIRED
	MOVLW	MUTINY_IN_PROGRESS	;If we are here, it means .2seconds has gone by in the mutinous stage.
	MOVWF	RF_DATA_MSB			;Send a mutiny in progress
	CALL	SEND_MSG
	CLRF	MUTINY_MESSAGE_TIMER	;Reset the timer (although, not necessary, good for track keeping)
	GOTO	RETURN_FROM_HANDLE_TIMER0

HANDLE_PAIRED_VELOCITY_TIMER_EXP:
    banksel PORTA
	MOVLW	I_AM_AVAILABLE		;Send an I am available command
	MOVWF	RF_DATA_MSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
    MOVLW   0x01
    MOVWF   RF_DATA_LSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	BSF		BROADCAST, 0
	CALL	SEND_MSG
	BCF		BROADCAST, 0
	CLRF	AVAILABLE_TIMER		;Clear the available timer
	MOVLW	NOT_PAIRED
	MOVWF	COMM_SUPER_STATE
	GOTO	RETURN_FROM_HANDLE_TIMER0
	
HANDLE_MUTINY_TIMER_EXPIRED:
    banksel PORTA
	MOVLW	I_AM_AVAILABLE		;The mutiny timer expired
	MOVWF	RF_DATA_MSB			;Broadcast an I am available
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
    MOVLW   0x02
    MOVWF   RF_DATA_LSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	BSF		BROADCAST, 0
	CALL	SEND_MSG
	BCF		BROADCAST, 0
	CLRF	AVAILABLE_TIMER		;Reset the available timer
	MOVLW	NOT_PAIRED
	MOVWF	COMM_SUPER_STATE
	GOTO	RETURN_FROM_HANDLE_TIMER0	;If we just mutinied, we don't want to send out the mutiny in progress, so just call it quits for now

CHECK_NORMAL_TIMER0:
    banksel PORTA
	MOVLW	TIMER0_1S
	SUBWF	VELOCITY_TIMER, W			;Check the velocity timer
	BTFSS	STATUS, Z
	GOTO	CHECK_MORALE_UPDATE_TIMER	;If it hasn't expired, go to the next timer to check
	CLRF	FWD_VELOCITY			;If it has expired, reset the velocities
	CLRF	ROT_VELOCITY
    CALL    SET_VELOCITY
    CLRF    VELOCITY_TIMER
CHECK_MORALE_UPDATE_TIMER:
    banksel PORTA
	MOVLW	TIMER0_1S
	SUBWF	MORALE_UPDATE_TIMER, W		;Check if the morale update timer has expired
	BTFSS	STATUS, Z
	GOTO	RETURN_FROM_HANDLE_TIMER0

    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    BTFSS   FWD_VELOCITY, 7             ;Test to see if the forward velocity is negative
    GOTO    NOT_NEGATIVE_FWD
    COMF    FWD_VELOCITY, W             ;If it is, invert and add 1, ie twos complement
    ADDLW   0x01
    MOVWF   TEMP_1                      
    GOTO    DONE_WITH_FWD_VELOCITY
NOT_NEGATIVE_FWD:
    MOVF    FWD_VELOCITY, W
    MOVWF   TEMP_1
DONE_WITH_FWD_VELOCITY:                 ;TEMP_1 contains Abs(Forward velocity)
    
    BTFSS   ROT_VELOCITY, 7             ;Test to see if rotational velocity is negative
    GOTO    NOT_NEGATIVE_ROT
    COMF    ROT_VELOCITY, W             ;If it is, invert and add 1, ie twos complement
    ADDLW   0x01                     
    GOTO    DONE_WITH_ROT_VELOCITY
NOT_NEGATIVE_ROT:
    MOVF    ROT_VELOCITY, W
DONE_WITH_ROT_VELOCITY:                 ;W contains Abs(Rotational velocity)
    MOVWF   TEMP_2                      ;Store rotatonial in TEMP_2 just in case we end up using it
    SUBWF   TEMP_1, W
    BTFSC   STATUS, C                   ;Check if we are negative
    GOTO    CONTINUE_UPDATE_MORAL       ;Forward is greater, TEMP_1 already contains forward
    MOVF    TEMP_2, W                      ;Rotational is greater, TEMP_2 contains Abs(rotational)
    MOVWF   TEMP_1
    
    
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
CONTINUE_UPDATE_MORAL:
	MOVLW	d'30'
	SUBWF	TEMP_1, W				;Check if we are 0-30
	BTFSS	STATUS, C
	GOTO	INC_1_MORALE
	MOVLW	d'60'
	SUBWF	TEMP_1, W				;Check if we are 30-60
	BTFSS	STATUS, C
	GOTO	FINISHED_UPDATING_MORALE
	MOVLW	d'90'
	SUBWF	TEMP_1, W				;Check if we are 60-90
	BTFSS	STATUS, C
	GOTO	DEC_1_MORALE
	GOTO	DEC_2_MORALE			;Otherwise, we are >90
FINISHED_UPDATING_MORALE:
    banksel PORTA
	CLRF	MORALE_UPDATE_TIMER
	MOVLW	d'10'					;Check if the morale, after updating, is now at -10
	ADDWF	MORALE, W
	BTFSC	STATUS, Z
	GOTO	HANDLE_MORALE_NEG_TEN
	GOTO	RETURN_FROM_HANDLE_TIMER0

HANDLE_MORALE_NEG_TEN:				;If the morale is at -10, perform the proper actions
    banksel PORTA
	CLRF	MUTINY_TIMER			;Reset the mutiny timer
	CLRF	MUTINY_MESSAGE_TIMER	;Reset the mutiny message timer
	CLRF	FWD_VELOCITY			;Reset the velocities
	CLRF	ROT_VELOCITY
    CALL    SET_VELOCITY
	MOVLW	MUTINOUS				;And switch to the mutinous sub state
	MOVWF	COMM_STATE
	GOTO	RETURN_FROM_HANDLE_TIMER0
	
DEC_2_MORALE:
    banksel PORTA
	DECF	MORALE, F
	DECF	MORALE, F
    MOVLW   d'11'
    ADDWF   MORALE, W
    BTFSS   STATUS, Z
	GOTO	FINISHED_UPDATING_MORALE
    MOVLW   0xF6
    MOVWF   MORALE
	GOTO	FINISHED_UPDATING_MORALE

DEC_1_MORALE:
    banksel PORTA
	DECF	MORALE, F
    GOTO    FINISHED_UPDATING_MORALE

INC_1_MORALE:
    banksel PORTA
	INCF	MORALE, F
    MOVLW   d'11'
    SUBWF   MORALE, W
    BTFSS   STATUS, Z
    GOTO	FINISHED_UPDATING_MORALE
    MOVLW   d'10'
    MOVWF   MORALE
    GOTO    FINISHED_UPDATING_MORALE
	
#ifdef TEST_WITH_PUSH_BUTTON
PUSH_BUTTON_OCCURED:
#ifdef TEST_STU_COMM
    CLRF    RF_DATA_MSB
    MOVLW   b'00000011'
    ANDWF   PORTA, W
    MOVWF   RF_DATA_LSB
    CALL    SEND_MSG
#endif
#ifdef TEST_SPI_COMM
    MOVLW   b'00000011'
    ANDWF   PORTA, W
    MOVWF   TX_SPI_DATA
    CALL    TRANSMIT_SPI
#endif
    BCF     INTCON, INTF    ;Must clear this flag manually
    RETURN
#endif

SEND_MSG:
    banksel PORTA
    CLRF    CHECKSUM
    banksel TXREG
    MOVLW   START_DELIMITER
	MOVWF	TXREG		;Send out the byte!
    CALL    WAIT_TO_SEND
    MOVLW   MSB_LENGTH
	MOVWF	TXREG		;Send out the byte!
    CALL    WAIT_TO_SEND
    MOVLW   LSB_LENGTH
	MOVWF	TXREG		;Send out the byte!
    CALL    WAIT_TO_SEND
    MOVLW   API_ID_TX
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
    MOVLW   FRAME_ID
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
    MOVF    DEST_ADDRESS_MSB, W
	BTFSC	BROADCAST, 0		;Check if this is a broadcast message, if it is, change the destination address
	MOVLW	0xFF
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
    MOVF    DEST_ADDRESS_LSB, W
	BTFSC	BROADCAST, 0		;Check if this is a broadcast message, if it is, change the destination address
	MOVLW	0xFF
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
    MOVLW   OPTIONS
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
    MOVF    RF_DATA_MSB, W
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
    MOVF    RF_DATA_LSB, W
	MOVWF	TXREG		;Send out the byte!
    ADDWF   CHECKSUM, F
    CALL    WAIT_TO_SEND
	MOVF    CHECKSUM, W
    SUBLW   0xFF
    MOVWF   TXREG
    CALL    WAIT_TO_SEND
    RETURN

WAIT_TO_SEND:
    banksel TXSTA
    BTFSC   TXSTA, TRMT         ;Wait until we are ready to send again.
    GOTO    END_WAIT_TO_SEND
    GOTO    WAIT_TO_SEND

END_WAIT_TO_SEND:
    banksel TXREG
    RETURN

HANDLE_MSG:
    banksel RCREG
	MOVF	RCREG, W
	MOVWF	RX_DATA     ;Save the received data
    pageselw    SWITCH_DECISION
    MOVF    PACKET_STATE, W
    CALL    SWITCH_DECISION
END_HANDLE_MSG:
    RETURN

SWITCH_DECISION:
    ADDWF   PCL, F
    GOTO    STATE_PACKET_START
    GOTO    STATE_LENGTH_MSB
    GOTO    STATE_LENGTH_LSB
    GOTO    STATE_GET_DATA
    GOTO    STATE_CHECK_SUM

STATE_PACKET_START:
    banksel PORTA
    MOVLW   0x7E
    SUBWF   RX_DATA, W     ;Check if it is in fact a 7E
    BTFSC   STATUS, Z
    INCF    PACKET_STATE, F    ;If it is a 7E, move to the next state, checking the MSB
    RETURN                  ;Else, keep the state here, and return.
STATE_LENGTH_MSB:
    banksel PORTA
    MOVF    RX_DATA, W
    BTFSS   STATUS, Z
    GOTO    BAD_CHECKSUM        ;If it isn't 0, reset the packset state machine
    INCF    PACKET_STATE, F    ;Nothing exciting here, just increment the state.
    RETURN
STATE_LENGTH_LSB:
    banksel PORTA
    CLRF    PACKET_ARRAY_I  ;Set i=0
    CLRF    LENGTH_OF_PACKET  ;Set j=0
    MOVF    RX_DATA, W
    ADDWF   PACKET_ARRAY_I, F   ;Set i=Length_LSB
    ADDWF   LENGTH_OF_PACKET, F   ;Set j=Length_LSB
    SUBLW   d'7'
    BTFSS   STATUS, C           ;If we have length > 7, get rid of it
    GOTO    BAD_CHECKSUM
    INCF    PACKET_STATE, F    ;Increment the state
    RETURN
STATE_GET_DATA:
    banksel PORTA
    MOVLW   PACKET_ARRAY       ;Get the memory location of the packet array
    ADDWF   PACKET_ARRAY_I, W  ;Find out where in the array to put the new item
    MOVWF   FSR
    MOVF    RX_DATA, W          ;Get the current data
    MOVWF   INDF                ;Store it in the array
    DECF    PACKET_ARRAY_I, F      ;Decrement our current i placement
    BTFSC   STATUS, Z           ;If we are at the end of i
    INCF    PACKET_STATE, F        ;Go ahead and move onto the next state, we have all of our data
    RETURN
STATE_CHECK_SUM:
    banksel PORTA
    MOVF    LENGTH_OF_PACKET, W   ;Store the Length_LSB into a temporary value
    MOVWF   TEMP_1
    CLRF    TEMP_2              ;TEMP_2 will store the sum of all of the bytes
ADD_ARRAY_VALUE:
    banksel PORTA
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
    ADDWF   TEMP_2, F           ;Add it to our list of current bytes summed
    DECF    TEMP_1, F           ;Check to see if we are at the end of the array
    BTFSS   STATUS, Z           ;If we arn't, keep adding to the file
    GOTO    ADD_ARRAY_VALUE
    MOVF    TEMP_2, W           ;At this point, we have a summation stored in TEMP_2
    SUBLW   0xFF                ;0xFF-TEMP_2.  At this point, W contains the checksum
    SUBWF   RX_DATA, W          ;Check to see if the checksums are the same
    BTFSS   STATUS, Z
    GOTO    BAD_CHECKSUM        ;If the checksums are not the same, we are going to have to reset the state machine and start from scratch
EVALUATE_DATA:
    banksel PORTA
    MOVF    LENGTH_OF_PACKET, W ;Store the Length_LSB into a temporary value
    MOVWF   TEMP_1
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
    SUBLW   0x81                ;Check to see if it a received packet
    BTFSC   STATUS, Z
    GOTO    HANDLE_RECEIVED_MSG ;If it is a message from another Xbee, Handle it
    SUBLW   0x89
    BTFSC   STATUS, Z
    GOTO    HANDLE_RECEIVED_STATUS
    CLRF    PACKET_STATE        ;If it is nothing else, go ahead and restart the state machine
    RETURN

HANDLE_RECEIVED_STATUS:
    banksel PORTA
    DECF    TEMP_1, F
    DECF    TEMP_1, F
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
    BTFSC   STATUS, Z           ;If the message was successful in sending
    GOTO    DONT_RESEND_MSG     ;Dont resend the message
    MOVLW   I_AM_AVAILABLE      ;Else, resend the message, first checking if it needs to be broadcast
    SUBWF   RF_DATA_MSB, W
    BTFSC   STATUS, Z
    BSF     BROADCAST, 0
    MOVLW   ASSERTION_OF_COMMAND_RESULT
    SUBWF   RF_DATA_MSB, W
    BTFSC   STATUS, Z
    BSF     BROADCAST, 0
    CALL    SEND_MSG            ;Resend the message
    BCF     BROADCAST, 0
DONT_RESEND_MSG:
    CLRF    PACKET_STATE        ;Else, do nothing
    RETURN

HANDLE_RECEIVED_MSG:
    banksel PORTA
    DECF    TEMP_1, F           ;Lets see who it is from
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w

    MOVWF   RX_SOURCE_ADDRESS_MSB
    MOVLW   0xBC
    SUBWF   RX_SOURCE_ADDRESS_MSB, W    ;Check to see if we have indeed gotten a message from a COACH
    BTFSS   STATUS, Z                   ;If not, just quit
    GOTO    RETURN_FROM_HANDLE

    DECF    TEMP_1, F
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
    MOVWF   RX_SOURCE_ADDRESS_LSB

    DECF    TEMP_1, F
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
    MOVWF   RX_SIGNAL_STRENGTH

    DECF    TEMP_1, F
    MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
    MOVWF   RX_OPTIONS

	DECF	TEMP_1, F
	MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
	MOVWF	RX_OPCODE

	DECF	TEMP_1, F
	MOVLW   PACKET_ARRAY        ;Read the array value
    ADDWF   TEMP_1, W
    MOVWF   FSR
    MOVF    INDF, W             ;At this point, the array value is stored in w
	MOVWF	RX_PARAMETER

	MOVLW	NOT_PAIRED			;Check what state we are currently in
	SUBWF	COMM_SUPER_STATE, W
	BTFSC	STATUS, Z
	GOTO	RECEIVED_NOT_PAIRED	;If we are not paired, check if the message received was a not paired message
    MOVLW   PAIRED
    SUBWF   COMM_SUPER_STATE, W
    BTFSC   STATUS, Z
	GOTO	RECEIVED_PAIRED				;Otherwise we are paired
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; IF WE GET HERE, IT IS BAD NEWS.  OUR STATE MACHINE IS IN NEITHER OF THE SUPER STATES
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    MOVLW   NOT_PAIRED
    MOVWF   COMM_SUPER_STATE
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
    ; SO JUST RESET THE COMM STATE MACHINE
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RETURN_FROM_HANDLE:	
    CLRF    PACKET_STATE
    RETURN

RECEIVED_PAIRED:				;Here, we are in the paired super-state
    banksel PORTA

    MOVF    RX_SOURCE_ADDRESS_LSB, W    ;Check to see if it is from the person who is in control of me.
    SUBWF   DEST_ADDRESS_LSB, W
    BTFSS   STATUS, Z
    GOTO    RETURN_FROM_HANDLE

	MOVLW	JETTISON_THE_CREW	;Check if the opcode was jettison
	SUBWF	RX_OPCODE, W
	BTFSC	STATUS, Z
	GOTO	HANDLE_PAIRED_JTC	;Handle a jettison
	MOVLW	NORMAL				;Find out what sub-state we are in
	SUBWF	COMM_STATE, W
	BTFSC	STATUS, Z
	GOTO	CHECK_NORMAL_RECEIVE
CHECK_MUTINOUS_RECEIVE:
    banksel PORTA
	MOVLW	SUPPRESS_MUTINY
	SUBWF	RX_OPCODE, W
	BTFSS	STATUS, Z
	GOTO	END_RECEIVED_PAIRED	;If it wasn't a suppress, mutiny, it doesn't make sense.  So end.
	MOVLW	0xFB				;Move -5 into w
	MOVWF	MORALE
	MOVLW	UPDATE_MORALE
	MOVWF	RF_DATA_MSB
	MOVF	MORALE, W
	MOVWF	RF_DATA_LSB
	CALL	SEND_MSG
	CLRF	VELOCITY_TIMEOUT_TIMER	;Reset the velocity timeout timer
	CLRF	MORALE_UPDATE_TIMER		;Reset the morale update timer
	MOVLW	NORMAL				;Transition to the normal state
	MOVWF	COMM_STATE
END_RECEIVED_PAIRED:
	GOTO    RETURN_FROM_HANDLE

HANDLE_PAIRED_JTC:
    banksel PORTA
	CLRF	FWD_VELOCITY	;Reset Velocities
	CLRF	ROT_VELOCITY
	CALL	SET_VELOCITY
	MOVLW	I_AM_AVAILABLE
	MOVWF	RF_DATA_MSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
    MOVLW   0x03
    MOVWF   RF_DATA_LSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	BSF		BROADCAST, 0	;Turn on broadcast
	CALL	SEND_MSG
	BCF		BROADCAST, 0	;Turn off broadcast
	CLRF	AVAILABLE_TIMER	;Restart the available timer
	MOVLW	NOT_PAIRED			;Transition to the not paired super state
	MOVWF	COMM_SUPER_STATE
	GOTO	END_RECEIVED_PAIRED
	
CHECK_NORMAL_RECEIVE:
    banksel PORTA
	MOVLW	VELOCITY
	SUBWF	RX_OPCODE, W		;Check if we have a velocity command
	BTFSS	STATUS, Z
    GOTO    CHECK_RECEIVED_FREE_ANALOG

	MOVLW	VELOCITY_TABLE>>8		;Time to do a table lookup
	MOVWF	PCLATH
	SWAPF	RX_PARAMETER, W
	ANDLW	0x0F				;Pull just the upper nibble to get forward velocity
	CALL	VELOCITY_TABLE		;At this point, w should contain the velocity table value
	MOVWF	FWD_VELOCITY

	MOVLW	VELOCITY_TABLE>>8		;Time to do a table lookup
	MOVWF	PCLATH
	MOVF	RX_PARAMETER, W
	ANDLW	0x0F				;Pull just the lower nibble to get the rotational velocity
	CALL	VELOCITY_TABLE
	MOVWF	ROT_VELOCITY

	CALL	SET_VELOCITY
	MOVLW	UPDATE_MORALE
	MOVWF	RF_DATA_MSB
	MOVF	MORALE, W
	MOVWF	RF_DATA_LSB
	CALL	SEND_MSG
    banksel PORTA
	CLRF	VELOCITY_TIMER		;Reset the velocity timer
	CLRF	VELOCITY_TIMEOUT_TIMER	;Reset the velocity timeout timer
	GOTO	END_RECEIVED_PAIRED

CHECK_RECEIVED_FREE_ANALOG:
    banksel PORTA
    MOVLW   FREE_ANALOG
    SUBWF   RX_OPCODE, W
    BTFSS   STATUS, Z           ;Check to see if we received a free analog command
    GOTO    CHECK_RECEIVED_FREE_DIGITAL ;If not, check if we got a free digital
    MOVF    RX_PARAMETER, W
    MOVWF   DUTY_CYCLE
    CALL    PWM_DUTY
	GOTO    END_RECEIVED_PAIRED

CHECK_RECEIVED_FREE_DIGITAL:
    banksel PORTA
    MOVLW   FREE_DIGITAL
    SUBWF   RX_OPCODE, W
    BTFSS   STATUS, Z           ;Check to see if we received a free digital command
    GOTO    END_RECEIVED_PAIRED ;If not, it doesn't make sense, so quit
    MOVLW   KICKER_MOTOR_BIT    ;If it is, toggle the kicker line
    XORWF   PORTC, F
    GOTO    END_RECEIVED_PAIRED

PWM_DUTY:
	; Handle direction first; we only want positive duty cycles
	banksel		PORTA
	btfsc		DUTY_CYCLE, 7
	goto		DUTY_NEG
DUTY_POS:
	; Set the direction registers for positive motion
	bcf		PORTC, 6
	movlw		CCP1CON_FWD
	movwf		CCP1CON

	; The value of duty cycle is already positive, so just pass it along
	goto		SET_8_BITS
DUTY_NEG:
	; Set the direction registers for negative motion
	bsf		PORTC, 6
	movlw		CCP1CON_REV
	movwf		CCP1CON

	; Take the 2's complement of the duty cycle to make it positive
	comf		DUTY_CYCLE, F
	movlw		1
	addwf		DUTY_CYCLE, F

	; Now we can pass along this value of the duty cycle
SET_8_BITS:
	; For the servo motor, the 8 bits are 2d + 0.5d
	
	; To divide by 2, clear the carry-bit and shift right
	bcf		STATUS, C
	rrf		DUTY_CYCLE, W
	movwf		DUTY_CYCLE_2

	; To multiply by 2, clear the carry-bit and shift left
	bcf		STATUS, C
	rlf		DUTY_CYCLE, W

	; Add these two together to get 2.5d
	addwf		DUTY_CYCLE_2, F

	; This addition may have overflowed; if it did, we wanted 100% = 250
	btfss		STATUS, C
	goto		SEND_8_BITS
TRUNCATE_DUTY:
	; Put the value 250 into DUTY_CYCLE_2
	movlw		d'250'
	movwf		DUTY_CYCLE_2
SEND_8_BITS:
	; DUTY_CYCLE_2 is now ready to go, so we just need to pass it through
	movf		DUTY_CYCLE_2, W
	movwf		CCPR1L
    RETURN

RECEIVED_NOT_PAIRED:
    banksel PORTA
	MOVLW	ASSERTION_OF_COMMAND
	SUBWF	RX_OPCODE, W
	BTFSS	STATUS, Z
	GOTO	RETURN_FROM_HANDLE	;Since the only command that is relavent when Not Paired is an assertion of command, go back if we get anything else
	MOVF	RX_PARAMETER, W				;Set the new team color
	MOVWF	TEAM_COLOR

    ADDLW   0x01                        ;Put the bit in the proper place
    BCF     PORTC, 0                    ;Clear both of the colors, just in case
    NOP
    BCF     PORTC, 1
    IORWF   PORTC, F                    ;Turn on the bit of the new team color
    

	MOVF	RX_SOURCE_ADDRESS_MSB, W	;Set who the new owner is
	MOVWF	DEST_ADDRESS_MSB
	MOVF	RX_SOURCE_ADDRESS_LSB, W
	MOVWF	DEST_ADDRESS_LSB
	MOVWF	RF_DATA_LSB			;Out of place a bit, but make sure to put it in the LSB for when we send out the AOC_Result
	CLRF	MORALE				;Reset Morale
	MOVLW	ASSERTION_OF_COMMAND_RESULT
	MOVWF	RF_DATA_MSB
	BSF		BROADCAST, 0
	CALL	SEND_MSG			;And send out the message
	BCF		BROADCAST, 0		;Turn off broadcast mode
	MOVLW	PAIRED
	MOVWF	COMM_SUPER_STATE		;Change to the paired state
	MOVLW	NORMAL
	MOVWF	COMM_STATE
	CLRF	VELOCITY_TIMEOUT_TIMER	;Reset the velocity timeout timer
	CLRF	MORALE_UPDATE_TIMER		;Reset the morale update timer
    CLRF    VELOCITY_TIMER          ;SHOULD NOT BE HERE, BUT WON'T HURT
	GOTO	RETURN_FROM_HANDLE
	
BAD_CHECKSUM:
    banksel PORTA
    CLRF    PACKET_STATE
    RETURN

TRANSMIT_SPI:
    banksel SSPSTAT
TRANSMIT_SPI_LOOP:
    BTFSS   SSPSTAT, BF
    GOTO    TRANSMIT_SPI_LOOP
	banksel SSPBUF 
    MOVF    SSPBUF, W 
	MOVF	TX_SPI_DATA, W      ;Put the data to send into the SSP Buffer
	MOVWF	SSPBUF
	RETURN

SET_VELOCITY:
	; The duty cycle equations depend on whether the velocity is forward or reverse
	; However, the same equations will be used, so calculate them once
	banksel		PORTA

	; The equation for one wheel is (FWD - ROT)/2
	; But with 8 bits, we have to divide each by 2 first

	; To divide by 2, sign-extend the carry-bit and shift right
	btfsc		FWD_VELOCITY, 7
	bsf			STATUS, C
	btfss		FWD_VELOCITY, 7
	bcf			STATUS, C
	rrf			FWD_VELOCITY, W
	movwf		FWD_VEL_2

	; To divide by 2, sign-extend the carry-bit and shift right
	btfsc		ROT_VELOCITY, 7
	bsf			STATUS, C
	btfss		ROT_VELOCITY, 7
	bcf			STATUS, C
	rrf			ROT_VELOCITY, W
	movwf		ROT_VEL_2
	
	; Perform and save the subtraction
	movf		ROT_VEL_2, W
	subwf		FWD_VEL_2, W
	movwf		SUB_VELOCITY

	; The equation for the other wheel is (FWD + ROT)/2
	; We already took care of the /2 part, so we just have to add

	; Perform and save the subtraction
	movf		ROT_VEL_2, W
	addwf		FWD_VEL_2, W
	movwf		ADD_VELOCITY

	; Now which wheel gets which velocity depends on FWD_VELOCITY's sign	
	btfsc		FWD_VELOCITY, 7
	goto		FWD_NEG
FWD_POS:

	; Left wheel gets SUB, right wheel gets ADD
	; Indicate that we want to send to the left wheel
    BSF         PORTC, RIGHT_SS             ;Turn on left SS Pic
    NOP
    BCF         PORTC, LEFT_SS
	movf		SUB_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	movf		SUB_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	; Indicate that we want to send to the right wheel
    BSF         PORTC, LEFT_SS             ;Turn on right SS Pic
    NOP
    BCF         PORTC, RIGHT_SS
	movf		ADD_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	movf		ADD_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	; We're done!
	goto		END_SET_VELOCITY
FWD_NEG:

	; Left wheel gets ADD, right wheel gets SUB
	; Indicate that we want to send to the left wheel
    BSF         PORTC, RIGHT_SS             ;Turn on left SS Pic
    NOP
    BCF         PORTC, LEFT_SS
	movf		ADD_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	movf		ADD_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	; Indicate that we want to send to the right wheel
    BSF         PORTC, LEFT_SS             ;Turn on right SS Pic
    NOP
    BCF         PORTC, RIGHT_SS
	movf		SUB_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI

	movf		SUB_VELOCITY, W
	movwf		TX_SPI_DATA
	call		TRANSMIT_SPI
END_SET_VELOCITY:
    BSF         PORTC, LEFT_SS
    BSF         PORTC, RIGHT_SS
    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Initialization Sequence
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
INIT:
    CALL    INIT_PORTS
#ifdef TEST_WITH_PUSH_BUTTON
    CALL    INIT_PUSH_TO_INT
#endif
    CALL    INIT_ASYNCH
    CALL    INIT_SPI
	CALL	INIT_TIMERS
	CALL	INIT_STATE_MACHINE
    CALL    PWM_INIT
    banksel PORTC
    CLRF    PORTC
    BSF     PORTC, 2
	CLRF	PACKET_STATE	;Clear the packet byte counter
	CLRF	MORALE
    CLRF    FWD_VELOCITY
    CLRF    ROT_VELOCITY
	BSF		INTCON, PEIE	;Enable peripheral interrupts
	BSF		INTCON, GIE		;Enable global interrupts
    GOTO    MAIN

TOGGLE_BIT:
    banksel PORTA
    MOVLW   0x01
    XORWF   PORTA, F
    RETURN

INIT_PORTS:
	banksel	ANSEL			;BANK 2
	CLRF	ANSEL			;Set all ports to be digital IO
	CLRF	ANSELH
	banksel TRISA			;BANK 1
	MOVLW	TRISA_CONFIG
	MOVWF	TRISA			;Set Port A0-A2 as inputs, A3-A7 as outputs
	MOVLW	TRISC_CONFIG
	MOVWF	TRISC			;Set Port C0-C7 as outputs
    MOVLW   TRISB_CONFIG
    MOVWF   TRISB           ;Set Port B0-B7 as inputs
    RETURN

#ifdef TEST_WITH_PUSH_BUTTON
INIT_PUSH_TO_INT:
    banksel OPTION_REG
	BCF		OPTION_REG, INTEDG  ;Make it interrupt on a falling edge
    NOP
	BSF		INTCON, INTE		;Enable interrupt on RA2
    RETURN
#endif
	
INIT_ASYNCH:
    banksel TXSTA
	BCF		TXSTA, SYNC		    ;Use 8-bit/Asynchronous
	BCF		BAUDCTL, BRG16
	BSF		TXSTA, BRGH
	MOVLW	0x40			;Set the baud rate to 9600 (technically 9615)
	MOVWF	SPBRG
	BCF		TXSTA, TX9		;Enable 8 bit transfers
	BSF		TXSTA, TXEN		;Transmit enabled
	banksel	RCSTA			;BANK 0
	BSF		RCSTA, SPEN		;Enable the serial port
	BCF		RCSTA, RX9		;Enable 8 bit received messages
	BSF		RCSTA, CREN		;Receive enabled
	banksel	PIE1			;BANK 1
	BSF		PIE1, RCIE		;Enable the receive interrupt
    RETURN

INIT_SPI:
	banksel SSPCON			;BANK 0
	MOVLW 	SSPCON_CONFIG
	MOVWF 	SSPCON
	banksel	SSPSTAT	        ;BANK 1
	MOVLW	SSPSTAT_CONFIG
	MOVWF	SSPSTAT
    banksel SSPBUF
    MOVLW   0x00
    MOVWF   SSPBUF
    RETURN

INIT_TIMERS:
    MOVLW	OPTION_REG_CONFIG
	banksel	OPTION_REG
	MOVWF	OPTION_REG
    banksel TMR0
	CLRF	TMR0
	banksel	INTCON
	BSF		INTCON, T0IE	;Enable the interrupt on overflow.
    banksel PORTA
	CLRF	TIMER0_COUNTER
	CLRF	AVAILABLE_TIMER
	CLRF	VELOCITY_TIMEOUT_TIMER
	CLRF	MORALE_UPDATE_TIMER
	CLRF	MUTINY_TIMER
	CLRF	MUTINY_MESSAGE_TIMER
	CLRF	VELOCITY_TIMER
    BCF     INTCON, T0IF
	RETURN
	
INIT_STATE_MACHINE:
	banksel	PORTA
	MOVLW	I_AM_AVAILABLE		;Start by sending an "I am available" message
	MOVWF	RF_DATA_MSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
    MOVLW   0x04
    MOVWF   RF_DATA_LSB
    ;;;;;;;;;;;;;;;;;;;;;;;;;;;
	BSF		BROADCAST, 0
	CALL	SEND_MSG
	BCF		BROADCAST, 0
	CLRF	AVAILABLE_TIMER		;Reset the available timer
	MOVLW	NOT_PAIRED			;We are now in the not paired super state
	MOVWF	COMM_SUPER_STATE
	RETURN

PWM_INIT:
	; Minimize bank selects by starting in bank 1
	banksel		PR2

	; Put the initialization byte into PR2
	movlw		PR2_CONFIG
	movwf		PR2

	; All other PWM registers are in bank 0(
	banksel		CCPR1L

	; Put the initialization byte into CCPR1L
	movlw		CCPR1L_CONFIG
	movwf		CCPR1L

	; Put the initialization byte into CCP1CON
	; It's okay even if TMR2 is already on, because duty cycle is 0
	movlw		CCP1CON_FWD
	movwf		CCP1CON

	; Put the initialization byte into T2CON
	; Do this last since it starts the TMR2 counting
	movlw		T2CON_CONFIG
	movwf		T2CON
    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Main Code (Note: Nothing interesting here...)
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
MAIN:
#ifdef TEST_STU_COMM
    MOVLW   0xBC
    MOVWF   DEST_ADDRESS_MSB
    MOVLW   0x05
    MOVWF   DEST_ADDRESS_LSB
#endif
LOOP:
    BTFSC   RCSTA, FERR     ;Check if there was a framing error
    MOVF    RCREG, W
    BTFSC   RCSTA, OERR     ;Check if there was an overrun error
    BCF     RCSTA, CREN
	GOTO	LOOP		;Loop around to continue testing
FINISHED:
	END