	list P=PIC16F690
#include "p16F690.inc"
	__config (_CP_OFF & _WDT_OFF & _PWRTE_ON & _HS_OSC & _MCLRE_OFF)

; Constants
#define		BAUDRATE		0x0F	; SPBRG = 15, sets baud rate to 9600, for a 10MHz crystal
#define		START_DEL		0x7E	; constant, to start message
#define		LENGTHMSB		0x00	; constant, determined by protocol
#define		LENGTHLSB		0x07	; constant, determined by protocol
#define		TX_API			0x01	; constant, XBEE transmit
#define		RC_API			0x81	; constant, XBEE receive
#define		TX_FRAMEID		0x01	; constant, arbitrary
#define		TX_OPTIONS		0x01	; constant, arbitrary
#define		TOT_BYTES		0x0B	; constant, total of 11 bytes
#define		CONTROLLER		0xBC	; constant, determined by protocol
#define		BOAT			0xAF	; constant, determined by protocol
#define		OUR_TEAM		0x06	; our team ID
#define		NOBOAT			0xFF	; our device is not paired
#define		BROADCASTBYTE	0xFF	; constant, XBEE defined
#define		SUPPRESS_MUT	0x00	; opcode, to boat
#define		ASSERT_COMMAND	0x01	; opcode, to boat	
#define		JETTISON_CREW	0x02	; opcode, to boat	
#define		VELOCITY		0x03	; opcode, to boat	
#define		FREE_DIGITAL	0x04	; opcode, to boat	
#define		FREE_AN			0x05	; opcode, to boat	
#define		MUTINY			0x20	; opcode, to controller
#define		AVAILABLE		0x21	; opcode, to controller
#define		ASSERT_RESULT	0x22	; opcode, to controller
#define		UPDATE_MORALE	0x23	; opcode, to controller
#define		ANYTHING		0x2A	; parameter, when it does not matter!
#define		NEG5MORALE		0xFB	; morale = -5 in two's complement
#define		NEG10MORALE		0xF6	; morale = -10 in two's complement
#define		NOMORALE		0x10	; value to indicate that there is currently no morale value
#define		JETTISON_BIT	0x5		; Jettison pin on PORTC
#define		AOC_BIT			0x4		; Assertion of Command button on PORTB
#define		FREE_DIG_BIT	0x6		; Free Digital on PORTB
#define		TEAM_BIT		0x0		; Team select on PORTA
#define		MORALE1_BIT		0x1		; Morale1 shift reg on PORTC
#define		SHIFT_CLK_BIT	0x3		; Shift reg clock on PORTC
#define		MORALE2_BIT		0x2		; Morale2 shift reg on PORTC
#define		BOAT_NUM_BIT	0x7		; Boat number shift reg on PORTC
#define		MUTINY_BIT		0x4		; Mutiny announce bit on PORTC
#define		AVAIL_BIT		0x7		; Bit for available light in morale2 shift register

; 7-segment digits
#define		DIGIT_0			b'11111100'
#define		DIGIT_1			b'00001100'
#define		DIGIT_2			b'11011010'
#define		DIGIT_3			b'10011110'
#define		DIGIT_4			b'00101110'
#define		DIGIT_5			b'10110110'
#define		DIGIT_6			b'11110110'
#define		DIGIT_7			b'00011100'
#define		DIGIT_8			b'11111110'
#define		DIGIT_9			b'10111110'
#define		DIGIT_A			b'01111110'
#define		DIGIT_B			b'11100110'
#define		DIGIT_OFF		b'00000000'

; Variable definitions
W_TEMP			equ		0x70		; for use by interrupt handler, to preserve W, accessible from any bank
STATUS_TEMP		equ		0x71		; for use by interrupt handler, to preserve STATUS, accessible from any bank
pairedID		equ		0x20		; the address LSB of the paired transmitter
bytebox			equ		0x21		; a temp storage location for use by receive function
saved_param		equ		0x22		; need to better define how this is used - do it in the receive function instead?
saved_id		equ		0x23		; the source ID from last transmission
morale			equ		0x24		; current morale in tenths of a morale point
velocity		equ		0x25		; current velocity (speed and direction)
temp			equ		0x26		; temp file - DO NOT USE IN INTERRUPTS
temp_shake		equ		0x27		; temp file - DO NOT USE IN INTERRUPTS
vel_analog		equ		0x28		; value of velocity analog read
free_analog		equ		0x29		; free analog value
suppress_analog	equ		0x30		; analog value of accelerometer for suppress mutiny
dir_analog		equ		0x31		; value of direction analog read
pir1_temp		equ		0x32		; temporary pir1 register for use in interrupt vector
intcon_temp		equ		0x33		; temporary intcon register for use in interrupt vector
morale1_disp	equ		0x34		; byte to pass to morale1 shift reg
morale2_disp	equ		0x35		; byte to pass to morale2 shift reg
boat_disp		equ		0x36		; byte to pass to bpat shift reg

; flags and counters

rc_counter		equ		0x37		; counter to retrieve correct byte from receive table
rc_discardmsg	equ		0x38		; flag set if recieved message was incorrect - prevent save of incoming message
tx_counter		equ		0x39		; counter to retrieve correct byte from transmit table
tx_status		equ		0x40		; bit 0: flag is set when a packet is done transmitting, clear while packet is transmitting
coach_state		equ		0x41		; state indicator only when set, bit 1 is (startup), bit 2 is (paired), bit 3 is (not paired), bit 4 is (mutinous crew), bit 5 is (waiting for mutinous crew response), bit 6 is (asserting command), bit 7 is (waiting for AoC result)
rc_opcodeflag	equ		0x42		; flag indicates which opcode was received
tx_opcodeflag	equ		0x43		; flag indicates which opcode to transmit, once the transmit function is called by a timer
assertion_cmd	equ		0x44		; assertion of command action (bit 0)

; timers - in timer 0
tim_enabled		equ		0x45		; flag indicates which time is enabled, allows for starting and stopping timers
tim_expired		equ		0x46		; flag indicates which timer has expired, for the state machine!
tim_np		 	equ		0x47		; Not Paired Timer (5 seconds), bit 5
tim_mutiny		equ		0x48		; Mutiny Timer (1 seconds), bit 4
tim_velocity	equ		0x49		; Velocity Timer (1 second), bit 3
tim_aoc			equ		0x50		; Assertion of Command timer (1 second), bit 2
tim_morale		equ		0x51		; Morale Timer (5 second), bit 1
tim_available	equ		0x52		; Available timer (3 seconds), bit 0
tim_tx_timeout	equ		0x53		; transmit timeout timer, bit 7
tim_analog		equ		0x54		; free analog send timer, bit 6
tim_shake		equ		0x55		; timer to keep track of shakes
tim_shake_to	equ		0x56		; timer to keep track of shake time out
tim_shake_stat	equ		0x57		; keeps track if timer is enabled (bit 1) or expired (bit 0)

; transmit/receive data storage

tx_opcode		equ		0x58		; memory
tx_parameter	equ		0x59		; memory
tx_checksum		equ		0x60		; memory
tx_id			equ		0x61		;

