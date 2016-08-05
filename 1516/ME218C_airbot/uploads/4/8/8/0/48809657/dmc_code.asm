;
; DMC Code
;

list P = PIC12F72
#include "p12F752.inc"
__config(_MCLRE_OFF&_CP_OFF & _WDTE_OFF & _PWRTE_ON & _FOSC0_INT )

;
;	Defines
;

	PAIRING_INPUT	equ		RA0
;	CLOCK_PIN		equ		RA1
	TAIL_WAG_PIN	equ		RA2
	COLOR_INPUT		equ		RA3
	RED_LED_PIN		equ		RA4
	BLUE_LED_PIN	equ 	RA5
	


;
;	Variable Definitions
;

	WREG_TEMP		equ		0x70
	PCLATH_TEMP		equ		0x71
	STATUS_TEMP		equ		0x72
	WAG_STATE		equ		0x40
	UNPAIR_FLAG		equ		0x41
	TIMER_INDEX		equ		0x42
	CLK_DC_VALUE	equ		0x43
	CLK_DC_COPY		equ		0x44

					org		0
					goto	Main
					org		4
					goto	ISR
					org		5
					;

;
; Main Code
;

Main:		

	banksel	OSCCON		
	BCF		OSCCON, IRCF0		;	Change clk to 31 kHz
	BCF		OSCCON,	IRCF1	

	banksel	ANSELA				;	Select the bank that contains TRISA
	BCF		ANSELA, ANSA0		;	Set PAIRING_INPUT pin as digital
	BCF		ANSELA, ANSA4		;	Set RED_LED_PIN as digital
	BCF		ANSELA, ANSA5		;	Set BLUE_LED_PIN as digital
	BCF		ANSELA, ANSA2		;	Set TAIL_WAG_PIN as digital

	banksel	TRISA
	BSF		TRISA,	TRISA0		;	set PAIRING_INPUT pin as input
	BSF		TRISA,	TRISA3		;	set COLOR_INPUT pin as input
	BCF		TRISA, 	TRISA4		;	set RED_LED_PIN as output
	BCF		TRISA,	TRISA5		;	set BLUE_LED_PIN as output

	banksel PORTA				;	select porta
	clrf	PORTA				;	init port
	
	banksel LATA
	clrf	LATA
	
	call	PWM_INIT
	
	;	Set up TMR0
	banksel	OPTION_REG
	BCF		OPTION_REG, T0CS	;	Set the TMR0 clk source to internal instruction clock
	BCF		OPTION_REG, PS0		;	Set TMR0 prescaler to 1:64
	BSF		OPTION_REG, PS1
	BSF		OPTION_REG, PS2
	BCF		OPTION_REG,	PSA		;	Assign prescaler bit to TMR0
	BCF		INTCON, T0IF		;	Clear TMR0 overflow flag
	BSF		INTCON, T0IE		;	Enable TMR0 interrupt
	BSF		INTCON, PEIE		;	enable the peripheral interrupt
	BSF		INTCON,	GIE			;	enable the global interrupt

	banksel	TIMER_INDEX
	MOVLW	0x15				;	21 in decimal
	MOVWF	TIMER_INDEX

	MOVLW	0x24				;	36 in decimal
	MOVWF	CLK_DC_VALUE


LOOP: 
	banksel	PORTA
	BTFSC	PORTA,PAIRING_INPUT		;	Check if we are paired
	GOTO	PAIRED_STATE			;	Go to PAIRED_STATE if we are paired
	GOTO	UNPAIRED_STATE			;	Go to UNPAIRED_STATE if we are unpaired

PWM_INIT:
	
	;disable CCP1 pin output driver
 	banksel	TRISA
	BSF		TRISA,	TRISA2			;	set TRISA2 pin to disable output driver
	;load PR2 register with period 
	banksel	PR2
	MOVLW	b'10000110'				;	Set PWM period to 20ms (value of 154)
	MOVWF	PR2
	banksel	CCP1CON					;	config CCP1 module for PWM model and loads Period in CCPR1L
	MOVLW	b'00101111'				
	MOVWF	CCP1CON					
	banksel	CCPR1L
	MOVLW	b'00001101'
	MOVWF	CCPR1L					;	Load CCPR1L register and DC1B<1:0> with DC value
	;Config and start timer 2
	call	TMR2_INIT
	;Enable PWM output pin
	banksel	TRISA
	BCF		TRISA,	TRISA2
	return
	

TMR2_INIT:
	banksel	PIR1
	BCF		PIR1, TMR2IF		;	clear TMR2IF interrupt flag of PIR1
	;config T2CKPS bits of T2CON register w/ timer prescale value (default 1:1)
	banksel	T2CON
	BSF		T2CON, TMR2ON		;	enable timer by setting TMR2ON bit of T2CON register
	banksel	PIE1
	BSF		PIE1, TMR2IE		;	enable the TMR2 interuupt
	return

