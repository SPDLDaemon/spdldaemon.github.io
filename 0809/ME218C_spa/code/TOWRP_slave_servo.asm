	list P=PIC16F690
#include "p16F690.inc"
	__config (_CP_OFF & _WDT_OFF & _PWRTE_ON & _HS_OSC & _MCLRE_OFF)

; Constants
#define			SERVO_FREQ		d'240'	; ~20ms in PR2 ticks
#define			SERVO_CENTER	d'18'	; ~1.5ms in PR2 ticks
#define			SERVO_BIT		0x1		; Bit on PORTA for servo

; Variable definitions
W_TEMP			equ		0xF0		; for use by interrupt handler, to preserve W, accessible from any bank
STATUS_TEMP		equ		0xF1		; for use by interrupt handler, to preserve STATUS, accessible from any bank
rxdata			equ		0x21		; variable to store received data in!
temp_file		equ		0x22		; temporary file NOT FOR USE IN INTERRUPTS
servo_width		equ		0x23		; width of servo signal

; begin code

			org 0					;
			goto 	INITIALIZE		; initialization routine!
			org 4					;
			goto	PUSH			; interrupt routine!
			org 5					;

PUSH:
			banksel W_TEMP			; bank 0
			movwf	W_TEMP			; save w to temp register
			swapf	STATUS, W		; swap status to be saved into w
			movwf	STATUS_TEMP		; save status to temp register

			banksel	PIR1			; go to bank 0
			btfsc	PIR1,TMR2IF		; Timer 2 interrupt
			call	TMR2_INT		; Timer 2 ISR
POP:
			swapf	STATUS_TEMP, W	; swap status_temp into w
			movwf	STATUS			; restore status to original state
			swapf	W_TEMP, F		; swap w_temp nibbles
			swapf	W_TEMP, W		; swap w_temp again to restore w
			retfie					; return from interrupt	

TMR2_INT:
			btfsc	PORTA,SERVO_BIT			; if was high...
			goto	SERVO_SET_LO	; ...time to send lo signal
SERVO_SET_HI:
			bsf		PORTA,SERVO_BIT			; set port hi
			movf	servo_width,w	; ...else time to send hi signal
			goto	SERVO_INT_CONTINUE	; continue
SERVO_SET_LO:
			bcf		PORTA,SERVO_BIT			; set port lo
			banksel PR2				; bank ?
			movf	PR2,w			; subtract PR2 from period (20 ms) to get low time
			sublw	SERVO_FREQ		;
SERVO_INT_CONTINUE:
			banksel PR2				; bank ?
			movwf	PR2				; set time for next interrupt
			banksel	PIR1			; go to bank 0
			bcf		PIR1,TMR2IF
			return

; this slave is running on the same clock as the master PICs (10MHz)

INITIALIZE:
INIT_VARS:
			movlw	SERVO_CENTER
			movwf	servo_width

INIT_PORTS:
			banksel ANSEL			; bank 2
			clrf	ANSEL			; digital I/O
			clrf	ANSELH			; digital I/O
			banksel	TRISA			; bank 1 or 3
			bcf		TRISA,SERVO_BIT	; set to output for servo
			banksel	PORTA
			clrf	PORTA
			
INITSSP:
			banksel SSPSTAT       	; bank 1
    		clrf    SSPSTAT       	; SMP = 0, CKE = 0
    		banksel	SSPCON		   	; bank 0
    		movlw   0x35          	; Set up SPI port, Slave mode, clock = SCK pin, notSS disabled		00110101
    	 	movwf   SSPCON        	; Enable SSP, CKP = 1

INIT_TMR2:
			banksel	PIR1			; bank 0
			bcf		PIR1,TMR2IF		; clear Timer 2 flag
			banksel	T2CON			; bank ?
			bsf		T2CON,TOUTPS3	; postscaler to 13
			bsf		T2CON,TOUTPS2	; postscaler to 13
			bsf		T2CON,T2CKPS1	; prescaler to 16 (10 MHz / 4 / 16 / 13 = 12.0192308 KHz; each PR2 tick = 0.832 us)
			bsf		T2CON,TMR2ON	; turn on Timer 2

INIT_INT:
			bsf		INTCON, GIE		; global interrupt enable
			bsf		INTCON, PEIE	; peripheral interrupt enable
			banksel	PIE1			; bank 1
			bsf		PIE1,TMR2IE		; Timer 2 interrupt enable

MAIN:
			banksel	SSPSTAT			; bank 1
			btfss  	SSPSTAT, BF   	; Has data been received(transmit complete)?
     		goto    MAIN          	; No
     		banksel SSPBUF	    	; Bank 0
    		movf   	SSPBUF, W     	; W = contents of SSPBUF
    		movwf  	rxdata        	; Save in user RAM, if data is meaningful, rxdata is now also in W
			call	TURN			;
			goto	MAIN			; repeat

TURN:
			movf	rxdata,W
			movwf	temp_file		;
			andlw	0x7				; only care about 3 LSBs
			btfss	temp_file,3		; If positive rotation...
			goto	TURN_CCW		; ...turn CCW
TURN_CW:							; ...else turn CW
			sublw	SERVO_CENTER	;subtract from center value
			goto	TURN_CONTINUE
TURN_CCW:
			addlw	SERVO_CENTER	; add to center value
TURN_CONTINUE:
			movwf	servo_width		; store new width
			return

			END
;