rc_api			equ 	0x62		; memory
rc_sourceMSB	equ		0x63		; memory
rc_sourceLSB	equ		0x64		; memory
rc_rssi			equ		0x65		; memory
rc_options		equ		0x66		; memory
rc_opcode		equ		0x67		; memory
rc_parameter	equ		0x68		; memory
rc_checksum		equ		0x69		; memory


; begin code
	
			org 0					;
			goto 	INITIALIZE		; initialization routine!
			org 4					; 
			goto	PUSH			; interrupt routine!
			org 5					;

TX_TABLE:
			addwf	PCL, f			; add W to PC counter --> jumps to entry in table				
			goto	BYTE_T1			;
			goto	BYTE_T2			;
			goto	BYTE_T3			;
			goto	BYTE_T4			;
			goto	BYTE_T5			;
			goto	BYTE_T6			;
			goto	BYTE_T7			;
			goto	BYTE_T8			;
			goto	BYTE_T9			;
			goto	BYTE_T10		;
			goto	BYTE_T11		;

DISP_TABLE:
			addwf	PCL,f			; Jump to table entry
			retlw	DIGIT_0			; display '0'
			retlw	DIGIT_1			; display '1'
			retlw	DIGIT_2			; display '2'
			retlw	DIGIT_3			; display '3'
			retlw	DIGIT_4			; display '4'
			retlw	DIGIT_5			; display '5'
			retlw	DIGIT_6			; display '6'
			retlw	DIGIT_7			; display '7'
			retlw	DIGIT_8			; display '8'
			retlw	DIGIT_9			; display '9'
			retlw	DIGIT_A			; display '10'
			retlw	DIGIT_B			; display '11'

RC_TABLE:	
			addwf	PCL, f			; add W to PC counter --> jumps to entry in table				
			goto	BYTE_R1			;
			goto	BYTE_R2			;
			goto	BYTE_R3			;
			goto	BYTE_R4			;
			goto	BYTE_R5			;
			goto	BYTE_R6			;
			goto	BYTE_R7			;
			goto	BYTE_R8			;
			goto	BYTE_R9			;
			goto	BYTE_R10		;
			goto	BYTE_R11		;

BYTE_T1		retlw	START_DEL		;
BYTE_T2		retlw	LENGTHMSB		;
BYTE_T3		retlw	LENGTHLSB		;
BYTE_T4		retlw	TX_API			;
BYTE_T5		retlw	TX_FRAMEID		;
BYTE_T6		retlw	BOAT			;
BYTE_T7		movf	tx_id,w		;
			return					;
BYTE_T8		retlw	TX_OPTIONS		;
BYTE_T9		movf	tx_opcode,w		;
			return					;
BYTE_T10	movf	tx_parameter,w	;
			return					;
BYTE_T11	movf	tx_checksum,w	;
			return					;

; error checking is inserted in the table ... flag discardmsg is set if any byte does not match up
; discardmsg flag gets clear after the save function

BYTE_R1 	movf	bytebox, w		; empty byte box into w
			xorlw	START_DEL		; compare equality with the start delimiter (they should be equal)
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; no need to save it for later access
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					;
BYTE_R2		movf	bytebox, w		; empty byte box into w
			xorlw	LENGTHMSB		; compare equality with the LENGTHMSB
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; no need to save it for later access
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					;		
BYTE_R3		movf	bytebox, w		; empty byte box into w
			xorlw	LENGTHLSB		; compare equality with the LENGTHLSB
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; no need to save it for later access
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					;
BYTE_R4		movf	bytebox, w		; empty byte box into w
			xorlw	RC_API			; compare equality with the RC_API - this is something that may change if we want other messages from XBEE
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; no need to save it for later access
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					;
BYTE_R5		movf	bytebox, w		; empty byte box into w
			movwf	rc_sourceMSB	; store for calculation
			xorlw	BOAT			; as a controller, can only receive messages from boat
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; no need to save it for later access
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					;	
BYTE_R6		movf	bytebox, w		; empty byte box into w
			movwf	rc_sourceLSB	;
			movf	pairedID,w		;
			xorlw	NOBOAT			; test if device is unpaired
			btfsc	STATUS, Z		; if device unpaired, z bit is set, else it is clear
			return					; if it is, do not set discard message command
			movf	rc_sourceLSB,w		;
			xorwf	pairedID,w		; if paired, check that it is paired with correct device
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					; no need to save it for later access
BYTE_R7		movf	bytebox, w		; empty byte box into w
			movwf	rc_rssi			; just save for checksum
			return					;
BYTE_R8		movf	bytebox, w		; empty byte box into w
			movwf	rc_options		; just save for checksum
			return					;
BYTE_R9		movf	bytebox, w		; empty byte box into w
			movwf	rc_opcode		; just save, for checksum, transfer out if good message later						
			return					;
BYTE_R10	movf	bytebox, w		; empty byte box into w
			movwf	rc_parameter	; just save, for checksum, transfer out if good message later								
			return					;
BYTE_R11	movf	bytebox, w		; empty byte box into w
			movwf	rc_checksum		; save
		    goto	CALC_RCCHECKSUM ; compare to calculated value, throw error if they are not equal
END_BYTE	return					;

CALC_RCCHECKSUM:
			movlw	0				; Start off with 0
			addlw	RC_API			; Add required bytes together, ignoring overflow
			addwf	rc_sourceMSB,W	;
			addwf	rc_sourceLSB,W	;
			addwf	rc_rssi,W		;
			addwf	rc_options,W	;
			addwf	rc_opcode,W		;
			addwf	rc_parameter,W	;
			addwf	rc_checksum,W	;
			xorlw	0xff			; check if the sum totals 0xff
			btfss	STATUS, Z		; if sums correctly, z bit is set, else it is clear
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			goto	END_BYTE		;

; interrupt handler, set interrupt priority here!
; measure interrupt speed on the scope, to verify it is not too long

PUSH:		
			movwf	W_TEMP			; save w to temp register
			swapf	STATUS, W		; swap status to be saved into w
			movwf	STATUS_TEMP		; save status to temp register
INT_FLAG:
			banksel	PIR1			; go to bank 0
			movf	PIR1,W
			movwf	pir1_temp
			movf	INTCON,W
			movwf	intcon_temp		; Look only at interrupts set at beginning

			btfsc	pir1_temp,TMR1IF	; Timer 1 interrupt flag
			call	TMR1_INT		; Timer 1 ISR
			banksel	PIR1			; go to bank 0
			btfsc	pir1_temp,ADIF	; AD interrupt flag
			call	AD_INT			; AD ISF
			banksel	PIR1			; go to bank 0
			btfsc	intcon_temp,T0IF	; timer 0 overflow interrupt flag
			call	T0_INT			; timer0 ISR
			banksel	PIR1			; go to bank 0
			btfsc	pir1_temp, TXIF	; transmit interrupt flag													
			call	TX_INT			; transmit ISR
			banksel	PIR1			; go to bank 0
			btfsc	pir1_temp, RCIF	; receive interrupt flag	
			call	RC_INT			; receive ISR
