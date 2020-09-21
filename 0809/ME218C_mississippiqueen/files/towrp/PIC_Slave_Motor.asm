;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;; PIC_Slave_Motor.asm
;; PIC Assembly code for the slave PICs of ME218C's
;; Spring 09 quarter project.
;; Written by: Andrew Parra
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

    list 	P=PIC16F690
    #include 	"p16F690.inc"
    __config 	(_CP_OFF & _WDT_OFF & _PWRTE_ON & _INTOSC & _MCLRE_OFF)

TEMP_1		equ	0x20
TEMP_2      equ 0x21
TX_DATA		equ 0x22
RX_DATA     equ 0x23
STATUS_TEMP	equ 0x24
W_TEMP		equ 0x25
RX_SPI_DATA equ 0x26
DUTY_CYCLE  equ 0x27

#define SSPCON_CONFIG   b'00110100'         ;Bit 7: (0) No write collision detect bit
                                            ;Bit 6: (0) No Receive overflow bit
                                            ;Bit 5: (1) In SPI Mode
                                            ;Bit 4: (1) Idle state for clock is a low level
                                            ;Bit 3-0: (0100) SPI Slave Mode, Clock = SCK Pin.  SS pin control enabled.
#define SSPSTAT_CONFIG  b'00000000'         ;Bit 7: (0) Input data sampled at middle of data output time
                                            ;Bit 6: (0) Data transmitted rising edge of SCK
#define TRISB_CONFIG    b'01111111'         ;Bit 7: (0) Output Tx
                                            ;Bit 6: (1) Input SCK

#define TRISC_CONFIG    b'01000000'         ;Bit 6: (1) Input to SS

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
#define	PR2_CONFIG	b'01100011'	; PR2 = 99 --> 625 Hz PWM Period

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; TEST #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;#define TEST_WITH_LEDS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; END TEST #DEFINES
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

            org	0
            GOTO INIT
            org	4
            GOTO ISR

ISR:
	MOVWF	W_TEMP
	SWAPF	STATUS, W
	MOVWF	STATUS_TEMP
ISR_BEGIN:
	banksel	PIR1
	BTFSC	PIR1, SSPIF     ;We received a message from the master
	CALL	HANDLE_SPI_MSG
END_ISR:
	SWAPF	STATUS_TEMP, W
	MOVWF	STATUS
	SWAPF	W_TEMP, F
	SWAPF	W_TEMP, W
	RETFIE

HANDLE_SPI_MSG:
    banksel SSPSTAT
LOOP_UNTIL_MSG_IN:
    BTFSS   SSPSTAT, BF   ;Has data been received(transmit complete)?
    GOTO    LOOP_UNTIL_MSG_IN          ;No
    banksel SSPBUF
    MOVF    SSPBUF, W     ;WREG reg = contents of SSPBUF
    MOVWF   RX_SPI_DATA   ;Save in user RAM, if data is meaningful
#ifdef TEST_WITH_LEDS
    MOVLW   b'00000011'
    ANDWF   RX_SPI_DATA, W
    MOVWF   PORTC
#endif
    CALL    PWM_DUTY
    RETURN

PWM_DUTY:
	; On receive interrupt, move the byte to the duty cycle register
	banksel		PIR1
	movf		RX_SPI_DATA, W
	movwf		DUTY_CYCLE

	; Handle direction first; we only want positive duty cycles
	btfsc		DUTY_CYCLE, 7
	goto		DUTY_NEG
DUTY_POS:
	; Set the direction registers for positive motion
	bcf		PORTC, 4
	movlw		CCP1CON_FWD
	movwf		CCP1CON

	; The value of duty cycle is already positive, so just pass it along
	goto		SET_8_BITS
DUTY_NEG:
	; Set the direction registers for negative motion
	bsf		PORTC, 4
	movlw		CCP1CON_REV
	movwf		CCP1CON

	; Take the 2's complement of the duty cycle to make it positive
	comf		DUTY_CYCLE, F
	movlw		1
	addwf		DUTY_CYCLE, F

	; Now we can pass along this value of the duty cycle
SET_8_BITS:
	; we don't need to check for duty cycle being <= 100
	; The way the PIC's PWM works, duty cycle will be truncated at 100

	; For the drive motors, we just need to pass through the duty cycle
	movf		DUTY_CYCLE, W
	movwf		CCPR1L
    RETURN

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Initialization Sequence
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
INIT:
    CALL    INIT_PORTS
    CALL    INIT_SPI
    CALL    PWM_INIT
    banksel INTCON
	BSF		INTCON, GIE		;Enable global interrupts
	BSF		INTCON, PEIE	;Enable peripheral interrupts
    banksel PORTC
    CLRF    PORTC
    GOTO    MAIN

INIT_PORTS:
	banksel	ANSEL			;BANK 2
	CLRF	ANSEL			;Set all ports to be digital IO
	CLRF	ANSELH
	banksel TRISA			;BANK 1
	MOVLW	b'00000000'
	MOVWF	TRISA			;Set Port A0-A2 as inputs, A3-A7 as outputs
	MOVLW	TRISC_CONFIG
	MOVWF	TRISC			;Set Port C0-C7 as outputs
    MOVLW   TRISB_CONFIG
    MOVWF   TRISB           ;Set Port B0-B7 as inputs
    RETURN

INIT_SPI:
	banksel SSPCON			;BANK 0
	MOVLW 	SSPCON_CONFIG
	MOVWF 	SSPCON
	banksel	SSPSTAT	        ;BANK 1
	MOVLW	SSPSTAT_CONFIG
	MOVWF	SSPSTAT	 	
    BSF     PIE1, SSPIE     ;Enable SSP interrupt (optional)
    RETURN

PWM_INIT:
	; Minimize bank selects by starting in bank 1
	banksel		PR2

	; Put the initialization byte into PR2
	movlw		PR2_CONFIG
	movwf		PR2

	; Make the PWM pin and direction pin output pins
	bcf		TRISC, 5
	bcf		TRISC, 4

	; Now move to bank 2 to turn off analog from the direction pin
	banksel		ANSELH
	CLRF        ANSELH
    CLRF        ANSEL

	; All other PWM registers are in bank 0
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
LOOP:
	GOTO	LOOP		;Loop around to continue testing
FINISHED:
	END