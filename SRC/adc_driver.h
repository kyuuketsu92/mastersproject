//Module for handling the communcation with the SPI connected to 

//Module prefix: adc_

//singe inclusion definition
#ifndef __ADCDRIVER_H
#define __ADCDRIVER_H

#include "app_error.h"

///////////////////////////////////////////////////////////
// DEFINITIONS, TYPEDEFS, ENUMS
///////////////////////////////////////////////////////////

//callback for the data collected 
typedef void (*adc_callback)(uint16_t dataOut);

///////////////////////////////////////////////////////////
// EXTERNAL FUNCTIONS
///////////////////////////////////////////////////////////

//initialisation of the 
uint32_t adc_init(uint32_t dataOutPin, uint32_t clockPin, uint32_t dataInPin, adc_callback handler);
uint32_t adc_read(uint8_t channel);
#endif //__ADCDRIVER_H