POP:			
			swapf	STATUS_TEMP, W	; swap status_temp into w
			movwf	STATUS			; restore status to original state
			swapf	W_TEMP, F		; swap w_temp nibbles
			swapf	W_TEMP, W		; swap w_temp again to restore w
			retfie					; return from interrupt	

TMR1_INT:
			bcf		PIR1,TMR1IF		; clear flag
			bsf		ADCON0,GO		; start AD conversion
			return

AD_INT:
			bcf		PIR1,ADIF		; clear flag
			movf	ADRESH,W		; load analog result into W
			btfsc	ADCON0,CHS0		; If AN0...
			goto	AD_VEL			; ...velocity input.

			btfsc	ADCON0,CHS1		; If AN2...
			goto	AD_FREE			; ...free analog input.

			btfsc	ADCON0,CHS2		; If AN4...
			goto	AD_SUP_MUT		; ... suppress mutiny input.

			btfsc	ADCON0,CHS3		; If AN8...
			goto	AD_DIR			; ...direction input.

AD_VEL		movwf	vel_analog		; save in velocity analog byte
			bcf		ADCON0,CHS0		; change channel
			bsf		ADCON0,CHS1		;
			return
AD_FREE		movwf	free_analog		; save in direction analog byte
			bcf		ADCON0,CHS1		; change channel
			bsf		ADCON0,CHS2		;
			return
AD_SUP_MUT	movwf	suppress_analog	; save in direction analog byte
			bcf		ADCON0,CHS2		; change channel
			bsf		ADCON0,CHS3		; 
			return
AD_DIR		movwf	dir_analog		; save in direction analog byte
			bcf		ADCON0,CHS3		; change channel
			bsf		ADCON0,CHS0		; 
			return


; transmit interrupt service routine and supporting functions

TX_INT:
			banksel	PIE1			; bank 1
			btfss	PIE1, TXIE		; test if interrupt is enabled 
			return					; transmit interrupts not enabled
			banksel	PWM1CON			; bank 0				
			clrf	tim_tx_timeout	; set timeout timer to 0
			movf	tx_counter,w	; move counter to w							
			call	TX_TABLE		; jump to table
			movwf	TXREG			; write value in w from table to transmit register, also clear interrupt flag
			incf	tx_counter,f	; increment counter
			movlw	TOT_BYTES		; move total bytes to w
			xorwf	tx_counter,w	; compare equality with txcounter
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear
			goto	TX_INT_END		; end routine
			bsf		tx_status,0		; set flag to indicate that packet transmit is completed
			banksel	PIE1			; bank 1
			bcf		PIE1, TXIE		; disable transmit interrupt
			banksel	PWM1CON			; bank 0
TX_INT_END	return					; end transmit service routine

;To transmit, call TRANSMIT
;It will check to see if a transmit is currently in progress, if not, it will assemble the message based on tx_opcodeflag

TRANSMIT:		
			btfsc	tx_status,0		; test if ready to transmit new packet
			call	ASSEMBLE_MESG	; assemble and send message
			return
ASSEMBLE_MESG:						; only one flag is ever set at a time

			movf	pairedID,w		;
			movwf	tx_id			; Load pairedID into the transmit ID variable
MSG1		btfss	tx_opcodeflag,0 ; need to update this - with individual updates of tx_opcode
			goto	MSG2			;
			movlw	SUPPRESS_MUT	;
			movwf	tx_opcode		; save value to tx_opcode!
			movlw	ANYTHING		;
			movwf	tx_parameter	; save value to tx_parameter!

MSG2		btfss	tx_opcodeflag,1 ;
			goto	MSG3			;
			movlw	ASSERT_COMMAND	; 
			movwf	tx_opcode		; save value to tx_opcode!
			movlw	0x0
			btfsc	PORTA,TEAM_BIT	;
			movlw	0x1				; determine current team from switch
			movwf	tx_parameter	; save value to tx_parameter!
MSG3		btfss	tx_opcodeflag,2 ;
			goto	MSG4			;
			movlw	JETTISON_CREW	;
			movwf	tx_opcode		; save value to tx_opcode!
			movf	ANYTHING,W		; move paired controller value into w
			movwf	tx_parameter	; save value to tx_parameter!
MSG4		btfss	tx_opcodeflag,3 ;
			goto	MSG5			;
			movlw	VELOCITY		;			
			movwf	tx_opcode		; save value to tx_opcode!
			movf	velocity,W		; load velocity
			movwf	tx_parameter	; save value to tx_parameter!

MSG5		btfss	tx_opcodeflag,4 ;
			goto	MSG6			;
			movlw	FREE_DIGITAL	;			
			movwf	tx_opcode		; save value to tx_opcode!
			movf	ANYTHING,W		;
			movwf	tx_parameter	; save value to tx_parameter!

MSG6		btfss	tx_opcodeflag,5 ;
			goto	PRESENDIT		;
			movlw	FREE_AN			;			
			movwf	tx_opcode		; save value to tx_opcode!
			movf	free_analog,W	; load analog value	
			movwf	tx_parameter	; save value to tx_parameter!

PRESENDIT	goto	CALC_TXCHECKSUM	; calculate the checksum
SENDIT		clrf	tx_counter		; set counter to 0	
			clrf	tx_opcodeflag	; clear all flags for transmit
			bcf		tx_status,0		; clear flag to indicate that packet transmit is now in progress
			clrf	tim_tx_timeout	; clear transmit timeout timer
			banksel	PIE1			; bank 1
			bsf		PIE1, TXIE		; transmit interrupt enable	- this starts send of message
			banksel	PWM1CON			; bank 0
			return					;

CALC_TXCHECKSUM:					; called from ASSEMBLE_MESG, calcs the checksum for the transmit
			movlw	0				; Start off with 0
			addlw	TX_API			; Add required bytes together, ignoring overflow
			addlw	TX_FRAMEID		;
			addlw	BOAT			;
			addwf	tx_id,W			;
			addlw	TX_OPTIONS		;
			addwf	tx_opcode,W		;
			addwf	tx_parameter,W	;
			sublw	0xff			; subtract sum of bytes from 0xff
			movwf	tx_checksum		; store in tx_checksum
			goto	SENDIT			;

; receive interrupt service routine and supporting functions
; need to implement save velocity and save analog signal

RC_INT:
			btfsc	rc_discardmsg,0	; test if discard flag is set
			goto	START_MSG		; if discard flag is set, start a new msg by clearing the counter and the flag 
			goto	RC_INT_CONT		; else continue
START_MSG	clrf	rc_counter		;
			bcf		rc_discardmsg,0	; reset discard message flag for next round
RC_INT_CONT:
			movf	RCREG,w			; read received value into w
			movwf	bytebox			; save w in the bytebox
			movf	rc_counter,w	; move recieve counter to w
			call	RC_TABLE		; jump to table
			incf	rc_counter,f	; increment counter	
									; test to see if packet is finished transmitting here!
			movlw	TOT_BYTES		; move total bytes to w
			xorwf	rc_counter,w	; compare equality with rc_counter
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			call 	RC_PACKETDONE	; perform end of packet procedure		
RC_INT_END	return					; end receive service routine

