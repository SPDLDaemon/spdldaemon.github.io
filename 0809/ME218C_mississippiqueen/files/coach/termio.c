#include <termio.h>
#include <mc9s12e128.h>     /* derivative information */
#include <s12sci.h>

char TERMIO_GetChar(void) {
  /* receives character from the terminal channel */
    while (!(SCI0SR1 & _S12_RDRF))
    {}; /* wait for input */
    return SCI0DRL;
}

void TERMIO_PutChar(char ch) {
  /* sends a character to the terminal channel */
    while (!(SCI0SR1 & _S12_TDRE))
    {};  /* wait for output buffer empty */
    SCI0DRL = ch;
}

void TERMIO_Init(void) {
  /* initializes the communication channel */
/* set baud rate to 115.2 kbaud and turn on Rx and Tx */

        SCI0BD = 13;
        SCI0CR2 = (_S12_TE | _S12_RE);
}

int kbhit(void) {
  /* checks for a character from the terminal channel */
    if (SCI0SR1 & _S12_RDRF)
            return 1;
    else
            return 0;

}


