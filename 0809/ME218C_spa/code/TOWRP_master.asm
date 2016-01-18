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
#define		TEAMID			0x06	; our team ID
#define		NOCONTROLLER	0xFF	; our device is not paired
#define		BROADCASTBYTE	0xFF	; constant, XBEE defined
#define		SUPPRESS_MUT	0x00	; opcode, to boat
#define		ASSERT_COMMAND	0x01	; opcode, to boat	
#define		JETTISON_CREW	0x02	; opcode, to boat	
#define		VELOCITY		0x03	; opcode, to boat	
#define		FREE_DIGITAL	0x04	; opcode, to boat	
#define		FREE_ANALOG		0x05	; opcode, to boat	
#define		MUTINY			0x20	; opcode, to controller
#define		AVAILABLE		0x21	; opcode, to controller
#define		ASSERT_RESULT	0x22	; opcode, to controller
#define		UPDATE_MORALE	0x23	; opcode, to controller
#define		ANYTHING		0x2A	; parameter, when it does not matter!
#define		NEG5MORALE		0xCE	; morale = -5, negative 50 deci_morale in two's complement
#define		NEG10MORALE		0x9C	; morale = -10, negative 100 deci_morale in two's complement

; Variable definitions

W_TEMP			equ		0xF0		; for use by interrupt handler, to preserve W, accessible from any bank
STATUS_TEMP		equ		0xF1		; for use by interrupt handler, to preserve STATUS, accessible from any bank
pairedID		equ		0x20		; the address LSB of the paired transmitter
teamcolor		equ		0x21		; stores the team color
bytebox			equ		0x22		; a temp storage location for use by receive function
pairedMSB		equ		0x23		; address MSB of paired device
pairedLSB		equ		0x24		; address LSB of paired device
deci_morale		equ		0x25		; current morale in tenths of a morale point
morale			equ		0x26		; current morale in full points
velocity		equ		0x27		; current velocity (speed and direction)
temp_velocity	equ		0x28		; stores velocity during morale update
temp_morale		equ		0x29		; stores morale during morale conversion

; flags and counters

rc_counter		equ		0x31		; counter to retrieve correct byte from receive table
rc_discardmsg	equ		0x32		; flag set if recieved message was incorrect - prevent save of incoming message
tx_counter		equ		0x33		; counter to retrieve correct byte from transmit table
tx_status		equ		0x34		; bit 0: flag is set when a packet is done transmitting, clear while packet is transmitting
boat_state		equ		0x35		; state indicator, bit 2 is (paired/not paired), bit 1 is (mutinous/normal)
rc_opcodeflag	equ		0x36		; flag indicates which opcode was received
tx_opcodeflag	equ		0x37		; flag indicates which opcode to transmit, once the transmit function is called by a timer

; timers
tim_enabled		equ		0x40		; flag indicates which time is enabled, allows for starting and stopping timers
tim_expired		equ		0x41		; flag indicates which timer has expired, for the state machine!
tim_veltimout 	equ		0x42		; Velocity Timeout Timer (5 seconds), bit 5
tim_mutiny		equ		0x43		; Mutiny Timer (3 seconds), bit 4
tim_velocity	equ		0x44		; Velocity Timer (1 second), bit 3
tim_avail		equ		0x45		; Available Timer (1 second), bit 2
tim_morale		equ		0x46		; Morale Update Timer (<1 second), bit 1
tim_mutinymsg	equ		0x47		; Mutiny Message Timer (0.2 seconds), bit 0
tim_tx_timeout	equ		0x48		; receive timeout timer, bit 7

; transmit/receive data storage

tx_opcode		equ		0x51		; memory
tx_parameter	equ		0x52		; memory
tx_checksum		equ		0x53		; memory
rc_api			equ 	0x54		; memory, not used
rc_sourceMSB	equ		0x55		; memory, not used
rc_sourceLSB	equ		0x56		; memory
rc_rssi			equ		0x57		; memory
rc_options		equ		0x58		; memory
rc_opcode		equ		0x59		; memory
rc_parameter	equ		0x60		; memory
rc_checksum		equ		0x61		; memory

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

BYTE_T1		retlw	START_DEL		;
BYTE_T2		retlw	LENGTHMSB		;
BYTE_T3		retlw	LENGTHLSB		;
BYTE_T4		retlw	TX_API			;
BYTE_T5		retlw	TX_FRAMEID		;
BYTE_T6		movf	pairedMSB,w		; either paired device or broadcast
			return					;
BYTE_T7		movf	pairedLSB,w		; either paired device or broadcast
			return					;
BYTE_T8		retlw	TX_OPTIONS		;
BYTE_T9		movf	tx_opcode,w		; determined in transmit function
			return					;