RC_PACKETDONE:
			clrf	rc_counter		; set counter to 0
			btfss	rc_discardmsg,0	; test to see if discard message flag was set
			goto	SAVERCDATA		; save the data that came in
			bcf		rc_discardmsg,0	; reset discard message flag for next round
			goto	RC_INT_END		; ends packet without saving data
SAVERCDATA:
			movf	rc_parameter, w	; empty received byte 10 into W
			movwf	saved_param		; save for use by program
			movf	rc_sourceLSB,w	;
			movwf	saved_id		; save ID for use by program
			call 	RECEIVE_FLAGS	; set appropriate flag by identifying the data that was read!
			return					; ends packet, having saved the data
RECEIVE_FLAGS:
			clrf	rc_opcodeflag	; clear opcodes because they're not always cleared elsewhere

			movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	MUTINY			; test whether saved_opcode is mutiny
			btfsc	STATUS, Z		; if mutiny, z bit is set, else it is clear	
			bsf		rc_opcodeflag,0	; set bit to indicate mutiny in progress, disregard parameter bit
									 
			movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	AVAILABLE  		; test whether saved_opcode is available
			btfsc	STATUS, Z		; if available, z bit is set, else it is clear
			bsf		rc_opcodeflag,1	; set bit to indicate available result	

			movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	ASSERT_RESULT	; test whether saved_opcode is available
			btfsc	STATUS, Z		; if assertion result, z bit is set, else it is clear	
			bsf		rc_opcodeflag,2	; set bit to indicate assert result, disregard parameter bit
									 
			movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	UPDATE_MORALE	; test whether saved_opcode is available
			btfsc	STATUS, Z		; if update morale, z bit is set, else it is clear
			bsf		rc_opcodeflag,3	; set bit to indicate morale received
	
			goto	RC_INT_END		; ends packet having saved data!

T0_INT:								; when enabling a timer, the appropriate overflow must be cleared a the same time (this is not handled here)
			banksel	PWM1CON			; bank 0
			bcf		INTCON, T0IF	; clear timer 0 flag

TIMER_TX_TIMEOUT:					; this timer ensures that transmission of data only occurs at 5Hz
			incf	tim_tx_timeout,f	; increment counter
			movf	tim_tx_timeout,w	;
			xorlw	0x08			; 8 overflows, 0.2 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_SHAKE	;
			clrf	tim_tx_timeout	; reset counter
			bsf		tim_expired,7	; set flag!

TIMER_SHAKE:
			btfss	tim_shake_stat,1	; test if timer is enabled, otherwise skip
			goto	TIMER_SHAKE_TO	;
			incf	tim_shake,f		; increment counter	
			movf	tim_shake,w		;
			xorlw	0x08			; 8 overflows, .25 seconds,
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_SHAKE_TO	;
			clrf	tim_shake		; reset counter
			bsf		tim_shake_stat,0	; set expired flag!	

TIMER_SHAKE_TO:
			btfss	tim_shake_stat,3	; test if timer is enabled, otherwise skip
			goto	TIMER_NP		;
			incf	tim_shake_to,f	; increment counter	
			movf	tim_shake_to,w	;
			xorlw	0x27			; 39 overflows, 1 second
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_NP		;
			clrf	tim_shake_to	; reset counter
			bsf		tim_shake_stat,2	; set expired flag!	

TIMER_NP:
			btfss	tim_enabled,5	; test if timer is enabled, otherwise skip
			goto	TIMER_MORALE		;
			incf	tim_np,f		; increment counter	
			movf	tim_np,w		;
			xorlw	0xBF			; 191 overflows, 5 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_MORALE	;
			clrf	tim_np			; reset counter
			bsf		tim_expired,5	; set flag!

TIMER_MORALE:
			btfss	tim_enabled,1	; test if timer is enabled, otherwise skip
			goto	TIMER_MUTINY	;
			incf	tim_morale,f	; increment counter		
			movf	tim_morale,w	;
			xorlw	0xBF			; 191 overflows, 5 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_MUTINY	; 
			clrf	tim_morale		; reset counter
			bsf		tim_expired,1	; set flag!
TIMER_MUTINY:		
			btfss	tim_enabled,4	; test if timer is enabled, otherwise skip
			goto	TIMER_AOC	;
			incf	tim_mutiny,f	; increment counter
			movf	tim_mutiny,w	;
			xorlw	0x27			; 39 overflows, 1 second
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_AOC	;
			clrf	tim_mutiny		; reset counter
			bsf		tim_expired,4	; set flag!

TIMER_AOC:
			btfss	tim_enabled, 2	; test if timer is enabled, otherwise skip
			goto	TIMER_VELOCITY	;
			incf	tim_aoc,f		; increment counter		
			movf	tim_aoc,w	;
			xorlw	0x27			; 39 overflows, 1 second 
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_VELOCITY	; 
			clrf	tim_aoc 		; reset counter
			bsf		tim_expired,2	; set flag!
TIMER_VELOCITY:	
			btfss	tim_enabled,3	; test if timer is enabled, otherwise skip
			goto	TIMER_AVAIL		;
			incf	tim_velocity,f	; increment counter	
			movf	tim_velocity,w	;
			xorlw	d'38'			; 38 overflows, <1 second
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_AVAIL		;
			clrf	tim_velocity	; reset counter
			bsf		tim_expired,3	; set flag!
TIMER_AVAIL:
			btfss	tim_enabled,0	; test if timer is enabled, otherwise skip
			goto	TIMER_FREE_AN	;		
			incf	tim_available,f	; increment counter			
			movf	tim_available,w	;
			xorlw	0x75			; 117, 3 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_FREE_AN	;
			clrf	tim_available	; reset counter
			bsf		tim_expired, 0	; set flag!
TIMER_FREE_AN:
			btfss	tim_enabled,6	; test if timer is enabled, otherwise skip
			goto	END_T0			;		
			incf	tim_analog,f	; increment counter			
			movf	tim_analog,w	;
			xorlw	d'39'			; 39, 1 second
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	END_T0			;
			clrf	tim_analog		; reset counter
			bsf		tim_expired,6	; set flag!
END_T0		return					;



; main code begins here, all initalization!
			
