//this is a module to help with the automatic generation of moving average buffers and calculations

#include "movavg.h"


int32_t movsum(int32_t new_measurement, movavg_struct * buffer)
{
    int32_t retval = 0;
    uint8_t i = 0; 
    
    //replace the oldest element
    buffer->buffer[buffer->counter] = new_measurement;


    //housekeeping
    buffer->counter++; //point to the next element
    if(buffer->counter >= buffer->order)
    {
        buffer->counter = 0;
    }


    //sum of all elements
    for (i = 0; i < buffer->order; i++)
    {
        retval += buffer->buffer[i];
    }

    return retval;
}

float movavg(int32_t new_measurement, movavg_struct * buffer)
{
    return ((float)movsum(new_measurement, buffer))/(float)buffer->order;
}