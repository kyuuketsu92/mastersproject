#include "hallEffect.h"
#include "adc_driver.h"
#include "nrfx_gpiote.h"
#include "boards.h"


uint32_t * int_cs_pins = 0;
hallef_cube_t * int_data_struct;

hell_eff_measurement_complete_callback measurements_complete_callback_handler = NULL;

uint8_t counter_side = 0;
uint8_t counter_sensor = 0;
bool isReady = false;

//Sides order CS pins should be given FRONT, TOP, RIGHT, DOWN, LEFT, BACK (where front is the side with the USB)

void adc_callback_handler(uint16_t dataOut)
{
    nrfx_gpiote_out_set(int_cs_pins[counter_side]);
    int_data_struct->sides[counter_side].sensors_measurement[counter_sensor] = dataOut;
    counter_sensor++;
    if(counter_sensor >= HALLEF_SENSOR_P_SIDE)
    {   
        counter_side++;
        counter_sensor = 0;
        if(counter_side >= HALLEF_SIDES)
        {
            if(measurements_complete_callback_handler != NULL)
            {
                measurements_complete_callback_handler();
                isReady = true;
            }
        }
    }    
    if(!isReady)
    {
        nrfx_gpiote_out_clear(int_cs_pins[counter_side]);
        adc_read(counter_sensor);
    }
}

void HF_initiate_reading_all(void)
{
    if(isReady)
    {
        isReady = false;
        counter_side = 0;
        counter_sensor = 0;
        
        nrfx_gpiote_out_clear(int_cs_pins[counter_side]);
        adc_read(counter_sensor);
    }
}
void HF_initialise(uint32_t dataOutPin, uint32_t clockPin, uint32_t dataInPin, uint32_t * csPins, hallef_cube_t * data_struct, hell_eff_measurement_complete_callback handler)
{
    int i = 0;
    nrfx_gpiote_out_config_t pinConfig = NRFX_GPIOTE_CONFIG_OUT_SIMPLE(true);
    isReady = true;
    int_cs_pins = csPins;
    adc_init(dataOutPin, clockPin, dataInPin, adc_callback_handler);
    int_data_struct = data_struct;
    measurements_complete_callback_handler = handler;
    for(i = 0; i < HALLEF_SIDES; i++)
    {
        
        nrfx_gpiote_out_init(int_cs_pins[i],&pinConfig);
    }
}