INITIALIZE:
INIT_PORTS:
			clrf	PORTA			; clear to initialize
			clrf	PORTB			; clear to initialize
			clrf	PORTC			; clear to initialize
			banksel ANSEL			; bank 2
			clrf	ANSEL			; digital I/O
			clrf	ANSELH			; digital I/O
			bsf		ANSEL,1			; analog (velocity) RA1/AN1
			bsf		ANSEL,2			; analog (Free Analog) RA2/AN2
			bsf		ANSEL,4			; analog (suppress mutiny action) RC0/AN4
			bsf		ANSELH,0		; analog (direction) RC6/AN8
			banksel	TRISA			; bank 1 or 3
			bsf		TRISA,1			; input (velocity)
			bsf		TRISA,2			; input (Free Analog)
			bsf		TRISC,0			; input (suppress mutiny action)
			bsf		TRISC,6			; input (direction)
			bcf		TRISC,MUTINY_BIT	; output (mutiny in progress announcement; pin 4)

			bsf		TRISB,AOC_BIT		; input (Assertion of Command; pin 4)
			bsf		TRISA,TEAM_BIT		; input (Team select; pin 0)
			bsf		TRISC,JETTISON_BIT	; input (Jettison crew; pin 5)
			bsf		TRISB,FREE_DIG_BIT	; input (Free Digital; pin 6)

			bcf		TRISC,MORALE1_BIT	; output (Morale1 shift reg; pin 1)
			bcf		TRISC,SHIFT_CLK_BIT	; output (shift reg clock; pin 3)
			bcf		TRISC,MORALE2_BIT	; output (Morale2 shift reg; pin 2)
			bcf		TRISC,BOAT_NUM_BIT	; output (Boat number shift reg; pin 7)

			bsf		TRISB,5			; input (XBee receive) 
			bcf		TRISB,7			; output (XBee transmit)
INIT_ASYNC:
			banksel	SPBRG			; bank 1
			movlw	BAUDRATE		; get baud rate
			movwf	SPBRG			; sets baud rate
			bcf		TXSTA, SYNC		; clear to enable asynchronous serial port (part 1 of 2)
			bsf		TXSTA, TXEN		; enable transmission
			banksel	RCSTA			; bank 0
			bsf		RCSTA, SPEN 	; enable asynchronous serial port (part 2 of 2)
			bsf		RCSTA, CREN		; enable reception

INIT_ANALOG:
			; Tc = (-10picofarad)*(1kiloohm + 7kiloohm + 0 kiloohm)*ln(1/2047) = 0.55 us
			; Tacq = 5us + 0.61 us + [(50degC - 25degC)*(.005us/degC)] = 5.735 us
			banksel	ADCON1
			bsf		ADCON1,ADCS2	; Fosc/64
			bsf		ADCON1,ADCS1	; Fosc/64
			bcf		ADCON1,ADCS0	; Fosc/64
			banksel ADCON0
			bsf		ADCON0,CHS0		; Channel 1
			bcf		ADCON0,ADFM		; Left justified, so can ignore LSB
			bsf		ADCON0,ADON		; Turn on analog subsystem
			banksel	PIR1
			bcf		PIR1,ADIF		; clear interrupt flag

INIT_TMR0:
			banksel	OPTION_REG		; bank 1 or 3
			bcf		OPTION_REG, T0CS ; use internal clock
			bcf		OPTION_REG, PSA	; use 8 bit prescaler
			bsf		OPTION_REG, PS2 ; set prescaler assignment 1:256
			bsf		OPTION_REG, PS1 ; set prescaler assignment 1:256
			bsf		OPTION_REG, PS0 ; set prescaler assignment 1:256

INIT_TMR1:
			banksel	PIR1			; bank 0
			bcf		PIR1,TMR2IF		; clear Timer 1 flag
									; Set prescaler to 1 (default) (10 MHz / 4 / 1 = 2500 KHz; each tick = 0.4 us; rollovers = 65536 * 0.4 us = 26.2144 ms)
			bsf		T1CON,TMR1ON	; turn on Timer 1

INIT_INT:
			bsf		INTCON, GIE		; global interrupt enable
			bsf		INTCON, PEIE	; peripheral interrupt enable
			bsf 	INTCON, T0IE	; enable Timer0 Overflow Interrupt
			banksel	PIE1			; bank 1
			bsf		PIE1, RCIE		; receive interrupt enable
			bsf		PIE1,TMR1IE		; Timer 1 interrupt enable
			bsf		PIE1,ADIE		; Analog interrupt

INIT_VARS:
			banksel	PWM1CON			; bank 0
			clrf	rc_counter		; set receive counter to 0
			clrf	rc_discardmsg	; clear discard message flag
			clrf	tx_counter		;
			clrf	tx_status		;
			bsf		tx_status,0		;
			clrf	coach_state		;
			clrf	rc_opcodeflag	; clear any received items
			clrf	tx_opcodeflag	;
			clrf	assertion_cmd	;
			movlw	0xff			;
			movwf	saved_param		;
			movwf	saved_id		;
			movlw	NOMORALE		;
			movwf	morale			; initialize to not have morale
			clrf	velocity		; clear velocity
			clrf	temp	;
			clrf	tim_enabled		; clear all enabled timers
			clrf	tim_expired		; clear all expiration flags
			clrf	tim_np		 	;
			clrf	tim_mutiny		;
			clrf	tim_velocity	;
			clrf	tim_aoc			;
			clrf	tim_morale		;
			clrf	tim_available	;
			clrf	tim_tx_timeout	;
			clrf	tim_tx_timeout	;
			movlw	OUR_TEAM		; 
			movwf	pairedID		;
			clrf	boat_disp
			clrf	morale1_disp
			clrf	morale2_disp
			clrf	tim_shake
			clrf	tim_shake_stat
			clrf	tim_shake_to
			clrf	temp_shake

MAIN:
			clrf	tim_aoc			; Clear AoC timer
			bsf		tim_enabled, 2	; Start AoC timer
			clrf	tim_np			; Clear NP timer
			bsf		tim_enabled,5	; Start the NP timer
			clrf	coach_state		; clear the current coach state
			bsf 	coach_state,1	; Set State as Startup
			bsf		tim_expired,2	; Send AOC right away
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; jump to the event checker

; everything below here is the event checker and event handler
; event checker

EVENTS:
			btfsc	coach_state,1	; Test if in startup mode
			goto	STARTUP			;
			btfsc	coach_state,2	; Test if paired									
			goto	PAIRED_EVENTS	; 
			btfsc	coach_state,3	; Test if not paired
			goto	UNPAIRED_EVENTS	;
			btfsc	coach_state,4	; Test if Mutinous crew
			goto	MUTINOUS_CREW	;
			btfsc	coach_state,5	; Test if Waiting for Mutinous Crew Response
			goto	WAITING_MUT_RES	;
			btfsc	coach_state,6	; Test if Asserting Command
			goto	ASSERTING_CMD	;
			btfsc	coach_state,7	; Test if Waiting for AoC response
			goto	WAITING_AOC_RES	;

STARTUP:							; this is called only when the COACH first starts and then never again
			btfsc	rc_opcodeflag,2	; check if assertion of command results has been received
			goto	Assert_Res		; jump to event handler
			btfsc	tim_expired,2	; check to see if AoC timer has expired
			goto	Assertion_CMD	; go to event handler
			btfsc	tim_expired,5	; check to see if the NP timer has expired
			goto	NP_Tim_Exp		; jump back to startup
			goto	EVENTS			; jump to the event checker
										