BYTE_T10	movf	tx_parameter,w	; determined in transmit function
			return					;
BYTE_T11	movf	tx_checksum,w	; calculated
			return					;

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

; error checking is inserted in the table ... flag discardmsg is set if any byte does not match up
; discardmsg flag gets cleared after the save function

BYTE_R1		movf	bytebox, w		; empty byte box into w
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
			xorlw	CONTROLLER		; as a boat, can only receive messages from Controller
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; no need to save it for later access
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					;	
BYTE_R6		movf	bytebox, w		; empty byte box into w
			movwf	rc_sourceLSB	; save incase this is an assertion of command
			movf	pairedID, W		;
			xorlw	NOCONTROLLER	; test if device is unpaired
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear
			return					; if it is, do not set discard message command
			movf	rc_sourceLSB,W	; 
			xorwf	pairedID,W		; if paired, check that it is paired with correct device
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			return					; if it is, do not set discard message command
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
			addlw	CONTROLLER		;
			addwf	rc_sourceLSB,W	;
			addwf	rc_rssi,W		;
			addwf	rc_options,W	;
			addwf	rc_opcode,W		;
			addwf	rc_parameter,W	;
			addwf	rc_checksum,W	;
			xorlw	0xff			; check if the sum totals 0xff
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear
			bsf		rc_discardmsg,0 ; not a valid receive, discard receive
			goto	END_BYTE		;

; interrupt handler

PUSH:		
			movwf	W_TEMP			; save w to temp register
			swapf	STATUS, W		; swap status to be saved into w
			movwf	STATUS_TEMP		; save status to temp register
INT_FLAG:							; determine which interrupt occured
			banksel	PIR1			; go to bank 0
			btfsc	INTCON, T0IF	; timer 0 overflow interrupt flag
			call 	T0_INT			; timer0 ISR
			banksel	PIR1			; go to bank 0
			btfsc	PIR1, TXIF		; transmit interrupt flag													
			call	TX_INT			; transmit ISR
			banksel	PIR1			; go to bank 0
			btfsc	PIR1, RCIF		; receive interrupt flag	
			call	RC_INT			; receive ISR
			banksel	PIR1			; go to bank 0
POP:			
			swapf	STATUS_TEMP, W	; swap status_temp into w
			movwf	STATUS			; restore status to original state
			swapf	W_TEMP, F		; swap w_temp nibbles
			swapf	W_TEMP, W		; swap w_temp again to restore w
			retfie					; return from interrupt	

; transmit interrupt service routine and supporting functions
TX_INT:								; takes 10ms per packet!
			banksel	PIE1			; bank 1
			btfss	PIE1, TXIE		; test if interrupt is enabled 
			return					; transmit interrupts not enabled
			banksel	PWM1CON			; bank 0 - all variables and registers are in bank 0	
			movf	tx_counter,w	; move counter to w							
			call	TX_TABLE		; jump to table
			movwf	TXREG			; write value in w from table to transmit register, also clear interrupt flag
			incf	tx_counter,f	; increment counter
			movlw	TOT_BYTES		; move total bytes to w
			xorwf	tx_counter,w	; compare equality with txcounter
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear
			goto	TX_INT_END		; end routine
			bsf		tx_status,0		; set flag to indicate that packet transmit is completed
			bcf		tim_enabled,6	; disable transmit timeout timer
			banksel	PIE1			; bank 1
			bcf		PIE1, TXIE		; disable transmit interrupt
			banksel	PWM1CON			; bank 0
TX_INT_END	return					; end transmit service routine

;To transmit, call TRANSMIT
;It will check to see if a transmit is currently in progress, if not, it will assemble the message based on tx_opcodeflag
; debug - need to implement transmit priority!!!

TRANSMIT:							; takes 60us to assemble message in preparation for transmit
			btfsc	tx_status,0		; test if ready to transmit new packet
			call	ASSEMBLE_MESG	; assemble and send message
			return					; don't transmit if not ready!
ASSEMBLE_MESG:						; only one flag is ever set at a time
			movlw	CONTROLLER		; this is a boat, so only talk to controllers
			movwf	pairedMSB		;
			movf	pairedID,W		; allows different controllers to pair with our boat!
			movwf	pairedLSB		;

MSG1		btfss	tx_opcodeflag,0 ; 
			goto	MSG2			;
			movlw	MUTINY			;
			movwf	tx_opcode		; save value to tx_opcode!
			movlw	ANYTHING		;
			movwf	tx_parameter	; save value to tx_parameter!
