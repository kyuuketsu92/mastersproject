#include "app_error.h"



#ifndef __WS2812B_H
#define __WS2812B_H

typedef struct{
  uint8_t RED;
  uint8_t GREEN;
  uint8_t BLUE;
} WS2812B_LED;

void ws2812b_init(uint32_t pin);
void ws2812b_shiftout(WS2812B_LED * colours, uint32_t ledCount);

#endif //__WS2812B_H