PAIRED_EVENTS:
			bcf		PORTC,MUTINY_BIT	; Announce mutiny in progress
			btfss	tim_expired, 7	; test if 5Hz tranmsit is ready
			goto	PE_CONT			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	PORTC,JETTISON_BIT	; check if the jettison crew button is pressed
			goto	Jettison_Crew	; go to event handler
			btfsc	tim_expired,3	; Velocity Timer expires
			goto	Velocity		; handle the expired velocity timer
			btfsc	tim_expired,6	; if free analog timer is expired
			goto	Free_an			; go to event handler
			btfsc	PORTB,FREE_DIG_BIT	; check if the free digital button is pressed
			goto	Free_dig		; go to event handler	
			goto	Velocity		; If nothing better to do, send velocity
PE_CONT		btfsc	rc_opcodeflag,3	; receive “Update Morale” command?
			goto	Morale_Update   ; go to event handler
			btfsc	tim_expired,1	; check if morale timer has expired
			goto	Morale_Expired	; go to event handler
			btfsc	rc_opcodeflag,1	; check to see if the available command has been recieved (from your TOWRP)
			goto	Available		; jump to event handler
			btfsc	rc_opcodeflag,0	; Test if mutiny is in progress
			goto	Mutiny			; go to event handler
			goto	EVENTS			; restart event checker

UNPAIRED_EVENTS:
			bcf		PORTC,MUTINY_BIT	; Announce mutiny in progress
			btfsc	rc_opcodeflag,1	; receive "I am available?"	
			goto	Available		; go to event handler				 
			goto	EVENTS			; restart event checker

MUTINOUS_CREW:
			btfss	tim_expired, 7	; test if 5Hz tranmsit is not ready
			goto	MC_CONT			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	PORTC,JETTISON_BIT	; check if the jettison crew button is pressed
			goto	Jettison_Crew	; go to event handler
MC_CONT		call	DETECT_SHAKE	; detects if human is shaking the coach
			btfsc	tim_shake_stat,2	; test if shake timer timeout is expired
			goto	Sup_Mut_Act		; jump to the event handler
			btfsc	tim_expired,1	;check if morale timer has expired
			goto	Morale_Expired	; go to event handler
			btfsc	rc_opcodeflag,1	; check to see if the available command has been recieved from your TOWRP
			goto	Available		; jump to event handler
			goto	EVENTS			; mutiny suppression action was not preformed
			
WAITING_MUT_RES:
			btfss	tim_expired, 7	; test if 5Hz tranmsit is ready
			goto	WAIT_CONT		; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	PORTC,JETTISON_BIT	; check if the jettison crew button is pressed
			goto	Jettison_Crew	; go to event handler
			btfsc	tim_expired,4	; check if the mutiny timer has expired
			goto	Mut_Tim_Exp		; jump to the event handler
WAIT_CONT	btfsc	tim_expired,1	; check if morale timer has expired
			goto	Morale_Expired	; go to event handler
			btfsc	rc_opcodeflag,3	; if received morale update
			goto	CHECK_MORALE	; check if it's -5
WAIT_CONT2	btfsc	rc_opcodeflag,1	; check to see if the available command has been recieved from your TOWRP
			goto	Available		; jump to event handler
			goto	EVENTS			; restart event checker
CHECK_MORALE:
			bcf		rc_opcodeflag,3	; clear opcode flag
			movlw	NEG5MORALE		;
			xorwf	saved_param,w	; Check if morale is -5
			btfss	STATUS,Z		; If morale -5, Z=1
			goto	WAIT_CONT2		; otherwise ignore it

			clrf	coach_state		; clear coach states
			bsf		coach_state,2	; change to paired state
			clrf	tim_velocity	; clear velocity timer
			bsf		tim_enabled,3	; enable velocity timer
			clrf	tim_analog		; clear free analog timer
			bsf		tim_enabled,6	; start free analog timer
			goto	Morale_Update	; Morale was updated

ASSERTING_CMD:
			btfss	tim_expired, 7	; test if 5Hz tranmsit is ready
			goto	AC_CONT			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	PORTB,AOC_BIT	; check if the assertion of command action has been preformed
			goto	Assertion_CMD	; jump to the event handler
AC_CONT		btfsc	rc_opcodeflag,2	; receive AOC result
			goto	Assert_Res		; jump to even handler
			btfsc	rc_opcodeflag,1	; receive "I am available?"	
			goto	Same_Available	; jump to event handler
			btfsc	tim_expired,0	; check if the available timer has expired
			goto	Avail_Tim_Exp	; jump to event handler
			goto	EVENTS			; restart event handler

WAITING_AOC_RES:
			btfss	tim_expired, 7	; test if 5Hz tranmsit is ready
			goto	WAR_CONT			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	tim_expired,2   ; check if the assertion of command timer has expired
			goto	AoC_Tim_Exp		; jump to event handler			
WAR_CONT	btfsc	rc_opcodeflag,2	; check if an assertion of command results was recieved with the proper coach number
			goto	Assert_Res		; jump to event handler
			goto	EVENTS			; jump to event handler
			

;event handler
Morale_Update:						; event handler when a morale update message is recieved
			clrf	tim_morale		; reset morale update timer
			bcf		tim_expired,1	; clear timer flag!
			bcf		rc_opcodeflag,3	; clear opcode flag
			movf	saved_param,W	;	
			movwf	morale			; Move parameter into morale
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; restart event checker

Morale_Expired:						; event handler when the morale timer expires
			bcf		tim_expired,1	; clear morale timer flag
			clrf	coach_state		; clear the current state of the coach
			bcf		tim_enabled,3	; disable the velocity timer if active
			bcf		tim_enabled,1	; disable the morale timer
			bcf		tim_enabled,4	; disable the mutiny timer if active
			bcf		tim_enabled,6	; disable the free analog timer, if active.			
			bsf		coach_state,3	; set coach state to not paired
			movlw	NOMORALE		;
			movwf	morale			; indicate there is no longer morale
			movlw	NOBOAT			;
			movwf	pairedID		; no longer paired
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; restart event checker

Jettison_Crew:						; event handler when the jettison crew action has been preformed
			clrf	coach_state		; clear the current state of the coach
			bsf		coach_state,3	; set coach state to not paired
			bcf		tim_enabled,3	; disable the velocity timer if active
			bcf		tim_enabled,1	; disable the morale timer
			bcf		tim_enabled,4	; disable the mutiny timer if active
			bcf		tim_enabled,6	; disable the free analog timer			
			bsf		tx_opcodeflag,2 ; set the jettison crew transmit flag
			call	TRANSMIT		; compile send
			movlw	NOMORALE		;
			movwf	morale			; indicate there is no longer morale
			movlw	NOBOAT			;
			movwf	pairedID		; no longer paired
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; restart event checker

Available:							; event handler when an "available" message is recieved from TOWRP
			bcf		rc_opcodeflag,1 ; clear the "I am available" flag
			clrf	coach_state		; clear the current state of the coach
			bsf		coach_state,6	; set coach state to Asserting Command
			bcf		tim_enabled,3	; disable the velocity timer if active
			bcf		tim_enabled,1	; disable the morale timer
			bcf		tim_enabled,4	; disable the mutiny timer if active
			bcf		tim_enabled,6	; disable the free analog timer
			clrf	tim_available	; clear the available timer
			bsf		tim_enabled,0	; enable the available timer
			bsf		morale2_disp,AVAIL_BIT	; clear available boat LED
			movlw	NOMORALE		;
			movwf	morale			;
			movf	saved_id,W		; load ID of message sender
			movwf	pairedID		; save as pairedID
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; restart the event checker

