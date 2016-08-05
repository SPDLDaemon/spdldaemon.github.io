#ifndef ADMULTI
#define ADMULTI
// ADMulti.h
// Setup up ADC0 to convert up to 4 channels using SS2

#include <stdint.h>

// initialize the A/D converter to convert on 1-4 channels
void ADC_MultiInit(uint8_t HowMany);

//------------ADC_MultiRead------------
// Triggers A/D conversion and waits for result
// Input: none
// Output: up to 4 12-bit result of ADC conversions
// software trigger, busy-wait sampling, takes about 18.6uS to execute
// data returned by reference
// lowest numbered converted channel is in data[0]

void ADC_MultiRead(uint32_t data[4]);
#endif
