//#define TEST
/****************************************************************************
 Copyright (C) Creative Applications Engineering 2004.
 Permission granted to freely use this file as long as this header remains
 intact.

 Description
         Library to provide simplified setup of Port AD and the Analog
         to Digital converter on the '9S12.

 Notes


 History
 When           Who	What/Why
 -------------- ---	--------
 05/02/07 20:49 jec  corrected definition of FreezeMode
 01/29/05 10:05 jec  modified slightly for the E128
 06/22/04 20:03 jec  first pass
****************************************************************************/

/*----------------------------- Include Files -----------------------------*/

#include <hidef.h>         /* common defines and macros */
#include <mc9s12e128.h>     /* derivative information */
#include <S12e128bits.h>    /* bit definitions  */
#include <ads12.h>
#include <string.h>

/*----------------------------- Module Defines ----------------------------*/
#define NumBitsPortAD 8
#define BadADNum 8
/* BaseIndex is used to read mode string right to left */
#define BaseIndex (NumBitsPortAD-1)
/*set the A/D converter to finish conversion when entering freeze mode */
#define ADFreezeMode _S12_FRZ1
/*shift to get the number of channels into the right bits to program ADCTL3 */
#define ChanShift 3
/*shift to get the sample clocks into the right bits to program ADCTL4 */
#define ADSampShift 5
/* for ADSampTime 0= 2clks, 1= 4 clks, 2= 8 clks, 3= 16 clks */
#define ADSampTime 0x00
/* legal values for ADClockDiv are even values 2-64 */
#define ADClockDiv 12
#define ADClockDivBits ((ADClockDiv-2)>>1)

/*------------------------------ Module Types -----------------------------*/
/*---------------------------- Private Functions ---------------------------*/
/*---------------------------- Module Variables ---------------------------*/
/* initializer on FirstADPin needs to be illegal value */
static unsigned char FirstADPin = BadADNum;
static unsigned char NumADChannels=0;

/*------------------------------ Module Code ------------------------------*/
/****************************************************************************
 Function
         ADS12_Init

 Parameters
         char [9] A null terminated string of 8 ASCII characters to describe
            the mode of each of the pins on Port AD. the legal values are:
            I for digital input, O for digital output, A for analog input.
            The string positions, reading left to right, corresspond to the
            pins MSB to LSB (modeString[0]=MSB, modeString[7]=LSB)

 Returns
         ADS12_Err if the input string is malformed
         ADS12_OK if the mode string was OK

 Description
         Initializes Port AD data direction register & ATDDIEN for digital I/O
         and the A/D converter to multi-channel, continious conversion.

 Notes
         Assumes a 24MHz bus clock, but simply sets the defualt values for now.
         Enforces a single block of A/D channels even though it is possible to
         make a contigious sequence that occupies a non-contigious block. For
         example, channels 6,7,0,& 1 are sequence contigious but occupy 2 blocks
         so would be rejected by this code.

 Author
         J. Edward Carryer, 06/22/04 20:15
****************************************************************************/
ADS12ReturnTyp ADS12_Init(char * modeString)
{
    unsigned char i;
    unsigned char OutputPins = 0;
    unsigned char InputPins = 0;

    if ( strlen(modeString) != NumBitsPortAD)
        return ADS12_Err;
    FirstADPin  = BadADNum;  /* clear values from any previous call */
    NumADChannels = 0;
   /* scan through the string and find the type of each pin, preparing the
      mode programming values as we go */
    for ( i=0; i< NumBitsPortAD; i++)
    {      /* step through the bits in the string, right to left */
        switch ( modeString[BaseIndex-i] )
        {
            case 'I' :  InputPins |= 0x01<<i;  /* add bit to mask */
                        break;
            case 'O' :  OutputPins |= 0x01<<i;  /* add bit to mask */
                        break;
            case 'A' :  if ( FirstADPin == BadADNum)/* no AD defined yet */
                        {
                            FirstADPin = i;
                            NumADChannels++;
                        }else  /* AD pin sequence has started be sure its OK */
                        { /* this test for non-contigious assume we are scanning
                             right to left through the string */
                            if ( i != (FirstADPin + NumADChannels))
                                return ADS12_Err;  /* AD pins not contiguious */
                            else
                                NumADChannels++;
                        }
                        break;
            default :   return ADS12_Err;
        }
    }
    if ( NumADChannels != 0)  /* if we requested A/D channels, set them up */
    {
    /* power up the a/d converter */
        ATDCTL2 |= _S12_ADPU;
    /* set up conversion sequnce length based on number of A/D channels
      requested. Disable the FIFO mode. set Freeze mode behavior */
        ATDCTL3 = ((NumADChannels << ChanShift) | ADFreezeMode);
    /* set up for 10 bit mode with #defined sample times & ADClock divisor */
        ATDCTL4 = (ADSampTime << ADSampShift) | ADClockDivBits;
    /* set up for right justified unsigned data, continuous conversions */
        ATDCTL5 = _S12_DJM | _S12_SCAN | _S12_MULT | FirstADPin ;
    }

    /* the '9S12C32 Device User Guide says to program ATDDIEN for all bits
       that will be 'standard I/O' It doesn't make sense to me to program
       the input buffer for output pins, but that's what I'll do, 'cause
       they said to... */
    ATDDIEN1 = ( InputPins | OutputPins);
    /* set the DDR for any output pins */
    DDRAD = OutputPins ;

   return ADS12_OK;
}