MSG2		btfss	tx_opcodeflag,1 ;
			goto	MSG3			;
			movlw	BROADCASTBYTE	; allows for a message broadcast - careful, this changes MSB and LSB
			movwf	pairedMSB		;
			movwf	pairedLSB		;
			movlw	AVAILABLE		; 
			movwf	tx_opcode		; save value to tx_opcode!
			movlw	ANYTHING		;
			movwf	tx_parameter	; save value to tx_parameter!
MSG3		btfss	tx_opcodeflag,2 ;
			goto	MSG4			;
			movlw	BROADCASTBYTE	; allows for a message broadcast - careful, this changes MSB and LSB
			movwf	pairedMSB		;
			movwf	pairedLSB		;			
			movf	pairedID,W		; move paired controller value into w
			movwf	tx_parameter	; save value to tx_parameter!
			movlw	ASSERT_RESULT	;
			movwf	tx_opcode		; save value to tx_opcode!
MSG4		btfss	tx_opcodeflag,3 ;
			goto	PRESENDIT		;
			movlw	UPDATE_MORALE	;			
			movwf	tx_opcode		; save value to tx_opcode!
			goto	DECIMORALE_TO_MORALE ; convert decimorale to morale
MSG4_CONT	movf	morale,W		; get morale
			movwf	tx_parameter	; save value to tx_parameter!

PRESENDIT	goto	CALC_TXCHECKSUM	; calculate the checksum
SENDIT		clrf	tx_opcodeflag	; clear all flags for transmit	
			clrf	tx_counter		; set counter to 0	
			bcf		tx_status,0		; clear flag to indicate that packet transmit is now in progress
			bsf		tim_enabled,6	; enable transmit timeout timer
			banksel	PIE1			; bank 1
			bsf		PIE1, TXIE		; transmit interrupt enable	- this starts send of message
			banksel	PWM1CON			; bank 0
			return					;

CALC_TXCHECKSUM:					; called from ASSEMBLE_MESG, calcs the checksum for the transmit
			movlw	0				; Start off with 0
			addlw	TX_API			; Add required bytes together, ignoring overflow
			addlw	TX_FRAMEID		;
			addwf	pairedMSB,W		;
			addwf	pairedLSB,W		;
			addlw	TX_OPTIONS		;
			addwf	tx_opcode,W		;
			addwf	tx_parameter,W	;
			sublw	0xff			; subtract sum of bytes from 0xff
			movwf	tx_checksum		; store in tx_checksum
			goto	SENDIT			;

; receive interrupt service routine and supporting functions

RC_INT:								; takes 11us to receive a packet		
			btfsc	rc_discardmsg,0	; test if discard flag is set
			goto	START_MSG		; if discard flag is set, start a new msg by clearing the counter and the flag 
			goto	RC_INT_CONT		; within bounds, continue
START_MSG	clrf	rc_counter		; clear receive counter for a new message
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
			goto 	RC_PACKETDONE	; total bytes has been reached! perform end of packet procedure
RC_INT_END	return					; end receive service routine
				
RC_PACKETDONE:
			clrf	rc_counter		; set counter to 0
			btfss	rc_discardmsg,0	; test to see if discard message flag was set
			goto	SAVERCDATA		; save the data that came in
			bcf		rc_discardmsg,0	; reset discard message flag for next round
			goto	RC_INT_END		; ends packet without saving data
SAVERCDATA:			
									; set appropriate flag by identifying the data that was read!
			clrf	rc_opcodeflag	; clear opcodes because they're not always cleared elsewhere - DEBUG: any reason not to do this?

RC_SM		movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	SUPPRESS_MUT	; test what the value is, and send to correct place in code
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear	
			bsf		rc_opcodeflag,0	; set bit to indicate mutiny suppression, disregard parameter bit
									 
RC_AC		movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	ASSERT_COMMAND	; test what the value is, and send to correct place in code
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	RC_JC			;
			bsf		rc_opcodeflag,1	; set bit to indicate command assertion
			movf	rc_parameter, w	; empty received byte 10 into W
			movwf	teamcolor		; save parameter as team color	
			movf	rc_sourceLSB,W	; import msgsourceLSB
			movwf	pairedID		; save a new team ID, because command was asserted! DEBUG - really want to do this here?

RC_JC		movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	JETTISON_CREW	; test what the value is, and send to correct place in code
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear	
			bsf		rc_opcodeflag,2	; set bit to indicate jettison crew, disregard parameter bit
									 
RC_V		movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	VELOCITY		; test what the value is, and send to correct place in code
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	RC_FD			;
			bsf		rc_opcodeflag,3	; set bit to indicate velocity
			movf	rc_parameter, w	; empty received byte 10 into W					
			movwf	velocity		; save parameter bit as velocity!
