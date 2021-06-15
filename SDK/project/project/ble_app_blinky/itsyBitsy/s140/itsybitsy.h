/**
 * Copyright (c) 2016 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
#ifndef itsy_H
#define itsy_H

#ifdef __cplusplus
extern "C" {
#endif

#include "nrf_gpio.h"

// LEDs definitions for PCA10056
#define LEDS_NUMBER 1

#define LED_1         NRF_GPIO_PIN_MAP(0,6)

#define DOT_DATA      NRF_GPIO_PIN_MAP(0,8)
#define DOT_CLK       NRF_GPIO_PIN_MAP(1,9)

#define LEDS_ACTIVE_STATE 1

#define LEDS_LIST { LED_1}

#define LEDS_INV_MASK  LEDS_MASK

#define BSP_LED_0      LED_1

#define BUTTONS_NUMBER 1

#define BUTTON_1       NRF_GPIO_PIN_MAP(0,29) //AIN5
#define BUTTON_PULL    NRF_GPIO_PIN_PULLUP

#define BUTTONS_ACTIVE_STATE 0

#define BUTTONS_LIST { BUTTON_1}

#define BSP_BUTTON_0   BUTTON_1

#define A0              NRF_GPIO_PIN_MAP(0,4) //AIN2
#define A1              NRF_GPIO_PIN_MAP(0,30)//AIN6
#define A2              NRF_GPIO_PIN_MAP(0,28)//AIN4
#define A3              NRF_GPIO_PIN_MAP(0,31)//AIN7
#define A4              NRF_GPIO_PIN_MAP(0,2) //AIN0
#define A5              NRF_GPIO_PIN_MAP(0,3) //AIN1

#define SCK             NRF_GPIO_PIN_MAP(0,13)
#define MO              NRF_GPIO_PIN_MAP(0,15)
#define MI              NRF_GPIO_PIN_MAP(0,20)

#define D2              NRF_GPIO_PIN_MAP(1,2)
#define RX              NRF_GPIO_PIN_MAP(0,25)
#define TX              NRF_GPIO_PIN_MAP(0,24)
#define SDA_PIN             16 //NRF_GPIO_PIN_MAP(0,16)
#define SCL_PIN             14 //NRF_GPIO_PIN_MAP(0,14)
#define D5              NRF_GPIO_PIN_MAP(0,27)
#define D7              NRF_GPIO_PIN_MAP(1,8)
#define D9              NRF_GPIO_PIN_MAP(0,7)
#define D10             NRF_GPIO_PIN_MAP(0,5) //AIN3
#define D11             NRF_GPIO_PIN_MAP(0,26)
#define D12             NRF_GPIO_PIN_MAP(0,11)
#define D13             NRF_GPIO_PIN_MAP(0,12)

#define RESET           NRF_GPIO_PIN_MAP(0,18) //maybe good for returning into boot

#ifdef __cplusplus
}
#endif

#endif // itsy_H
