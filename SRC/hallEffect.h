#include "app_error.h"



#ifndef __HALLEFF_H
#define __HALLEFF_H

#define HALLEF_SENSOR_P_SIDE 4
#define HALLEF_SIDES 6

typedef struct{
    uint16_t sensors_measurement[HALLEF_SENSOR_P_SIDE];
}hallef_side_t;

typedef struct{
    hallef_side_t sides[HALLEF_SIDES];
}hallef_cube_t;

typedef void (*hell_eff_measurement_complete_callback)(void);


void HF_initialise(uint32_t dataOutPin, uint32_t clockPin, uint32_t dataInPin, uint32_t * csPins, hallef_cube_t * data_struct, hell_eff_measurement_complete_callback handler);
void HF_initiate_reading_all(void);

#endif // __HALLEFF_H