RC_FD		movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	FREE_DIGITAL	; test what the value is, and send to correct place in code
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear	
			bsf		rc_opcodeflag,4	; set bit to indicate free digital, disregard parameter bit
									; 
RC_FA		movf	rc_opcode, w	; empty received byte 9 into W
			xorlw	FREE_ANALOG		; test what the value is, and send to correct place in code
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	RC_INT_END		; ends packet having saved data!
			bsf		rc_opcodeflag,5	; set bit to indicate free analog
									; save parameter bit as analog scaling!
			goto	RC_INT_END		; ends packet having saved data!

									; this interrupt requires 13us to update the timers when one is enabled, 16us when three are enabled
T0_INT:								; when enabling a timer, the appropriate overflow must be cleared a the same time (this is not handled here)
			banksel	PWM1CON			; bank 0
			bcf		INTCON, T0IF	; clear timer 0 flag
									; NOTE: removed "if no timer's enabled, skip" stuff because TX_TIMEOUT always enabled, but never explicitly

TIMER_TX_TIMEOUT:
			incf	tim_tx_timeout,f	; increment counter, always on
			movf	tim_tx_timeout,w	;
			xorlw	0x08			; 8 overflows, .2 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_VELTIMEOUT	;
			clrf	tim_tx_timeout	; reset counter
			bsf		tim_expired,7	; set flag!

TIMER_VELTIMEOUT:
			btfss	tim_enabled, 5	; test if timer is enabled, otherwise skip
			goto	TIMER_MUTINY	;
			incf	tim_veltimout,f	; increment counter	
			movf	tim_veltimout,w	;
			xorlw	0xBF			; 191 overflows, 5 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_MUTINY	;
			clrf	tim_veltimout	; reset counter
			bsf		tim_expired, 5	; set flag!
TIMER_MUTINY:		
			btfss	tim_enabled, 4	; test if timer is enabled, otherwise skip
			goto	TIMER_VELOCITY	;
			incf	tim_mutiny,f	; increment counter
			movf	tim_mutiny,w	;
			xorlw	0x75			; 117 overflow, 3 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_VELOCITY	;
			clrf	tim_mutiny		; reset counter
			bsf		tim_expired, 4	; set flag!
TIMER_VELOCITY:	
			btfss	tim_enabled, 3	; test if timer is enabled, otherwise skip
			goto	TIMER_AVAIL		;
			incf	tim_velocity,f	; increment counter	
			movf	tim_velocity,w	;
			xorlw	0x27			; 39 overflows, 1 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_AVAIL		;
			clrf	tim_velocity	; reset counter
			bsf		tim_expired, 3	; set flag!
TIMER_AVAIL:
			btfss	tim_enabled, 2	; test if timer is enabled, otherwise skip
			goto	TIMER_MORALE	;
			incf	tim_avail,f		; increment counter		
			movf	tim_avail,w	;
			xorlw	0x27			; 39 overflows, 1 second 
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_MORALE	; 
			clrf	tim_avail		; reset counter
			bsf		tim_expired, 2	; set flag!
TIMER_MORALE:
			btfss	tim_enabled, 1	; test if timer is enabled, otherwise skip
			goto	TIMER_MUTINYMSG	;
			incf	tim_morale,f	; increment counter		
			movf	tim_morale,w	;
			xorlw	0x08			; 8 overflows, .2 seconds (updates in fifths of a point)
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	TIMER_MUTINYMSG	;
			clrf	tim_morale		; reset counter
			bsf		tim_expired,1	; set flag!
TIMER_MUTINYMSG:
			btfss	tim_enabled, 0	; test if timer is enabled, otherwise skip
			goto	END_T0			;		
			incf	tim_mutinymsg,f	; increment counter			
			movf	tim_mutinymsg,w	;
			xorlw	0x08			; 8 overflows, .2 seconds
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	END_T0			;
			clrf	tim_mutinymsg	; reset counter
			bsf		tim_expired, 0	; set flag!
END_T0		return					;


