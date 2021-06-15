#ifndef __MOVAVG_H__
#define __MOVAVG_H__

#include "stdint.h"

typedef struct {
	int32_t * buffer; //pointer to the 
	uint8_t order; 
	uint8_t counter; //pointing to the next value to copy into
}movavg_struct;


#define MOV_STRUCT_GEN(ORDER, NAME, COUNT) int32_t NAME ## _ ## COUNT ## _buffer[ORDER] = {0}; movavg_struct NAME ## _ ## COUNT = { .buffer = NAME ## _ ## COUNT ## _buffer, .order = ORDER, .counter = 0};


int32_t movsum(int32_t new_measurement, movavg_struct * buffer);
float movavg(int32_t new_measurement, movavg_struct * buffer);

#endif