/* our_c32.c
 * =========
 */

#include <ME218_C32.h>
#include <stdio.h>
#include "ADS12.h"
#include "our_c32.h"

//#define DEBUG

/* SetDDR
 * ======
 * See header file for usage.
 */
char SetDDR(char port, char pin, char direction) {
  static char AD_string[9] = "IIIIIIII";

  char new_AD_string[9];
  char ch;
  char mask;
	  
  if(port > 2 || port < 0) {
  	printf("ERROR: Invalid port.\r\n");
    return ERROR;
  }
  if(port == AD && pin == -1) {
    printf("ERROR: You can't set all the AD pins at once.\r\n");
    return ERROR;
  }
  if(pin < -1 || pin > 7 || (port == M && pin > 5)) {
    printf("ERROR: Invalid pin.\r\n");
    return ERROR;
  }
  if(pin != -1 &&
     ((port != AD && direction != READ && direction != WRITE) ||
      (port == AD && direction != READ && direction != WRITE && direction != ANALOG))) {
  	printf("ERROR: Invalid data direction.\r\n");
  	return ERROR;
  }
  
  if(port == AD) {
    strcpy(new_AD_string, AD_string);
    switch(direction) {
      case READ:
        ch = 'I';
        break;
      case WRITE:
        ch = 'O';
        break;
      case ANALOG:
        ch = 'A';
        break;
    }
    new_AD_string[7-pin] = ch;
    if(ADS12_Init(new_AD_string) == ADS12_OK) {
      strcpy(AD_string, new_AD_string);
      return SUCCESS;
    } else {
      printf("ERROR: problem setting AD port.");
      return ERROR;
    }
  }

  switch(pin) {
  	case 0:
  	  if(direction == READ) {
  	    mask = BIT0LO;
  	  } else {
  	    mask = BIT0HI;
  	  }
  	  break; 
  	case 1:
  	  if(direction == READ) {
  	    mask = BIT1LO;
  	  } else {
  	    mask = BIT1HI;
  	  }
  	  break;   
  	case 2:
  	  if(direction == READ) {
  	    mask = BIT2LO;
  	  } else {
  	    mask = BIT2HI;
  	  }
  	  break;    
  	case 3:
  	  if(direction == READ) {
  	    mask = BIT3LO;
  	  } else {
  	    mask = BIT3HI;
  	  }
  	  break;
  	case 4:
  	  if(direction == READ) {
  	    mask = BIT4LO;
  	  } else {
  	    mask = BIT4HI;
  	  }
  	  break;
  	case 5:
  	  if(direction == READ) {
  	    mask = BIT5LO;
  	  } else {
  	    mask = BIT5HI;
  	  }
  	  break;
  	case 6:
  	  if(direction == READ) {
  	    mask = BIT6LO;
  	  } else {
  	    mask = BIT6HI;
  	  }
  	  break;
  	case 7:
  	  if(direction == READ) {
  	    mask = BIT7LO;
  	  } else {
  	    mask = BIT7HI;
  	  }
      break;
  }
  
  if(port == T) {
    if(pin == -1) {
      DDRT = direction;
    } else if(direction == READ) {
      DDRT &= mask;
    } else if(direction == WRITE) {
      DDRT |= mask;
    }
  } else if(port == M) {
    if(pin == -1) {
      DDRM = direction;
    } else if(direction == READ) {
      DDRM &= mask;
    } else if(direction == WRITE) {
      DDRM |= mask;
    }
  }
  return SUCCESS;
}

/* ReadPort
 * ========
 * See header file for usage.
 */
short ReadPort(char port, char pin) {
  char mask;
  short result = 0;

  if(port > 2 || port < 0) {
  	printf("ERROR: Invalid port.\r\n");
    return ERROR;
  }
  if(pin < -1 || pin > 7 || (port == M && pin > 5)) {
    printf("ERROR: Invalid pin.\r\n");
    return ERROR;
  }
  if(pin == -1 && port == AD) {
    printf("ERROR: Cannot read all on AD port.\r\n");
    return ERROR;
  }
  
  if(port == AD) {
    result = ADS12_ReadADPin(((unsigned char)pin));
    return result;
  }
  
  switch(pin) {
  	case 0:
  	  mask = BIT0HI;
  	  break; 
  	case 1:
  	  mask = BIT1HI;
  	  break;   
  	case 2:
  	  mask = BIT2HI;
  	  break;    
  	case 3:
  	  mask = BIT3HI;
  	  break;
  	case 4:
  	  mask = BIT4HI;
  	  break;
  	case 5:
  	  mask = BIT5HI;
  	  break;
  	case 6:
  	  mask = BIT6HI;
  	  break;
  	case 7:
  	  mask = BIT7HI;
      break;
  }
  
  if(port == T) {
    if(pin == -1) {
      return PTT;
    } else {
      result = PTT & mask;
    }
  } else if(port == M) {
    if(pin == -1) {
      return PTM;
    } else {
      result = PTM & mask;
    }
  }
  return (result != 0);
}