; main code begins here
INITIALIZE:
CLEARALL:
			clrf	PORTA			; clear to initialize
			clrf	PORTB			; clear to initialize
			clrf	PORTC			; clear to initialize
			clrf	W_TEMP			; clear to initialize
			clrf	STATUS_TEMP		; clear to initialize
			clrf	pairedID		; clear to initialize
			clrf	teamcolor		; clear to initialize
			clrf	bytebox			; clear to initialize
			clrf	pairedMSB		; clear to initialize
			clrf	pairedLSB		; clear to initialize
			clrf	deci_morale		; clear to initialize
			clrf	velocity		; clear to initialize
			clrf	temp_velocity	; clear to initialize
			clrf	rc_counter		; clear to initialize
			clrf	rc_discardmsg	; clear to initialize
			clrf	tx_counter		; clear to initialize
			clrf	tx_status		; clear to initialize			
			clrf	boat_state		; clear to initialize	
			clrf	rc_opcodeflag	; clear to initialize
			clrf	tx_opcodeflag	; clear to initialize
			clrf	morale			; clear to initialize
			clrf	tim_enabled		; clear to initialize
			clrf	tim_expired		; clear to initialize
			clrf	tim_veltimout	; clear to initialize
			clrf	tim_mutiny		; clear to initialize
			clrf	tim_velocity	; clear to initialize
			clrf	tim_avail		; clear to initialize
			clrf	tim_morale		; clear to initialize
			clrf	tim_mutinymsg	; clear to initialize
			clrf	tim_tx_timeout	; clear to initialize
			clrf	tx_opcode		; clear to initialize
			clrf	tx_parameter	; clear to initialize
			clrf	tx_checksum		; clear to initialize
			clrf	rc_api			; clear to initialize
			clrf	rc_sourceMSB	; clear to initialize
			clrf	rc_sourceLSB	; clear to initialize
			clrf	rc_rssi			; clear to initialize
			clrf	rc_options		; clear to initialize
			clrf	rc_opcode		; clear to initialize
			clrf	rc_parameter	; clear to initialize
			clrf	rc_checksum		; clear to initialize

INITPORTS:
			banksel ANSEL			; bank 2
			clrf	ANSEL			; digital I/O
			clrf	ANSELH			; digital I/O	
			banksel	TRISA			; bank 1 or 3
			clrf	TRISA			; output (team0) on port 0, output (team1) on port 1
			clrf	TRISB			; output (SSP clock out) on 6, output (XBee transmit) on 7
			bsf		TRISB, 5		; input (XBee receive)
			clrf	TRISC			; output on all of PortC (includes SSP data out and state indicator LEDs)
INITASYNC:
			banksel	SPBRG			; bank 1
			movlw	BAUDRATE		; get baud rate
			movwf	SPBRG			; sets baud rate
			bcf		TXSTA, SYNC		; clear to enable asynchronous serial port (part 1 of 2)
			bsf		TXSTA, TXEN		; enable transmission
			banksel	RCSTA			; bank 0
			bsf		RCSTA, SPEN 	; enable asynchronous serial port (part 2 of 2)
			bsf		RCSTA, CREN		; enable reception
INITSSP:
			banksel SSPSTAT       	; bank 1
    		clrf    SSPSTAT       	; SMP = 0, CKE = 0
    		banksel	SSPCON		   	; bank 0
    		movlw   0x31          	; Set up SPI port, Master mode, OSC = CLK, CLK/16, = 625kHz,	00110001
    	 	movwf   SSPCON        	; Enable SSP, CKP = 1	
INITTIM0:
			banksel	OPTION_REG		; bank 1 or 3
			bcf		OPTION_REG,T0CS ; use internal clock
			bcf		OPTION_REG,PSA	; use 8 bit prescaler
			bsf		OPTION_REG,PS2	; set prescaler assignment 1:256
			bsf		OPTION_REG,PS1	; set prescaler assignment 1:256
			bsf		OPTION_REG,PS0	; set prescaler assignment 1:256
INIT_INT:
			bsf		INTCON,GIE		; global interrupt enable
			bsf		INTCON,PEIE	; peripheral interrupt enable
			bsf 	INTCON,T0IE	; enable Timer0 Overflow Interrupt
			banksel	PIE1			; bank 1
			bsf		PIE1,TMR2IE		; Timer 2 interrupt enable
			bsf		PIE1,RCIE		; receive interrupt enable
MAIN:
			banksel	PWM1CON			; bank 0
			bsf		tx_status,0		; indicates ready for new transmission
			movlw	NOCONTROLLER	; indicates that our device is not paired
			movwf	pairedID		; to wait for assertion of command
			movlw	0x02			; indicates that no team LED should be lit
			movwf	teamcolor		; saves team color

			bsf		tx_opcodeflag,1	; broadcast "I am available"
			call 	TRANSMIT		; 
			clrf	tim_avail		; Clear available timer
			bsf		tim_enabled, 2	; Start available timer
			bcf 	boat_state,2	; Set State as Not Paired
			bcf		boat_state,1	; set as normal
			goto	EVENTS			; go to Event 


; everything below here is the event checker and event handler

; event checker

EVENTS:
			btfsc	boat_state,2	; Test paired or not paired, and goto									
			goto	PAIRED_EVENTS	; 
			goto	UNPAIRED_EVENTS ;

