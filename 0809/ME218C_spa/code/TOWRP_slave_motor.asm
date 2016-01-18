	list P=PIC16F690
#include "p16F690.inc"
	__config (_CP_OFF & _WDT_OFF & _PWRTE_ON & _INTOSCIO & _MCLRE_OFF)

; Constants
#define			MOTOR_PERIOD	d'250'	; ~20ms in PR2 ticks
#define			MOTOR_BIT		0x5		; Bit on PORTC for motor
#define			VEL_0		d'12'	; 0% velocity value (~1ms)
#define			VEL_15		d'14'	; 15% velocity value (~1.15ms)
#define			VEL_29		d'16'	; 29% velocity value (~1.29ms)
#define			VEL_45		d'18'	; 45% velocity value (~1.45ms)
#define			VEL_59		d'20'	; 59% velocity value (~1.59ms)
#define			VEL_75		d'22'	; 75% velocity value (~1.75ms)
#define			VEL_89		d'24'	; 89% velocity value (~1.89ms)
#define			VEL_100		d'25'	; 100% velocity value (~2ms)

; Variable definitions
W_TEMP			equ		0xF0		; for use by interrupt handler, to preserve W, accessible from any bank
STATUS_TEMP		equ		0xF1		; for use by interrupt handler, to preserve STATUS, accessible from any bank
rxdata			equ		0x21		; variable to store received data in!
temp_file		equ		0x22		; temporary file NOT FOR USE IN INTERRUPTS
motor_width		equ		0x23		; width of motor signal

; begin code

			org 0					;
			goto 	INITIALIZE		; initialization routine!
			org 4					;
			goto	PUSH			; interrupt routine!
			org 5					;

VEL_TABLE
			addwf	PCL,f			; lookup speed
			retlw	VEL_0		; 000 (0%)
			retlw	VEL_15		; 001 (15%)
			retlw	VEL_29		; 010 (29%)
			retlw	VEL_45		; 011 (45%)
			retlw	VEL_59		; 100 (59%)
			retlw	VEL_75		; 101 (75%)
			retlw	VEL_89		; 110 (89%)
			retlw	VEL_100		; 111 (100%)

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
			btfsc	PORTC,MOTOR_BIT			; if was high...
			goto	MOTOR_SET_LO	; ...time to send lo signal
MOTOR_SET_HI:
			bsf		PORTC,MOTOR_BIT			; set port hi
			movf	motor_width,w	; ...else time to send hi signal
			goto	MOTOR_INT_CONTINUE	; continue
MOTOR_SET_LO:
			bcf		PORTC,MOTOR_BIT			; set port lo
			banksel PR2				; bank ?
			movf	PR2,w			; subtract PR2 from period (20 ms) to get low time
			sublw	MOTOR_PERIOD	;
MOTOR_INT_CONTINUE:
			banksel PR2				; bank ?
			movwf	PR2				; set time for next interrupt
			banksel	PIR1			; go to bank 0
			bcf		PIR1,TMR2IF
			return



INITIALIZE:
INIT_CLOCK:
			banksel	OSCCON			; bank 1
			bsf		OSCCON, IRCF2	; 8MHz, part 1 of 3
			bsf		OSCCON, IRCF1	; 8MHz, part 2 of 3
			bsf		OSCCON, IRCF0	; 8MHz, part 3 of 3
			bsf		OSCCON, SCS	; select internal oscillator as system clock

INIT_VARS:
			banksel	PWM1CON			; bank 0
			movlw	VEL_0
			movwf	motor_width

INIT_PORTS:
			banksel ANSEL			; bank 2
			clrf	ANSEL			; digital I/O
			clrf	ANSELH			; digital I/O
			banksel	TRISC			; bank 1 or 3
			bcf		TRISC,MOTOR_BIT	; set to output for motor
			bsf		TRISB, 4		; input (SSP data in)
			bsf		TRISB, 6		; input (SSP clock in)
			banksel	PORTC
			clrf	PORTC
			
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
			bsf		T2CON,TOUTPS3	; postscaler to 10
			bsf		T2CON,TOUTPS0	; postscaler to 10
			bsf		T2CON,T2CKPS1	; prescaler to 16 (8 MHz / 4 / 16 / 10 = 12.5 KHz; each PR2 tick = 80 us)
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
			call	SET_VELOCITY	;
			goto	MAIN			; repeat

SET_VELOCITY:
			banksel	PWM1CON			; bank 0
			movf	rxdata,W
			movwf	temp_file		; store temporarily
			movlw	VEL_0
			btfsc	temp_file,7		; If negative velocity...
			goto	VEL_CONTINUE	; ...don't move.

			movlw	b'01110000'
			andwf	temp_file,f		; only care about three velocity bytes
			bcf		STATUS,C		; don't want C shifting into byte
			rrf		temp_file,f		; shift into LSB spaces for use with table lookup
			rrf		temp_file,f		;
			rrf		temp_file,f		;
			rrf		temp_file,w		;

			call	VEL_TABLE

VEL_CONTINUE:
			movwf	motor_width		; load into motor_width
			return

			END
;