Same_Available:
			bcf		rc_opcodeflag,1	; clear opcode flag
			bcf		tim_expired,0	; clear the available timer flag
			clrf	tim_available	; reset the available timer
			goto	EVENTS			; restart event checkers


Mutiny:								; handle the event of a mutiny in progress
			bsf		PORTC,MUTINY_BIT	; Announce mutiny in progress
			bcf		tim_enabled,3	; disable velocity timer
			bcf		tim_enabled,6	; disable free analog timer	
			clrf	coach_state		; clear the coach status
			bsf		coach_state,4	; set the coach state to Mutinous Crew
			bcf		rc_opcodeflag,0 ; clear the mutiny flag
			movlw	NEG10MORALE		;
			movwf	morale			; set morale to -10
			call	UPDATE_DISPLAYS	; update displays
			goto	EVENTS			; restart event handler

Mut_Tim_Exp:						; handle the event of the mutiny timer expiring
			clrf	tim_mutiny		; reset the mutiny timer
			bcf		tim_expired,4	; clear the mutiny timer flag
			bsf		tx_opcodeflag,0 ; set the suppress mutiny command to be sent
			call	TRANSMIT		; compile message and send
			goto	EVENTS			; restart event checker

Sup_Mut_Act:						; handle the event of the suppression of mutiny action being preformed
			clrf	tim_shake_stat	; clear all shake enabled and expired flags
			clrf	tim_shake_to	; also clear the shake timeout timer
			clrf	tim_shake		; clear the shake timer too for good measure
			clrf	tim_mutiny		; clear the mutiny timer
			bsf		tim_enabled,4	; enable the mutiny timer
			bsf		tx_opcodeflag,0 ; set the suppress mutiny command to be sent
			call	TRANSMIT		; compile message and send		
			clrf	coach_state		; clear the current state of the coach
			bsf		coach_state,5	; set the current state to waiting for TOWRP Response
			goto	EVENTS			; restart event handler

Assertion_CMD:						; handle the event when the assertion of command action is preformed
			bcf		assertion_cmd,0	; clear the assertion of command flag
			bcf		tim_expired,2	; clear the AoC expired flag
			clrf	tim_aoc			; clear the AoC timer
			bsf		tim_enabled,2	; enable the AoC timer
			bsf		tx_opcodeflag,1	; set the AoC command transmit flag
			call	TRANSMIT		; compile and send message
			btfsc	coach_state,1	; test if coach is in startup
			goto	EVENTS			; restart event checker
			clrf	coach_state		; clear the current coach status
			bsf		coach_state,7 	; set coach state to waiting for assertion results
			goto	EVENTS			; restart event checker

Avail_Tim_Exp:						; handle the event when the available timer has expired
			bcf		tim_expired,0	; clear the available timer expired flag
			bcf		tim_enabled,2	; disable the AoC timer if active
			bcf		tim_enabled,0	; disable the available timer
			clrf	coach_state		; clear the current coach state
			bsf		coach_state,3	; set coach state to not paired
			bcf		morale2_disp,AVAIL_BIT	; clear available boat LED
			movlw	NOMORALE		;
			movwf	morale			; indicate there is no longer morale
			movlw	NOBOAT			;
			movwf	pairedID		; no longer paired
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; restart the event checker

AoC_Tim_Exp:						; handle the event when the assertion of command timer expires
			bcf		tim_expired,2	; clear the AoC timer expired flag
			clrf	tim_aoc			; clear the assertion of command timer
			bsf		tx_opcodeflag,1 ; set the assertion of command message
			goto	TRANSMIT		; compile and send message
			goto	EVENTS			; restart event checker

Assert_Res:							; handle the event when an assertion of command results was recieved
			bcf		rc_opcodeflag,2	; clear the assertion of command flag
			bcf		tim_enabled,2	; disable the AoC timer if active
			bcf		tim_enabled,0	; disable the available timer
			bcf		tim_enabled,5	; disable the Morale timer if active
			clrf	coach_state		; clear the current coach status
			movf	saved_param,w	;
			bcf		morale2_disp,AVAIL_BIT	; clear available boat LED
			xorlw	OUR_TEAM		; test if our team won the boat
			btfss	STATUS,Z		; Z = 1 if we won
			goto	DID_NOT_WIN		;

			clrf	tim_morale		; reset morale update timer
			bsf		tim_enabled,1	; start the morale timer
			clrf	tim_velocity	; clear velocity timer
			bsf		tim_enabled,3	; start velocity timer
			clrf	tim_analog		; clear free analog timer
			bsf		tim_enabled,6	; start free analog timer	
			bsf		coach_state,2	; change the coach status to paired
			movf	saved_id,W		;
			movwf	pairedID		; store ID of boat just paired with
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; jump back to event checker

DID_NOT_WIN	bsf		coach_state,3	; go to not paired
			movlw	NOMORALE		;
			movwf	morale			; indicate there is no longer morale
			movlw	NOBOAT			;
			movwf	pairedID		; no longer paired
			call	UPDATE_DISPLAYS	; Update displays
			goto	EVENTS			; jump back to event checker

NP_Tim_Exp:							; handle the event when the NP timer has expired
			clrf	tim_np			; reset the NP timer
			bcf		tim_expired,5	; clear the NP timer expired flag
			clrf	coach_state		; clear the current coach status
			bsf		coach_state,3	; set the state of the coach to not paired
			movlw	NOMORALE		;
			movwf	morale			; indicate there is no longer morale
			movlw	NOBOAT			;
			movwf	pairedID		; no longer paired
			call	UPDATE_DISPLAYS	; Update displays
			bcf		tim_enabled,5	; disable the NP timer
			bcf		tim_enabled,2	; disable the AoC timer
			goto	EVENTS			; restart event checker

Free_an:
			bsf		tx_opcodeflag,5	; send free analog command
			call	TRANSMIT		; 
			clrf	tim_analog		; reset counter
			bcf		tim_expired,6	; clear flag
			goto	EVENTS			; restart event checker

Free_dig:
			bsf		tx_opcodeflag,4	; send free digital command
			call	TRANSMIT		;
			goto	EVENTS			; restart event checker

DETECT_SHAKE:
			movf	suppress_analog,w	; analog data to test is saved in suppress_analog every time timer1 overflows
			movwf	temp_shake		; store temporarily for conversion
			sublw	d'58'			; compare analog with lower threshold
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1			  	; If analog is less than 58 (z=0,c=1)...
			btfsc	STATUS,Z
			goto	SHAKE_ACTIVE	; shake is occuring! 

			movf	temp_shake,W	; reload velocity
			sublw	d'214'			; compare analog with upper threshold
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than 214 (z=0,c=1)...
			btfss	STATUS,Z
			goto	SHAKE_ACTIVE
			clrf	tim_shake_stat	; reset the shake timer timeout, because a shake did not happen fast enough	
			return					; accelerometer is within thresholds, so no shaking is occuring