UNPAIRED_EVENTS:					; DYSFUNCTIONAL: ASSERTION OF COMMAND (is discarding all msgs that it receives ...)
			bcf		PORTC,0
			bsf		PORTC,1			; indicate state with LEDs
			bcf		PORTC,2			; indicate state with LEDs
			btfss	tim_expired,7	; test if 5Hz tranmsit is ready
			goto	EVENTS			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	rc_opcodeflag,1	; receive "Assertion of Command"?	
			goto	RC_Assertion	; go to event handler
			btfsc	tim_expired,2	; Available Timer expires 
			goto	TEX_Available	; go to event handler					 
			goto	EVENTS			; restart event checker
										
PAIRED_EVENTS:						; FUNCTIONAL: ALL!!!
			btfss	tim_expired,7	; test if 5Hz tranmsit is ready
			goto	PAIRED_CONT		; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	tim_expired, 5	; Velocity Timeout Timer expires
			goto	TEX_Velocity_Timeout	; go to event handler
			btfsc	rc_opcodeflag,2	; receive "Jettison Crew Command"?
			goto	RC_Jettison_Crew		; go to event handler
			bsf		tim_expired,7	; If got here, flag must have been expired and nothing transmitted. Need to re-flag for next set of events.

PAIRED_CONT	btfsc	boat_state,1	; Test normal or mutiny, and goto
			goto	MUTINY_EVENTS	;
			goto	NORMAL_EVENTS	;

NORMAL_EVENTS:						; FUNCTIONAL: ALL.
			bcf		PORTC,0			; indicate state with LEDs
			bcf		PORTC,1			; indicate state with LEDs
			bsf		PORTC,2			; indicate state with LEDs
			movf	deci_morale,W	; move deci morale counter to w
			xorlw	NEG10MORALE		; morale = -10 (100 deci_morales)
			btfsc	STATUS, Z		; if equal, z bit is set, else it is clear			
			goto	Minus10morale	; go to event handler
			btfsc	tim_expired,1	; Morale Update Timer expires
			goto	EX_Morale_Update		; go to event handler
			btfsc	tim_expired,3	; Velocity Timer expires
			goto	EX_Velocity			; go to event handler	
			
			btfss	tim_expired,7	; test if 5Hz tranmsit is ready
			goto	EVENTS			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	rc_opcodeflag,3	; receive “Velocity”?
			goto	RC_Velocity		; go to event handler		
			goto	EVENTS			; restart event checker	

MUTINY_EVENTS:						; FUNCTIONAL: ALL!!!
			bsf		PORTC,0			; debug leds
			bcf		PORTC,1			; indicate state with LEDs debug
			bcf		PORTC,2			; indicate state with LEDs debug
			btfss	tim_expired,7	; test if 5Hz tranmsit is ready
			goto	EVENTS			; don't enter transmit
			bcf		tim_expired,7	; clear flag
			btfsc	rc_opcodeflag,0	; receive “Suppress Mutiny” 
			goto	RC_Supress_Mut	; go to event handler
			btfsc	tim_expired,4	; Mutiny Timer expires
			goto	EX_Mutiny		; go to event handler
			btfsc	tim_expired,0	; Mutiny Message Timer expires
			goto	EX_Mutiny_Msg	; go to event handler
			goto	EVENTS			; restart event checker

;event handler

RC_Assertion:						; store coach/team color already done in message decode
			call	UPDATE_TEAM_COLOR	; light the correct LEDs for what team we are on
			bsf		boat_state,2	; set as PAIRED
			bcf		boat_state,1	; set as NORMAL						
			clrf	deci_morale		; set morale = 0									
			bsf		tx_opcodeflag,2	; broadcast "Assertion of Command Result"
			call 	TRANSMIT		; compile send
			bcf		tim_enabled, 2	; disable available timer
			call 	STARTNORMALPAIREDTIMERS	;
			bcf		rc_opcodeflag,1	; clear flag!
			goto	EVENTS			; restart event checker
TEX_Available:
			clrf	tim_avail		; reset timer
			bsf		tx_opcodeflag,1	; broadcast "I am available"
			call 	TRANSMIT		; compile send
			bcf		tim_expired,2	; clear flag!
			goto	EVENTS			; restart event checker
TEX_Velocity_Timeout:
			call	LEAVINGPAIRED	; set of shared commands
			bcf		tim_expired,5	; clear flag!
			goto	EVENTS			; restart event checker	
RC_Jettison_Crew:
			call	LEAVINGPAIRED	; set of shared commands
			bcf		rc_opcodeflag,2	; clear flag!
			goto	EVENTS			; restart event checker		
