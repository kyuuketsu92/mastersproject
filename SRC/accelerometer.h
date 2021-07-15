//Module for handling the communcation with the accelerometer

//Module prefix: ACCEL_

//singe inclusion definition
#ifndef __ACCEL_H__
#define __ACCEL_H__

#include "stdint.h"
#include "stdbool.h"
#include "boards.h"

///////////////////////////////////////////////////////////
// DEFINITIONS, TYPEDEFS, ENUMS
///////////////////////////////////////////////////////////
typedef struct{
    int16_t accX;
    int16_t accY;
    int16_t accZ;
    int16_t temp;
    int16_t rotX;
    int16_t rotY;
    int16_t rotZ;
}ACCEL_struct_t;

///////////////////////////////////////////////////////////
// EXTERNAL FUNCTIONS
///////////////////////////////////////////////////////////

//initialisation of the SPI protocol that talks with the accelerometer sensor
void ACCEL_init();

//turn on the accelerometer
void ACCEL_start();

//read the latest measurements, only reads into the module struct
void ACCEL_read();

//todo
//setting of accelerometer and gyro sensitivity
//initialisation of the interrupt, turn on-off for the deep sleep wake up later on

//function to copy the module readings back to main. Meant as a read only defence.
void ACCEL_get_data(ACCEL_struct_t * out_struct);



#endif