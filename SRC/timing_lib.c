#include "timing_lib.h"

#include "app_timer.h"
#include "string.h"

#define TIMING_LIB_CALLBACK_NUMS 10

//storage of events that needs to be ran immediately
typedef struct{
  timing_callback callback_25[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_50[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_75[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_100[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_150[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_250[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_500[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_1000[TIMING_LIB_CALLBACK_NUMS];
}timing_lib_data_struct_blocking_t;

//here the timing event handler will just set flags and the loop caller will be executing one event at a time
typedef struct{
  uint8_t flag_25[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_50[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_75[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_100[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_150[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_250[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_500[TIMING_LIB_CALLBACK_NUMS];
  uint8_t flag_1000[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_25[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_50[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_75[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_100[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_150[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_250[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_500[TIMING_LIB_CALLBACK_NUMS];
  timing_callback callback_1000[TIMING_LIB_CALLBACK_NUMS];
}timing_lib_data_struct_flags_t;

timing_lib_data_struct_blocking_t data_struct_blocking = {NULL};
timing_lib_data_struct_flags_t data_struct_flags = {NULL};

APP_TIMER_DEF(timer_25ms);
APP_TIMER_DEF(timer_50ms);
APP_TIMER_DEF(timer_75ms);
APP_TIMER_DEF(timer_100ms);
APP_TIMER_DEF(timer_150ms);
APP_TIMER_DEF(timer_250ms);
APP_TIMER_DEF(timer_500ms);
APP_TIMER_DEF(timer_1000ms);

void tim_event25ms(void * p_context);
void tim_event50ms(void * p_context);
void tim_event75ms(void * p_context);
void tim_event100ms(void * p_context);
void tim_event150ms(void * p_context);
void tim_event250ms(void * p_context);
void tim_event500ms(void * p_context);
void tim_event1000ms(void * p_context);


//initialises the timers
void timing_lib_initialise(void)
{
   // Initialize timer module, making it use the scheduler
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);

    app_timer_create(&timer_25ms, APP_TIMER_MODE_REPEATED, tim_event25ms);
    app_timer_create(&timer_50ms, APP_TIMER_MODE_REPEATED, tim_event50ms);
    app_timer_create(&timer_75ms, APP_TIMER_MODE_REPEATED, tim_event75ms);
    app_timer_create(&timer_100ms, APP_TIMER_MODE_REPEATED, tim_event100ms);
    app_timer_create(&timer_150ms, APP_TIMER_MODE_REPEATED, tim_event150ms);
    app_timer_create(&timer_250ms, APP_TIMER_MODE_REPEATED, tim_event250ms);
    app_timer_create(&timer_500ms, APP_TIMER_MODE_REPEATED, tim_event500ms);
    app_timer_create(&timer_1000ms, APP_TIMER_MODE_REPEATED, tim_event1000ms);
}

//sets all the callbacks to NULL (just in case)
void timing_lib_clear_data(void)
{
    memset(data_struct_blocking.callback_25, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_50, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_75, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_100, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_150, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_250, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_500, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_blocking.callback_1000, NULL, TIMING_LIB_CALLBACK_NUMS);

    memset(data_struct_flags.callback_25, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_50, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_75, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_100, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_150, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_250, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_500, 0xFF, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.callback_1000, 0xFF, TIMING_LIB_CALLBACK_NUMS);

    memset(data_struct_flags.flag_25, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_50, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_75, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_100, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_150, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_250, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_500, NULL, TIMING_LIB_CALLBACK_NUMS);
    memset(data_struct_flags.flag_1000, NULL, TIMING_LIB_CALLBACK_NUMS);
}

//starts the timers
void timing_lib_start(void)
{
    app_timer_start(timer_25ms,250,NULL);
    app_timer_start(timer_50ms,500,NULL);
    app_timer_start(timer_75ms,750,NULL);
    app_timer_start(timer_100ms,1000,NULL);
    app_timer_start(timer_150ms,1500,NULL);
    app_timer_start(timer_250ms,2500,NULL);
    app_timer_start(timer_500ms,5000,NULL);
    app_timer_start(timer_1000ms,10000,NULL);
}

//registers an event that is executed on the timing event handler
uint32_t timing_lib_register_blocking_event(timing_lib_evt_e timing, timing_callback callback)
{
    uint8_t i;
    uint32_t retval = 1; //init with error
    switch(timing)
    {
        case TIMING_LIB_25MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_25[i] == NULL)
                {
                    data_struct_blocking.callback_25[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_50MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_50[i] == NULL)
                {
                    data_struct_blocking.callback_50[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_75MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_75[i] == NULL)
                {
                    data_struct_blocking.callback_75[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_100MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_100[i] == NULL)
                {
                    data_struct_blocking.callback_100[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_150MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_150[i] == NULL)
                {
                    data_struct_blocking.callback_150[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_250MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_250[i] == NULL)
                {
                    data_struct_blocking.callback_250[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_500MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_500[i] == NULL)
                {
                    data_struct_blocking.callback_500[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_1000MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_blocking.callback_1000[i] == NULL)
                {
                    data_struct_blocking.callback_1000[i] = callback;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
    }
    return retval;
}

//registers an event that is executed on the timing event handler
uint32_t timing_lib_register_flagged_event(timing_lib_evt_e timing, timing_callback callback)
{
    uint8_t i;
    uint32_t retval = 1; //init with error
    switch(timing)
    {
        case TIMING_LIB_25MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_25[i] == NULL)
                {
                    data_struct_flags.callback_25[i] = callback;
                    data_struct_flags.flag_25[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_50MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_50[i] == NULL)
                {
                    data_struct_flags.callback_50[i] = callback;
                    data_struct_flags.flag_50[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_75MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_75[i] == NULL)
                {
                    data_struct_flags.callback_75[i] = callback;
                    data_struct_flags.flag_75[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_100MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_100[i] == NULL)
                {
                    data_struct_flags.callback_100[i] = callback;
                    data_struct_flags.flag_100[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_150MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_150[i] == NULL)
                {
                    data_struct_flags.callback_150[i] = callback;
                    data_struct_flags.flag_150[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_250MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_250[i] == NULL)
                {
                    data_struct_flags.callback_250[i] = callback;
                    data_struct_flags.flag_250[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_500MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_500[i] == NULL)
                {
                    data_struct_flags.callback_500[i] = callback;
                    data_struct_flags.flag_500[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
        case TIMING_LIB_1000MS:
            for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
            {
                if(data_struct_flags.callback_1000[i] == NULL)
                {
                    data_struct_flags.callback_1000[i] = callback;
                    data_struct_flags.flag_1000[i] = 0;
                    retval = 0;
                }
                if(retval == 0)
                {
                    break;
                }
            }
        break;
    }
    return retval;
}

//call from main to execute flagged events
void timing_lib_handle_flagged_events(void)
{
    uint8_t i;
    uint8_t num_Run = 1; //how many events should run per loop 

    //25ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_25[i] == 1)
            {
                data_struct_flags.callback_25[i](NULL);
                data_struct_flags.flag_25[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }

    //50ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_50[i] == 1)
            {
                data_struct_flags.callback_50[i](NULL);
                data_struct_flags.flag_50[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }

    //75ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_75[i] == 1)
            {
                data_struct_flags.callback_75[i](NULL);
                data_struct_flags.flag_75[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }

    //100ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_100[i] == 1)
            {
                data_struct_flags.callback_100[i](NULL);
                data_struct_flags.flag_100[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }

    //150ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_150[i] == 1)
            {
                data_struct_flags.callback_150[i](NULL);
                data_struct_flags.flag_150[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }

    //250ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_250[i] == 1)
            {
                data_struct_flags.callback_250[i](NULL);
                data_struct_flags.flag_250[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }

    //500ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_500[i] == 1)
            {
                data_struct_flags.callback_500[i](NULL);
                data_struct_flags.flag_500[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    } 
    
    //1000ms
    if(num_Run != 0)
    {
        for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
        {
            if(data_struct_flags.flag_1000[i] == 1)
            {
                data_struct_flags.callback_1000[i](NULL);
                data_struct_flags.flag_1000[i] = 0;
                num_Run--;
            }
            if(num_Run == 0)
            {
                break;
            }
        }
    }
}

void tim_event25ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_25[i] != NULL)
        {
            data_struct_flags.flag_25[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_25[i] != NULL)
        {
            data_struct_blocking.callback_25[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event50ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_50[i] != NULL)
        {
            data_struct_flags.flag_50[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_50[i] != NULL)
        {
            data_struct_blocking.callback_50[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event75ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_75[i] != NULL)
        {
            data_struct_flags.flag_75[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_75[i] != NULL)
        {
            data_struct_blocking.callback_75[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event100ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_75[i] != NULL)
        {
            data_struct_flags.flag_75[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_75[i] != NULL)
        {
            data_struct_blocking.callback_75[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event150ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_150[i] != NULL)
        {
            data_struct_flags.flag_150[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_150[i] != NULL)
        {
            data_struct_blocking.callback_150[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event250ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_250[i] != NULL)
        {
            data_struct_flags.flag_250[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_250[i] != NULL)
        {
            data_struct_blocking.callback_250[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event500ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_500[i] != NULL)
        {
            data_struct_flags.flag_500[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_500[i] != NULL)
        {
            data_struct_blocking.callback_500[i](NULL);
        }
        else
        {
            break;
        }
    }
}
void tim_event1000ms(void * p_context)
{
    uint8_t i;
    //set flags
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_flags.callback_1000[i] != NULL)
        {
            data_struct_flags.flag_1000[i] = 1;
        }
        else
        {
            break;
        }
    }
    //execute callbacks
    for(i = 0; i < TIMING_LIB_CALLBACK_NUMS; i++)
    {
        if(data_struct_blocking.callback_1000[i] != NULL)
        {
            data_struct_blocking.callback_1000[i](NULL);
        }
        else
        {
            break;
        }
    }
}