Minus10morale:
			bsf		boat_state,2	; set as PAIRED
			bsf		boat_state,1	; set as MUTINY
			bcf		tim_enabled, 5	; disable velocity time out timer
			bcf		tim_enabled, 3	; disable velocity timer
			bcf		tim_enabled, 1	; disable morale update timer
			clrf	tim_mutiny		; clear mutiny timer
			bsf		tim_enabled,4	; start mutiny timer
			clrf	tim_mutinymsg	; clear mutiny message timer
			bsf		tim_enabled,0	; start mutiny message timer
			banksel	SSPBUF			; bank 0
			clrf	velocity		; velocity = 0
			movf	velocity,W		; data to be transmitted via SSP
			movwf	SSPBUF			; start transmission
			goto	EVENTS			; restart event checker
RC_Velocity:
			clrf	tim_velocity	; restart velocity timer
			clrf	tim_veltimout	; restart velocity time out timer
			banksel	SSPBUF			; bank 0
			movf	velocity,W		; data to be transmitted via SSP
			movwf	SSPBUF			; start transmission to update motor speeds
			bsf		tx_opcodeflag,3	; send "Update Morale"
			call 	TRANSMIT		; compile send
			bcf		rc_opcodeflag,3	; clear flag!
			goto	EVENTS			; restart event checker
EX_Morale_Update:
			call	HANDLE_MORALE	; go to the handle morale function 
			clrf	tim_morale		; reset morale update timer
			bcf		tim_expired,1	; clear flag!
			goto	EVENTS			; restart event checker
EX_Velocity:
			banksel	SSPBUF			; bank 0
			clrf	velocity		; velocity = 0
			movf	velocity,W		; data to be transmitted via SSP
			movwf	SSPBUF			; start transmission
			bcf		tim_expired,3	; clear flag!
			goto	EVENTS			; restart event checker
EX_Mutiny:
			call	LEAVINGPAIRED	; set of shared commands
			bcf		tim_expired,4	; clear flag!
			goto	EVENTS			; restart event checker
RC_Supress_Mut:
			bsf		boat_state,2	; set as PAIRED
			bcf		boat_state,1	; set as NORMAL
			bcf		tim_enabled,4	; disable mutiny timer
			bcf		tim_enabled,0	; disable mutiny message timer
			call 	STARTNORMALPAIREDTIMERS	;
			movlw	NEG5MORALE		; set morale = -5
			movwf	deci_morale		; save in morale count
			bsf		tx_opcodeflag,3	; send "Update Morale"
			call 	TRANSMIT		; compile send
			bcf		rc_opcodeflag,0	; clear flag!
			goto	EVENTS			; restart event checker
EX_Mutiny_Msg:
			bsf		tx_opcodeflag,0	; send “Mutiny in Progress”
			call 	TRANSMIT		; compile send
			bcf		tim_expired,0	; clear flag!
			goto	EVENTS			; restart event checker


; below are functions that are dedicated to the event handler, to make code more readable

UPDATE_TEAM_COLOR:
RED			movf	teamcolor,w	; load team color into w
			xorlw	0x00			; if team 0, then light red LED
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	BLUE			;
			bsf		PORTA, 0		;
			bcf		PORTA, 1		;
			return					;
BLUE		movf	teamcolor,w	; load team color into w
			xorlw	0x01			; if team 1, then light blue LED
			btfss	STATUS, Z		; if equal, z bit is set, else it is clear	
			goto	NEITHER			;
			bcf		PORTA, 0		;
			bsf		PORTA, 1		;
			return					;
NEITHER		bcf		PORTA, 0		;
			bcf		PORTA, 1		;
			return					;

STARTNORMALPAIREDTIMERS:
			clrf	tim_veltimout	; clear velocity timeout timer									
			bsf		tim_enabled,5	; start velocity timeout timer
			clrf	tim_velocity	; clear velocity timer
			bsf		tim_enabled,3	; start velocity timer
			clrf	tim_morale		; clear morale timer									
			bsf		tim_enabled,1	; start morale update timer
			return					;
	
LEAVINGPAIRED:
			movlw	0x02			; indicates that no team LED should be lit
			movwf	teamcolor		; saves team color
			call	UPDATE_TEAM_COLOR	; light the correct LEDs for what team we are on
			bcf		boat_state,2	; set as NOT PAIRED
			movlw	NOCONTROLLER	; move no controller number to w
			movwf	pairedID		; save pairedID as not paired
			banksel	SSPBUF			; bank 0
			clrf	velocity		; velocity = 0
			movf	velocity,W		; data to be transmitted via SSP
			movwf	SSPBUF			; start transmission
			bcf		tim_enabled, 5	; disable velocity time out timer			
			bcf		tim_enabled, 4	; disable mutiny timer
			bcf		tim_enabled, 3	; disable velocity timer
			bcf		tim_enabled, 1	; disable morale update timer
			bcf		tim_enabled, 0	; disable mutiny message timer
			clrf	tim_avail		; clear available timer
			bsf		tim_enabled, 2	; start available timer
			bsf		tx_opcodeflag,1	; broadcast "I am available"
			call 	TRANSMIT		; compile send
			return					; 