/* WritePort
 * =========
 * See header file for usage.
 */
char WritePort(char port, char pin, char value) {
  char mask;

  if(port > 2 || port < 0) {
  	printf("ERROR: Invalid port.\r\n");
    return ERROR;
  }
  if(pin < -1 || pin > 7 || (port == M && pin > 5)) {
    printf("ERROR: Invalid pin.\r\n");
    return ERROR;
  }
  
  switch(pin) {
  	case 0:
  	  if(value == LO) {
  	    mask = BIT0LO;
  	  } else {
  	    mask = BIT0HI;
  	  }
  	  break; 
  	case 1:
  	  if(value == LO) {
  	    mask = BIT1LO;
  	  } else {
  	    mask = BIT1HI;
  	  }
  	  break;   
  	case 2:
  	  if(value == LO) {
  	    mask = BIT2LO;
  	  } else {
  	    mask = BIT2HI;
  	  }
  	  break;    
  	case 3:
  	  if(value == LO) {
  	    mask = BIT3LO;
  	  } else {
  	    mask = BIT3HI;
  	  }
  	  break;
  	case 4:
  	  if(value == LO) {
  	    mask = BIT4LO;
  	  } else {
  	    mask = BIT4HI;
  	  }
  	  break;
  	case 5:
  	  if(value == LO) {
  	    mask = BIT5LO;
  	  } else {
  	    mask = BIT5HI;
  	  }
  	  break;
  	case 6:
  	  if(value == LO) {
  	    mask = BIT6LO;
  	  } else {
  	    mask = BIT6HI;
  	  }
  	  break;
  	case 7:
  	  if(value == LO) {
  	    mask = BIT7LO;
  	  } else {
  	    mask = BIT7HI;
  	  }
      break;
  }
  
  if(port == T) {
    if(pin == -1) {
      PTT = value;
    } else if(value == LO) {
      PTT &= mask;
    } else if(value == HI) {
      PTT |= mask;
    }
  } else if(port == M) {
    if(pin == -1) {
      PTM = value;
    } else if(value == LO) {
      PTM &= mask;
    } else if(value == HI) {
      PTM |= mask;
    }
  } else if(port == AD) {
    if(pin == -1) {
      PTAD = value;
    } else if(value == LO) {
      PTAD &= mask;
    } else if(value == HI) {
      PTAD |= mask;
    }
  }
  return SUCCESS;
}

//Test harness
#ifdef DEBUG

void main(void) {
  char dummy[1];

/*
  //Test SetDDR
  SetDDR(M, 1, WRITE);
  printf("%x\r\n", DDRM);
  SetDDR(M, 1, READ);
  printf("%x\r\n", DDRM);
  SetDDR(M, 6, WRITE);
  printf("erred?\r\n");
  SetDDR(M, 3, 8);
  printf("erred?\r\n");
  
  printf("\r\n");
  
  SetDDR(T, 4, WRITE);
  printf("%x\r\n", DDRT);
  SetDDR(T, 7, WRITE);
  printf("%x\r\n", DDRT);
  SetDDR(T, -2, WRITE);
  printf("erred?\r\n");
  SetDDR(T, -1, 0x1);
  printf("%x\r\n", DDRT);
  
  printf("\r\n");
  
  SetDDR(AD, -1, WRITE);
  printf("erred?\r\n");
  SetDDR(AD, 1, WRITE);
  SetDDR(AD, 3, ANALOG);
  SetDDR(AD, 1, READ);
*/

/*
  //Test ReadPort
  SetDDR(M, -1, 0x0);
  SetDDR(T, -1, 0x0);
  SetDDR(AD, 0, ANALOG);
  
  printf("%d\r\n", ReadPort(M, 0));
  gets(dummy);
  printf("%d\r\n", ReadPort(M, 5));
  gets(dummy);
  
  printf("\r\n");
  
  printf("%d\r\n", ReadPort(T, 2));
  gets(dummy);
  printf("%d\r\n", ReadPort(T, 6));
  gets(dummy);
  
  printf("\r\n");
  
  printf("%d\r\n", ReadPort(AD, 0));
  gets(dummy);
  printf("%d\r\n", ReadPort(AD, 0));
  gets(dummy);
  printf("%d\r\n", ReadPort(AD, 0));
  gets(dummy);
*/

  //Test WritePort
  SetDDR(M, -1, 0xff);
  SetDDR(T, -1, 0xff);
  SetDDR(AD, 0, WRITE);

  WritePort(M, 1, HI);
  printf("M1 HI\r\n");
  gets(dummy);
  WritePort(M, 1, LO);
  printf("M1 LO\r\n");
  gets(dummy);
  
  WritePort(T, 5, LO);
  printf("T5 LO\r\n");
  gets(dummy);
  WritePort(T, 5, HI);
  printf("T5 HI\r\n");
  gets(dummy);
  WritePort(T, 6, HI);
  printf("T6 HI\r\n");
  gets(dummy);
  WritePort(AD, 0, HI);
  printf("AD0 HI\r\n");
  gets(dummy);
}

#endif