/****************************************************************************
 Function
         ADS12_ReadADPin

 Parameters
         char PinNumber   the Port AD pin number to read the analog value from

 Returns
         the result (10 bits right justified) if the pin number is legal,
         -1 otherwise.

 Description
         reads and returns a/d results from the appropriate registers.

 Notes
         None.

 Author
         J. Edward Carryer, 06/22/04 22:09
****************************************************************************/
short ADS12_ReadADPin( unsigned char pinNumber)
{
    volatile unsigned int *pADResult = &ATDDR0;

    if ((pinNumber >= BadADNum) ||
        (pinNumber > (FirstADPin + NumADChannels)) ||
        (pinNumber < FirstADPin))
        return (-1);
    else
        return (*(pADResult + (pinNumber-FirstADPin)));
}

/*------------------------------- Footnotes -------------------------------*/
#ifdef TEST
#include <stdio.h>
#include <timers12.h>

void main(void)
{
    unsigned char OutValues = 0;

    puts("Starting Port A/D library test \r\n");
    puts("Bit 0-3 are Ana, bits 4&5 are digital in, bits 6&7 digital out\r\n");

    if (ADS12_ReadADPin(0) >= 0 )
        puts("Read before init succeeded, this is a problem\r\n");
    else
        puts("Read before init failed, as it should\r\n");

    if (ADS12_Init("AAIIAAAA") == ADS12_OK)
        puts("Init in 2 analog blocks succeeded, this is a problem\r\n");
    else
        puts("Init in 2 analog blocks failed, as it should\r\n");

    if (ADS12_Init("OOIIAAAAA") == ADS12_OK)
        puts("Init with too many pins succeeded, this is a problem\r\n");
    else
        puts("Init with too many pins failed, as it should\r\n");

    if (ADS12_Init("OOIIAAAi") == ADS12_OK)
        puts("Init with bad pin mode succeeded, this is a problem\r\n");
    else
        puts("Init with bad pin mode failed, as it should\r\n");

    if (ADS12_Init("OOIIAAAA") == ADS12_OK)
        puts("Init with good mode succeeded, as it should\r\n");
    else
        puts("Init with good mode failed, this is a problem\r\n");

    TMRS12_Init(TMRS12_RATE_8MS);
    while (1)
    {
        /* cycle through the 4 values on the output pins*/
        PTAD = (OutValues %4)<<6;
        printf(" Ch0 = %d, Ch1 = %d, Ch2 = %d, Ch3 = %d, Input = %x, Output = %x \r",
                 ADS12_ReadADPin(0), ADS12_ReadADPin(1), ADS12_ReadADPin(2),
                 ADS12_ReadADPin(3), PTIAD & (BIT4HI | BIT5HI),
                 PTAD & (BIT6HI | BIT7HI) );
        OutValues++;
        TMRS12_InitTimer(0, 61); /* 0.5s w/ 8.19mS interval */
        while(TMRS12_IsTimerExpired(0) != 1)
            ;
    }
}
#endif /* TEST */
/*------------------------------ End of file ------------------------------*/