HANDLE_MORALE:
			banksel	PWM1CON			; bank 0
			movf	velocity,w
			andlw	b'01110000'		; only care about 3 velocity bits
			movwf	temp_velocity	; store temporarily
			bcf		STATUS,C		; so doesn't get moved into register
			rrf		temp_velocity,f	; move bits to right
			rrf		temp_velocity,f	; move bits to right
			rrf		temp_velocity,f	; move bits to right
			rrf		temp_velocity,f	; move bits to right
			movf	temp_velocity,w	; load into W

			sublw	0x3				; compare speed with 011 (45%)
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If speed is less than 45% (z=0,c=1)...
			btfsc	STATUS,Z
			goto	UP_MORALE_2		; ...increase morale by 1/5

			movf	temp_velocity,w	; reload speed
			sublw	0x5				; compare speed with 101 (75%)
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If speed is less than 75% (z=0,c=1)...
			btfsc	STATUS,Z
			goto	MORALE_CONTINUE	; ...don't change morale

			movf	temp_velocity,w	; reload speed
			sublw	0x7				; compare speed with 111 (100%)
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If speed is less than 100% (z=0,c=1)...
			btfsc	STATUS,Z
			goto	DOWN_MORALE_2	; ...decrease morale by 1/5

			goto	DOWN_MORALE_4	; Else decrease morale by 2/5
UP_MORALE_2:
			movlw	0x2				; add 0.2 morale (0x2)
			addwf	deci_morale,f
			goto	MORALE_CONTINUE	; continue
DOWN_MORALE_2:
			movlw	0x2				; subtract 0.2 morale (0x2)
			subwf	deci_morale,f
			goto	MORALE_CONTINUE	; continue
DOWN_MORALE_4:
			movlw	0x4				; subtract 0.4 morale (0x4)
			subwf	deci_morale,f
MORALE_CONTINUE:
			btfss	deci_morale,7	; If decimorale is positive...
			goto	POS_TEST		; ...only do positive test

NEG_TEST	movf	deci_morale,w	; load deci_morale
			sublw	NEG10MORALE		; compare deci_morale with -100 (two's complement)
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If deci_morale is less than -100 (z=0,c=1)...
			btfsc	STATUS,Z
			goto	MORALE_NEG10	; ...set deci_morale to -100
			return

POS_TEST	movlw	0x64			; load 100
			subwf	deci_morale,w	; compare 100 with deci_morale
			movf	STATUS,w		; need to check STATUS bits
			andlw	0x5				; only care about Z and C
			xorlw	0x1				; If 100 is less than deci_morale (z=0,c=1)...
			btfsc	STATUS,Z
			goto	MORALE_10		; ...set deci_morale to 100
			return					; Else return

MORALE_NEG10 movlw	NEG10MORALE		; load -100 (two's complement)
			goto	MORALE_CONT		; continue
MORALE_10	movlw	0x64			; load 100
MORALE_CONT	movwf	deci_morale		; overwrite decimorale
			return					;


DECIMORALE_TO_MORALE:
			banksel	PWM1CON			; bank 0
			movlw	0x0				;
			movwf	morale			; initialize morale to 0
			movf	deci_morale,w	;
			movwf	temp_morale		; store so as not to modify deci_morale
			btfsc	deci_morale,7	; If negative value...
			goto	NEG_CONVERSION	; ...go to negative conversion routine
POS_CONVERSION:						; Else do positive conversion routine
			movlw	0x0A			;
			subwf	temp_morale,f	; subtract 10
			btfsc	temp_morale,7	; If negative...
			goto	MSG4_CONT		; ...continue.
			incf	morale,f		; Else add 1 to morale number
			goto	POS_CONVERSION	; loop
			
NEG_CONVERSION:
			movlw	0x0A			;
			addwf	temp_morale,f	; add 10
			btfsc	STATUS,Z		; If zero...
			goto	NEG_CONVERSION2	; ...keep looping
			btfss	temp_morale,7	; If positive...
			goto	MSG4_CONT		; ...continue.
NEG_CONVERSION2:
			decf	morale,f		; (Else) subtract 1 from morale number
			goto	NEG_CONVERSION	; loop

			END
;