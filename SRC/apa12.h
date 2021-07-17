//Module for handling the communcation with the RGB led on the board
//it is practically a one way SPI protocol

//Module prefix: apa12led_

//singe inclusion definition
#ifndef __APA12_H
#define __APA12_H


#include "app_error.h"



uint32_t apa12led_init(uint32_t dataPin, uint32_t clockPin);
void apa12led_sub_red(uint8_t amount);
void apa12led_add_red(uint8_t amount);
void apa12led_sub_green(uint8_t amount);
void apa12led_add_green(uint8_t amount);
void apa12led_sub_blue(uint8_t amount);
void apa12led_add_blue(uint8_t amount);
void apa12led_clear(void);
void apa12led_setRed(uint8_t amount);
void apa12led_setGreen(uint8_t amount);
void apa12led_setBlue(uint8_t amount);

//call periodically 
uint32_t apa12led_transfer(void);





#endif //__APA12_H