SHAKE_ACTIVE:		
			bcf		tim_shake_stat,0	; clear shake timer expired flag
			clrf	tim_shake			; clear shake timer
			bsf		tim_shake_stat,1	; enable shake timer
			bsf		tim_shake_stat, 3	; enable the shake timer timeout
			return						;

Velocity:
			goto	BUILDVELOCITY	;create the appropriate velocity		DEBUG: call instead of goto (Ed says)
VEL_CONT	bsf		tx_opcodeflag,3 ; send velocity command
			call	TRANSMIT		;
			clrf	tim_velocity	; reset counter
			bcf		tim_expired,3	; clear flag
			goto	EVENTS			; restart event checker		

BUILDVELOCITY:
			movf	vel_analog,w	; Load velocity analog value
			call	CONVERT_ANALOG_VEL	; Convert analog to velocity value
			movwf	velocity		; load speed part of velocity

			movf	dir_analog,w	; Load direction analog value
			call	CONVERT_ANALOG_DIR	;
			iorwf	velocity,f		; raise bits
			goto	VEL_CONT		; continue

CONVERT_ANALOG_VEL:	; thing to convert should be in W
			movwf	temp	; store temporarily

			sublw	d'90'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'11110000'		; ...speed is -100%

			movf	temp,W	; reload velocity
			sublw	d'94'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'11100000'		; ...speed is -89%

			movf	temp,W	; reload velocity
			sublw	d'99'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'11010000'		; ...speed is -75%

			movf	temp,W	; reload velocity
			sublw	d'104'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'11000000'		; ...speed is -59%

			movf	temp,W	; reload velocity
			sublw	d'109'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'10110000'		; ...speed is -45%

			movf	temp,W	; reload velocity
			sublw	d'115'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'10100000'		; ...speed is -29%

			movf	temp,W	; reload velocity
			sublw	d'120'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'10010000'		; ...speed is -15%

			movf	temp,W	; reload velocity
			sublw	d'130'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000000'		; ...speed is 0%

			movf	temp,W	; reload velocity
			sublw	d'135'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00010000'		; ...speed is 15%

			movf	temp,W	; reload velocity
			sublw	d'141'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00100000'		; ...speed is 29%

			movf	temp,W	; reload velocity
			sublw	d'146'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00110000'		; ...speed is 45%

			movf	temp,W	; reload velocity
			sublw	d'152'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'01000000'		; ...speed is 59%

			movf	temp,W	; reload velocity
			sublw	d'157'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'01010000'		; ...speed is 75%

			movf	temp,W	; reload velocity
			sublw	d'161'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'01100000'		; ...speed is 89%

			retlw	b'01110000'				; Else speed is 100%

CONVERT_ANALOG_DIR:	; thing to convert should be in W
			movwf	temp	; store temporarily

			sublw	d'94'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001111'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'100'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001110'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'105'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001101'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'110'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001100'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'115'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001011'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'121'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001010'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'126'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00001001'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'136'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000000'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'140'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000001'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'145'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000010'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'150'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000011'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'154'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000100'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'159'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000101'		; ...this direction.

			movf	temp,W	; reload velocity
			sublw	d'163'			; compare analog with #
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If analog is less than # (z=0,c=1)...
			btfsc	STATUS,Z
			retlw	b'00000110'		; ...this direction.

			retlw	b'00000111'		; Else this direction.

UPDATE_DISPLAYS:
			clrf	boat_disp		; clear boat display register
			clrf	morale1_disp	; clear morale1 display register
			movlw	b'11110011'		; clear negative and ten's digit bits of morale2 display register
			andwf	morale2_disp,f

			movlw	NOBOAT
			xorwf	pairedID,w		; check to make sure boat is paired
			btfsc	STATUS,Z		; If not paired (Z=1)...
			goto	DISP_CONT		; ...skip loading pairedID.

			movf	pairedID,w		; Else load the boat ID
			call	DISP_TABLE		; look up the display byte
			movwf	boat_disp		; save in boat_disp

DISP_CONT	movlw	NOMORALE		;
			xorwf	morale,w		; check if displaying morale is appropriate
			btfsc	STATUS,Z		; If no morale...
			goto	DISP_END		; ...skip loading morale display.

			movlw	d'10'			;
			movwf	temp			; store 10 in temp for comparison later
			movf	morale,w		; load the morale
			btfss	morale,7		; If positive...
			goto	DISP_CONT2		; ...continue.
			sublw	0xff			; Else make positive.
			addlw	0x1				;
			bsf		morale2_disp,3	; and set negative sign bit
DISP_CONT2	xorwf	temp,f			; Check if absolute value of morale is 10
			btfss	STATUS,Z		; If it is not 10 (z=1)...
			goto	DISP_CONT3		; ...continue.
			bsf		morale2_disp,2	; Else display 1 on display 2
			movlw	0x0				; and move 0 into W

DISP_CONT3	call	DISP_TABLE		; look up display byte for morale1
			movwf	morale1_disp	; save

DISP_END	call	SEND_TO_DISP	; output all three to the displays
			return

SEND_TO_DISP:
			btfsc	boat_disp,0		; Put bit 0 onto the boat line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,0
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,0		; Put bit 0 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,0
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,0		; Put bit 0 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,0
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,1		; Put bit 1 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,1
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,1		; Put bit 1 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,1
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,1		; Put bit 1 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,1
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,2		; Put bit 2 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,2
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,2		; Put bit 2 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,2
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,2		; Put bit 2 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,2
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,3		; Put bit 3 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,3
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,3		; Put bit 3 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,3
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,3		; Put bit 3 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,3
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,4		; Put bit 4 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,4
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,4		; Put bit 4 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,4
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,4		; Put bit 4 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,4
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,5	; Put bit 5 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,5
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,5		; Put bit 5 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,5
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,5		; Put bit 5 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,5
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,6		; Put bit 6 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,6
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,6		; Put bit 6 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,6
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,6		; Put bit 6 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,6
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			btfsc	boat_disp,7		; Put bit 7 onto the line
			bsf		PORTC,BOAT_NUM_BIT
			btfss	boat_disp,7
			bcf		PORTC,BOAT_NUM_BIT
			btfsc	morale1_disp,7		; Put bit 7 onto the morale1 line
			bsf		PORTC,MORALE1_BIT
			btfss	morale1_disp,7
			bcf		PORTC,MORALE1_BIT
			btfsc	morale2_disp,7		; Put bit 7 onto the morale2 line
			bsf		PORTC,MORALE2_BIT
			btfss	morale2_disp,7
			bcf		PORTC,MORALE2_BIT
			bsf		PORTC,SHIFT_CLK_BIT	; Set clock high
			nop						; Wait
			bcf		PORTC,SHIFT_CLK_BIT	; Set clock low

			return

			END
;