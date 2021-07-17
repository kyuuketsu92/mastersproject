//This module is done to handle timing event handles

//todo: either blocking event handlers or flag setting system to be handled from the main loop

//Module prefix: timing_lib_

//singe inclusion definition
#ifndef __TIMING_LIB_H_
#define __TIMING_LIB_H_

#include "stdint.h"

///////////////////////////////////////////////////////////
// DEFINITIONS, TYPEDEFS, ENUMS
///////////////////////////////////////////////////////////
typedef enum{
  TIMING_LIB_25MS,
  TIMING_LIB_50MS,
  TIMING_LIB_75MS,
  TIMING_LIB_100MS,
  TIMING_LIB_150MS,
  TIMING_LIB_250MS,
  TIMING_LIB_500MS,
  TIMING_LIB_1000MS
}timing_lib_evt_e;

typedef void (*timing_callback)(void * p_context);

///////////////////////////////////////////////////////////
// EXTERNAL FUNCTIONS
///////////////////////////////////////////////////////////

//initialises the timers
void timing_lib_initialise(void);

//sets all the callbacks to NULL (just in case)
void timing_lib_clear_data(void);

//starts the timers
void timing_lib_start(void);

//registers an event that is executed on the timing event handler
//err_code 0 - success, 1 - full array
uint32_t timing_lib_register_blocking_event(timing_lib_evt_e timing, timing_callback callback);

//registers an event that gets its flag set and a separate caller from loop will execute it
//err_code 0 - success, 1 - full array
uint32_t timing_lib_register_flagged_event(timing_lib_evt_e timing, timing_callback callback);

//call from main to execute flagged events
void timing_lib_handle_flagged_events(void);

#endif