PAIRED_STATE:
	BTFSC	PORTA,	COLOR_INPUT		
	call	BLUE_ON
	BTFSS	PORTA,	COLOR_INPUT
	call	RED_ON
	BCF		UNPAIR_FLAG, 0		;	clear the unpaired flag
	banksel	OPTION_REG
	MOVLW	b'11010101'			;	change tmr0 prescaler to 64
	MOVWF	OPTION_REG
	Goto	LOOP
	

UNPAIRED_STATE:
	BCF		PORTA,	RED_LED_PIN
	BCF		PORTA,	BLUE_LED_PIN
	BSF		UNPAIR_FLAG, 0		;	Set the unpaired flag
	MOVLW	0x24				;	36 in decimal
	MOVWF	CLK_DC_VALUE
	banksel	OPTION_REG
	MOVLW	b'11010010'			;	change tmr0 prescaler to 8
	MOVWF	OPTION_REG
	Goto	LOOP

BLUE_ON:
	BSF 	PORTA,	BLUE_LED_PIN
	BCF		PORTA,	RED_LED_PIN
	return

RED_ON:
	BSF 	PORTA,	RED_LED_PIN
	BCF		PORTA,	BLUE_LED_PIN
	return

WAG_TAIL:
	BTFSC	WAG_STATE, 0
	goto	WAG_POS2
	BTFSS	WAG_STATE, 0
	goto	WAG_POS1
DONE_WAGGING:
	return
	
WAG_POS1:
	banksel	CCP1CON
	BSF		CCP1CON, DC1B0	;	change angle to 45 degrees
	BSF		CCP1CON, DC1B1
	banksel	CCPR1L
	MOVLW	b'00001001'
	MOVWF	CCPR1L
	banksel	WAG_STATE
	BSF		WAG_STATE, 0
	goto	DONE_WAGGING

WAG_POS2:
	banksel	CCP1CON
	BCF		CCP1CON, DC1B0	;	change angle to 135 degrees
	BSF		CCP1CON, DC1B1
	banksel	CCPR1L
	MOVLW	b'00001101'
	MOVWF	CCPR1L
	banksel	WAG_STATE
	BCF		WAG_STATE, 0
	goto	DONE_WAGGING

CheckUnpairFlag:
	BCF		INTCON,	T0IF		;	Clear the TMR0 interrupt flag
	banksel	UNPAIR_FLAG
	BTFSC	UNPAIR_FLAG, 0
	call 	WAG_TAIL
	BTFSS	UNPAIR_FLAG, 0
	call	TICK_TIMER
	return

TICK_TIMER:
	;banksel	TIMER_INDEX
	;DECFSZ	TIMER_INDEX
	call	ChangeClkDuty
	return
	
ChangeClkDuty:
	banksel	CLK_DC_VALUE
	INCF	CLK_DC_VALUE


	;	Prepare New Duty cycle value
	MOVF	CLK_DC_VALUE, w
	MOVWF	CLK_DC_COPY
	RRF		CLK_DC_COPY
	RRF		CLK_DC_COPY
	BCF		CLK_DC_COPY, 6
	BCF		CLK_DC_COPY, 7


	;	Load New Duty Cycle value
	BTFSC	CLK_DC_VALUE, 0
	BSF		CCP1CON, DC1B0
	BTFSS	CLK_DC_VALUE, 0
	BCF		CCP1CON, DC1B0
	BTFSC	CLK_DC_VALUE, 1
	BSF		CCP1CON, DC1B1
	BTFSS	CLK_DC_VALUE, 1
	BCF		CCP1CON, DC1B1
	MOVF	CLK_DC_COPY, w
	MOVWF	CCPR1L	
	return
	
ISR:

PUSH:
	movwf		WREG_TEMP			;	save WREG
	movf		STATUS,	w			;	store STATUS in WREG
	clrf		STATUS				;	change to file register bank0
	movwf		STATUS_TEMP			;	save STATUS value
	movf		PCLATH,	w			;	store PCLATH in WREG
	movwf		PCLATH_TEMP			;	save PCLATH value
	clrf		PCLATH				;	change to program memory page0

ISR_BODY:							;	ISR body
	BCF			INTCON, GIE			;	Disable global interrupts
	banksel 	PIR1
	BTFSC		PIR1, TMR2IF		;	Check if TMR2 flag is active
	BCF			PIR1, TMR2IF
	BTFSC		INTCON, T0IF		;	Check the TMR0 interrupt flag
	call		CheckUnpairFlag


POP:
	clrf 		STATUS				;	select bank 0
	movf		PCLATH_TEMP, w		;	store saved PCLATH value in WREG
	movwf		PCLATH				;	restore PCLATH
	movf		STATUS_TEMP, w		; 	store saved STATUS value in WREG
	movwf		STATUS				;	restore STATUS
	swapf		WREG_TEMP, f		;	prepare WREG to be restored
	swapf		WREG_TEMP, w		;	restore WREG keeping STATUS bits
	retfie							;	return from interrupt (will reenable global interrupts)


END
