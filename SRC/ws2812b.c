#include "ws2812b.h"
#include "app_util_platform.h"
#include "nrfx_gpiote.h"

uint32_t pinInternal;

void delay0L() //850ns -> 700 - 1000 ok
{
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
}

void delay0H()//400ns -> 250 - 550 ok
{
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
}

void delay1L()//450ns -> 300 - 600 ok
{
}

void delay1H()//800ns -> 650 - 950 ok
{
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
    __ASM (" NOP\n\t");
}

uint32_t delay50us()
{
    uint32_t retval = 0;
    int i = 0;
    for(i = 0; i < 250; i++)
    {
        retval += 1;
    }
    return retval;
}



void ws2812b_init(uint32_t pin)
{
    nrfx_gpiote_out_config_t pinConfig = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(false);

    pinInternal = pin;
    nrfx_gpiote_out_init(pinInternal,&pinConfig);

    //test
    //nrfx_gpiote_out_clear(pinInternal);
    //delay50us();
    //nrfx_gpiote_out_set(pinInternal);

}

void ws2812b_shiftout(WS2812B_LED * colours, uint32_t ledCount)
{
    uint32_t i = 0;
    delay50us();
    CRITICAL_REGION_ENTER()
    for(i = 0; i < ledCount; i++)
    {
        uint8_t j;
        //green
        uint8_t ledmask = 0x80;
        for(j = 0; j < 8; j++)
        {
            if((colours[i].GREEN & ledmask) != 0)
            {
                NRF_P1->OUTSET = (4UL);
                delay1H();
                ledmask = ledmask / 2;
                NRF_P1->OUTCLR = (4UL);
                delay1L();
            }
            else
            {
                NRF_P1->OUTSET = (4UL);
                delay0H();
                NRF_P1->OUTCLR = (4UL);
                delay0L();
                ledmask = ledmask / 2;
            }
        }

        //red
        ledmask = 0x80;
        for(j = 0; j < 8; j++)
        {
            if((colours[i].RED & ledmask) != 0)
            {
                NRF_P1->OUTSET = (4UL);
                delay1H();
                ledmask = ledmask / 2;
                NRF_P1->OUTCLR = (4UL);
                delay1L();
            }
            else
            {
                NRF_P1->OUTSET = (4UL);
                delay0H();
                NRF_P1->OUTCLR = (4UL);
                delay0L();
                ledmask = ledmask / 2;
            }
        }

        //blue
        ledmask = 0x80;
        for(j = 0; j < 8; j++)
        {
            
            if((colours[i].BLUE & ledmask) != 0)
            {
                NRF_P1->OUTSET = (4UL);
                delay1H();
                ledmask = ledmask / 2;
                NRF_P1->OUTCLR = (4UL);
                delay1L();
            }
            else
            {
                NRF_P1->OUTSET = (4UL);
                delay0H();
                NRF_P1->OUTCLR = (4UL);
                delay0L();
                ledmask = ledmask / 2;
            }
        }
    }
    CRITICAL_REGION_EXIT()
}

