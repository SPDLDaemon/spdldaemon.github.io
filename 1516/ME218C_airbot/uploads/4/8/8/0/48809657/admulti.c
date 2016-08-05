// ADMulti.c
// Setup up ADC0 to convert up to 4 channels using SS2



#include <stdint.h>
#include "inc/hw_gpio.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "inc/tm4c123gh6pm.h"

#include "ADMulti.h"

static const uint32_t HowMany2Mask[4] = {0x01,0x03,0x07,0x0F};
// this mapping puts PE0 as resuult 0, PE1 as result 1...
static const uint32_t HowMany2Mux[4] = {0x03,0x023,0x123,0x0123};
static const uint32_t HowMany2CTL[4] = {ADC_SSCTL2_END0|ADC_SSCTL2_IE0,
                                        ADC_SSCTL2_END1|ADC_SSCTL2_IE1,
                                        ADC_SSCTL2_END2|ADC_SSCTL2_IE2,
                                        ADC_SSCTL2_END3|ADC_SSCTL2_IE3};

static uint8_t NumChannelsConverting;

// initialize the A/D converter to convert on 1-4 channels
void ADC_MultiInit(uint8_t HowMany){ 
  volatile uint32_t delay;
  uint8_t index = HowMany-1; // index into the HowMany2Mask array
  
  // first sanity check on the HowMany parameter
  if ( (0 == HowMany) || (4 < HowMany))
    return;
  
  NumChannelsConverting = HowMany;
  
  SYSCTL_RCGCADC_R |= 0x00000001; // 1) activate ADC0
  SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4; // 1) activate clock for Port E
  delay = SYSCTL_RCGCGPIO_R;      // 2) allow time for clock to stabilize
  delay = SYSCTL_RCGCGPIO_R;
  GPIO_PORTE_DIR_R &= ~HowMany2Mask[index];      // 3) make PE0, PE1, PE2, PE3 input
  GPIO_PORTE_AFSEL_R |= HowMany2Mask[index];     // 4) enable alternate function on PE0 - PE3
  GPIO_PORTE_DEN_R &= ~HowMany2Mask[index];      // 5) disable digital I/O on PE0 - PE3
  GPIO_PORTE_AMSEL_R |= HowMany2Mask[index];     // 6) enable analog functionality on PE4 PE5
  
  ADC0_PC_R &= ~0xF;              // 8) clear max sample rate field
  ADC0_PC_R |= 0x1;               //    configure for 125K samples/sec
  ADC0_SSPRI_R = 0x3210;          // 9) Sequencer 3 is lowest priority
  ADC0_ACTSS_R &= ~0x0004;        // 10) disable sample sequencer 2
  ADC0_EMUX_R &= ~0x0F00;         // 11) seq2 is software trigger
  ADC0_SSMUX2_R = HowMany2Mux[index];         // 12) set channels for SS2
  ADC0_SSCTL2_R = HowMany2CTL[index];         // 13) set which sample is last
  ADC0_IM_R &= ~0x0004;           // 14) disable SS2 interrupts
  ADC0_ACTSS_R |= 0x0004;         // 15) enable sample sequencer 2
}

//------------ADC_MultiRead------------
// Triggers A/D conversion and waits for result
// Input: none
// Output: up to 4 12-bit result of ADC conversions
// software trigger, busy-wait sampling, takes about 18.6uS to execute
// data returned by reference
// lowest numbered converted channel is in data[0]
void ADC_MultiRead(uint32_t data[4]){ 
  uint8_t i;
  
  ADC0_PSSI_R = 0x0004;               // 1) initiate SS2
  while((ADC0_RIS_R&0x04)==0)
  {};                                 // 2) wait for conversion(s) to complete
  for (i=0; i< NumChannelsConverting; i++){
    data[i] = ADC0_SSFIFO2_R&0xFFF;   // 3) read result, one at a time
  }
  ADC0_ISC_R = 0x0004;                // 4) acknowledge completion, clear int
}
