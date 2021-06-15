#include "app_error.h"



#ifndef __ADCDRIVER_H
#define __ADCDRIVER_H

typedef void (*adc_callback)(uint16_t dataOut);


uint32_t adc_init(uint32_t dataOutPin, uint32_t clockPin, uint32_t dataInPin, adc_callback handler);
uint32_t adc_read(uint8_t channel);
#endif //__ADCDRIVER_H