/**
 * Copyright (c) 2015 - 2020, Nordic Semiconductor ASA
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
/**
 * @brief Blinky Sample Application main file.
 *
 * This file contains the source code for a sample server application using the LED Button service.
 */

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"



#include "boards.h"
#include "app_timer.h"



#include "nrf_pwr_mgmt.h"
#include "apa12.h"
#include "ws2812b.h"
#include "math.h"
#include "hallEffect.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

//i2c
#include "nrf_drv_twi.h"

//usb
#include "nrf_drv_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

//ble
#include "BLE_main.h"

#include "movavg.h"

/* TWI instance ID. */
#define TWI_INSTANCE_ID     0

#define MPU6050_ADDR 0x68
#define MPU6050_PWR_ADDR 0x6B
#define MPU6050_DATA_START_ADDR 0x3B
#define MPU6050_READBYTES_COUNT 14 
/* Indicates if operation on TWI has ended. */
static volatile bool m_xfer_done = false;

/* TWI instance. */
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

/* Buffer for samples read from temperature sensor. */
static uint8_t m_sample[14] = {0};
static int16_t accX;
static int16_t accY;
static int16_t accZ;
static int16_t temp;
static int16_t rotX;
static int16_t rotY;
static int16_t rotZ;

//usb definitions
// USB DEFINES START
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

static char m_cdc_data_array[BLE_NUS_MAX_DATA_LEN];

/** @brief CDC_ACM class instance */
APP_USBD_CDC_ACM_GLOBAL_DEF(m_app_cdc_acm,
                            cdc_acm_user_ev_handler,
                            CDC_ACM_COMM_INTERFACE,
                            CDC_ACM_DATA_INTERFACE,
                            CDC_ACM_COMM_EPIN,
                            CDC_ACM_DATA_EPIN,
                            CDC_ACM_DATA_EPOUT,
                            APP_USBD_CDC_COMM_PROTOCOL_AT_V250);

// USB DEFINES END
static bool m_usb_connected = false;

typedef struct {float L; float a; float b;}Lab;
typedef struct {float r; float g; float b;}RGB;
#define LUMINOSITY 0.5f
#define CHROME 0.08f
#define STEPS 720

#define LED_BLINK_INTERVAL 800

APP_TIMER_DEF(m_blink_ble);
APP_TIMER_DEF(m_blink_cdc);


#define ADVERTISING_LED                 BSP_BOARD_LED_2                        /**< Is on when device is advertising. */
#define CONNECTED_LED                   BSP_BOARD_LED_1                        /**< Is on when device has connected. */
#define LEDBUTTON_LED                   BSP_BOARD_LED_0                         /**< LED to be toggled with the help of the LED Button Service. */
#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */




#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */



APP_TIMER_DEF(my_timer_id);





#define NUMLEDS 24
static WS2812B_LED ledColours[NUMLEDS] = {0};
double step = 6.28 / STEPS;


static uint32_t CSPINS[6] = {D13,D12,D11,D10,D9,D7}; //front bottom left top right  back

#define ENDLINE_STRING "\r\n"





//MATH RELATED

//not the most ideal system to auto generate the full cube's buffers at compilation but I cannot find a loop as a preprocessor directive
#define LOWPASS_OERDER 16
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 0)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 1)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 2)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 3)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 4)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 5)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 6)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 7)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 8)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 9)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 10)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 11)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 12)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 13)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 14)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 15)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 16)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 17)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 18)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 19)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 20)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 21)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 22)
MOV_STRUCT_GEN(LOWPASS_OERDER, mov_buff, 23)
//adding the pointers into a buffer for easier program handling
static movavg_struct * movavg_structs[24] = {&mov_buff_0,&mov_buff_1,&mov_buff_2,&mov_buff_3,&mov_buff_4,&mov_buff_5,&mov_buff_6,&mov_buff_7,&mov_buff_8,&mov_buff_9,&mov_buff_10,
                                    &mov_buff_11,&mov_buff_12,&mov_buff_13,&mov_buff_14,&mov_buff_15,&mov_buff_16,&mov_buff_17,&mov_buff_18,&mov_buff_19,&mov_buff_20,&mov_buff_21,&mov_buff_22,&mov_buff_23};

static hallef_cube_t cube_data_struct_values = {0}; //contains the readings after an iteration, will be used to trigger the moving averaging
static hallef_cube_t cube_data_struct_values_lowpass = {0}; //contains the low passed values
static hallef_cube_t cube_data_struct_config = {0}; //this is where I'll store the values of the unpressed buttons which for now will be averaged at the start. 
#define BUTTON_NOISE_THRESHOLD 5
#define BUTTON_WEAK_PRESS_THRESHOLD BUTTON_NOISE_THRESHOLD
#define BUTTON_NORMAL_PRESS_THRESHOLD BUTTON_WEAK_PRESS_THRESHOLD + 10
#define BUTTON_STRONG_PRESS_THRESHOLD BUTTON_NORMAL_PRESS_THRESHOLD + 10
typedef enum{
    CUBE_STATE_STARTUP,
    CUBE_STATE_RUNNING
}cube_state_e;
static volatile cube_state_e cube_state = CUBE_STATE_STARTUP;
static uint32_t averaging_buffer[24] = {0}; 
static int16_t cube_startup_counter = -50;


/**@brief Function for assert macro callback.
 *
 * @details This function will be called in case of an assert in the SoftDevice.
 *
 * @warning This handler is an example only and does not fit a final product. You need to analyze
 *          how your product is supposed to react in case of Assert.
 * @warning On assert from the SoftDevice, the system can only recover on reset.
 *
 * @param[in] line_num    Line number of the failing ASSERT call.
 * @param[in] p_file_name File name of the failing ASSERT call.
 */
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


/**@brief Function for the LEDs initialization.
 *
 * @details Initializes all LEDs used by the application.
 */
static void leds_init(void)
{
    bsp_board_init(BSP_INIT_LEDS);
}


/**@brief Function for the Timer initialization.
 *
 * @details Initializes the timer module.
 */
static void timers_init(void)
{
    // Initialize timer module, making it use the scheduler
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the button handler module.
 */
static void buttons_init(void)
{
    ret_code_t err_code;

    //The array must be static because a pointer to it will be saved in the button handler module.
    static app_button_cfg_t buttons[] =
    {
        {LEDBUTTON_BUTTON, false, BUTTON_PULL, NULL}
    };

    err_code = app_button_init(buttons, ARRAY_SIZE(buttons),
                               BUTTON_DETECTION_DELAY);
    APP_ERROR_CHECK(err_code);
}


static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}
#define SMOOTH 1

#if SMOOTH
RGB oklab_to_linear_srgb(Lab c, float strength) 
{
    volatile RGB retval = {0};
    float l_ = c.L + 0.3963377774f * c.a + 0.2158037573f * c.b;
    float m_ = c.L - 0.1055613458f * c.a - 0.0638541728f * c.b;
    float s_ = c.L - 0.0894841775f * c.a - 1.2914855480f * c.b;

    float l = l_*l_*l_;
    float m = m_*m_*m_;
    float s = s_*s_*s_;
    
    retval.r = +4.0767416621f * l - 3.3077115913f * m + 0.2309699292f * s;
    retval.r *= strength;
    if(retval.r > 255) 
        retval.r = 255;
    if(retval.r < 0)   
        retval.r = 0;
    retval.g = -1.2684380046f * l + 2.6097574011f * m - 0.3413193965f * s;
    retval.g *= strength;
    if(retval.g > 255) 
        retval.g = 255;
    if(retval.g < 0)   
        retval.g = 0;
    retval.b = -0.0041960863f * l - 0.7034186147f * m + 1.7076147010f * s;
    retval.b *= strength;
    if(retval.b > 255) 
        retval.b = 255;
    if(retval.b < 0)   
        retval.b = 0;


    return retval;
}
#endif

/**
 * @brief Function for reading data from temperature sensor.
 */
static void read_sensor_data()
{
    m_xfer_done = false;
    volatile ret_code_t err_code;
    uint8_t reg[1] = {MPU6050_DATA_START_ADDR};

    //nrf_drv_twi_tx(&m_twi, MPU6050_ADDR, reg, sizeof(reg), false);

    err_code = nrf_drv_twi_tx(&m_twi, MPU6050_ADDR, reg, sizeof(reg), true);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false)
    {
        
    }
    /* Read 14 byte from the specified address - skip 3 bits dedicated for fractional part of temperature. */
    err_code = nrf_drv_twi_rx(&m_twi, MPU6050_ADDR, m_sample, sizeof(m_sample)); 
    m_xfer_done = false;
    //APP_ERROR_CHECK(err_code);
}

void periodicTimerHandler(void * p_context)
{
    static uint32_t counter = 0;
    int i = 0;
    int k = 0;
    static bool toggle = true;


    static uint32_t ledBlueStep = 0; //360 steps
    static uint32_t ledGreenStep = (uint32_t)(STEPS/3);
    static uint32_t ledRedStep = (uint32_t)(2*(STEPS/3));
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t j;
    Lab input = {0};
    RGB output = {0};


    input.a = CHROME*cos(step*ledBlueStep);
    input.b = CHROME*sin(step*ledBlueStep);
    input.L = LUMINOSITY;

    ledBlueStep++;
    ledGreenStep++;
    ledRedStep++;
    if(ledBlueStep >= STEPS)
    {
       ledBlueStep = 0;
    }
    if(ledRedStep >= STEPS)
    {
       ledRedStep = 0;
    }
    if(ledGreenStep >= STEPS)
    {
       ledGreenStep = 0;
    }

    if(counter % 5 == 0)
    {
        //25msec
        static char status[50] = {0};


      

      //board led visualisation
      //output = oklab_to_linear_srgb(input,255);

      //internal LED that was used for debugging but no point when the cube is assembled fully
      //apa12led_setBlue((uint8_t)output.b);
      //apa12led_setGreen((uint8_t)output.g);
      //apa12led_setRed((uint8_t)output.r);
      //apa12led_transfer();

      #define DECOR 0
      #if DECOR
      for(i = 0; i < NUMLEDS; i++)
      {
        ledColours[i].BLUE = output.b;
        ledColours[i].RED = output.r;
        ledColours[i].GREEN = output.g;
      }
      #else
      //first we need to wait for roughly 200 measurements - 50 for startup and buffer filling and 150 for averaging
      //averaging_buffer


      //change button led colours
      uint16_t measurement;
      int16_t difference = 0;

     
      for(k = 0; k < HALLEF_SIDES; k++)
      {
        for(i = 0; i < HALLEF_SENSOR_P_SIDE; i++)
        {
          ledColours[k*4+i].RED = 0;
          ledColours[k*4+i].GREEN = 0;
          ledColours[k*4+i].BLUE = 0;
          measurement = cube_data_struct_values_lowpass.sides[k].sensors_measurement[i];
          switch(cube_state)
          {
              case CUBE_STATE_STARTUP:
                  ledColours[k*4+i].RED = 255;
                  if(cube_startup_counter >= 0)
                  {
                      averaging_buffer[k*4+i] += measurement;
                  }
              break;

              case CUBE_STATE_RUNNING:
                  difference = cube_data_struct_values_lowpass.sides[k].sensors_measurement[i] - cube_data_struct_config.sides[k].sensors_measurement[i];
                  if(difference >= BUTTON_STRONG_PRESS_THRESHOLD)
                  {
                      ledColours[k*4+i].RED = 255;
                  }
                  else if(difference >= BUTTON_NORMAL_PRESS_THRESHOLD)
                  {
                      ledColours[k*4+i].RED = 255;
                      ledColours[k*4+i].GREEN = 255;
                  }
                  else if(difference >= BUTTON_WEAK_PRESS_THRESHOLD)
                  {
                      ledColours[k*4+i].GREEN = 255;
                  }
                  else
                  {
                      ledColours[k*4+i].BLUE = 100;
                  }
              break;

              default:
                //should not happen
              break;
          }
        }
      }
      if(cube_state == CUBE_STATE_STARTUP)
      {
          cube_startup_counter++;
          if(cube_startup_counter >= 150)
          {
              float average = 0;
              uint8_t i,k;

              cube_state = CUBE_STATE_RUNNING;
              for(k = 0; k < HALLEF_SIDES; k++)
              {
                  for(i = 0; i < HALLEF_SENSOR_P_SIDE; i++)
                  {
                      average = ((float)averaging_buffer[k*4+i])/(float)(cube_startup_counter-1);
                      cube_data_struct_config.sides[k].sensors_measurement[i] = (uint16_t)average;
                  }
              }
          }
      }

      #endif
      ws2812b_shiftout(ledColours,NUMLEDS);

      //send sensor data
      memset(status,0,50);

      #define SIDE 0 
      #if 1
      sprintf(status,"%i,%i,%i,%i\r\n",
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[0],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[1],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[2],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[3]);
      ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,
                                              status,
                                              50);
      #endif

      #if 0
      sprintf(status,"%i,%i,%i\r\n",
        accX, accY, accZ);
      ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,
                                              status,
                                              50);

      #endif
      #if 0
      sprintf(status,"%i,%i,%i\r\n",
        rotX, rotY, rotZ);
      ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,
                                              status,
                                              50);

      #endif
      #if 0
      sprintf(status,"%i\r\n",
        temp);
      ret_code_t ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,
                                              status,
                                              50);

      #endif
      if(ret != NRF_SUCCESS)
      {
          NRF_LOG_INFO("CDC ACM unavailable");
      }

      //new reading
      HF_initiate_reading_all();
      read_sensor_data();
    }

    if(counter % 200 == 0)
    {
        //1sec
        
    }
    

    //housekeeping
    counter++;
    if(counter >= 1000 )
    {
        counter = 0;
    }
}

void adc_read_finished_handler(void)
{
    //I guess this is where the averaging should be plugged into.
    uint8_t button = 0; //0-3
    uint8_t side = 0; //0-5

    uint8_t index = 0;
    uint16_t measurement = 0;

    for(side = 0; side < 6; side++)
    {
      for(button = 0; button < 4; button++)
      {
          index = (side * 4) + button; //0-23
          measurement = cube_data_struct_values.sides[side].sensors_measurement[button];
          measurement = (uint16_t)movavg(measurement,movavg_structs[index]);
          cube_data_struct_values_lowpass.sides[side].sensors_measurement[button] = measurement;
      }
    }
}

/** @brief User event handler @ref app_usbd_cdc_acm_user_ev_handler_t */
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event)
{
    app_usbd_cdc_acm_t const * p_cdc_acm = app_usbd_cdc_acm_class_get(p_inst);

    switch (event)
    {
        case APP_USBD_CDC_ACM_USER_EVT_PORT_OPEN:
        {
            /*Set up the first transfer*/
            ret_code_t ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                                   m_cdc_data_array,
                                                   1);
            UNUSED_VARIABLE(ret);
            ret = app_timer_stop(m_blink_cdc);
            APP_ERROR_CHECK(ret);
            //bsp_board_led_on(LED_CDC_ACM_CONN);
            NRF_LOG_INFO("CDC ACM port opened");
            break;
        }

        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            NRF_LOG_INFO("CDC ACM port closed");
            if (m_usb_connected)
            {
                //ret_code_t ret = app_timer_start(m_blink_cdc,
                //                                 APP_TIMER_TICKS(LED_BLINK_INTERVAL),
                //                                (void *) LED_CDC_ACM_CONN);
                //APP_ERROR_CHECK(ret);
            }
            break;

        case APP_USBD_CDC_ACM_USER_EVT_TX_DONE:
            break;

        case APP_USBD_CDC_ACM_USER_EVT_RX_DONE:
        {
            ret_code_t ret;
            static uint8_t index = 0;
            index++;

            do
            {
                if ((m_cdc_data_array[index - 1] == '\n') ||
                    (m_cdc_data_array[index - 1] == '\r') ||
                    (index >= (20)))
                {
                    if (index > 1)
                    {
                        //bsp_board_led_invert(LED_CDC_ACM_RX);
                        NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                        NRF_LOG_HEXDUMP_DEBUG(m_cdc_data_array, index);

                        do
                        {
                            uint16_t length = (uint16_t)index;
                            if (length + sizeof(ENDLINE_STRING) < BLE_NUS_MAX_DATA_LEN)
                            {
                                memcpy(m_cdc_data_array + length, ENDLINE_STRING, sizeof(ENDLINE_STRING));
                                length += sizeof(ENDLINE_STRING);
                            }

                            //ret = ble_nus_data_send(&m_nus,(uint8_t *) m_cdc_data_array,&length,m_conn_handle);

                            if (ret == NRF_ERROR_NOT_FOUND)
                            {
                                NRF_LOG_INFO("BLE NUS unavailable, data received: %s", m_cdc_data_array);
                                break;
                            }

                            if (ret == NRF_ERROR_RESOURCES)
                            {
                                NRF_LOG_ERROR("BLE NUS Too many notifications queued.");
                                break;
                            }

                            if ((ret != NRF_ERROR_INVALID_STATE) && (ret != NRF_ERROR_BUSY))
                            {
                                APP_ERROR_CHECK(ret);
                            }
                        }
                        while (ret == NRF_ERROR_BUSY);
                    }

                    index = 0;
                }

                /*Get amount of data transferred*/
                size_t size = app_usbd_cdc_acm_rx_size(p_cdc_acm);
                NRF_LOG_DEBUG("RX: size: %lu char: %c", size, m_cdc_data_array[index - 1]);

                /* Fetch data until internal buffer is empty */
                ret = app_usbd_cdc_acm_read(&m_app_cdc_acm,
                                            &m_cdc_data_array[index],
                                            1);
                if (ret == NRF_SUCCESS)
                {
                    index++;
                }
            }
            while (ret == NRF_SUCCESS);

            break;
        }
        default:
            break;
    }
}

static void usbd_user_ev_handler(app_usbd_event_type_t event)
{
    switch (event)
    {
        case APP_USBD_EVT_DRV_SUSPEND:
            break;

        case APP_USBD_EVT_DRV_RESUME:
            break;

        case APP_USBD_EVT_STARTED:
            break;

        case APP_USBD_EVT_STOPPED:
            app_usbd_disable();
            break;

        case APP_USBD_EVT_POWER_DETECTED:
            NRF_LOG_INFO("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;

        case APP_USBD_EVT_POWER_REMOVED:
        {
            NRF_LOG_INFO("USB power removed");
            ret_code_t err_code = app_timer_stop(m_blink_cdc);
            APP_ERROR_CHECK(err_code);
            //bsp_board_led_off(LED_CDC_ACM_CONN);
            m_usb_connected = false;
            app_usbd_stop();
        }
            break;

        case APP_USBD_EVT_POWER_READY:
        {
            NRF_LOG_INFO("USB ready");
            //ret_code_t err_code = app_timer_start(m_blink_cdc,
            //                                      APP_TIMER_TICKS(LED_BLINK_INTERVAL),
            //                                      (void *) LED_CDC_ACM_CONN);
            //APP_ERROR_CHECK(err_code);
            m_usb_connected = true;
            app_usbd_start();
        }
            break;

        default:
            break;
    }
}

/**
 * @brief TWI events handler.
 */
void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
    switch (p_event->type)
    {
        case NRF_DRV_TWI_EVT_DONE:
            if (p_event->xfer_desc.type == NRF_DRV_TWI_XFER_RX)
            {
                accX = m_sample[0] << 8 | m_sample[1];
                accY = m_sample[2] << 8 | m_sample[3];
                accZ = m_sample[4] << 8 | m_sample[5];
                temp = m_sample[6] << 8 | m_sample[7];
                rotX = m_sample[8] << 8 | m_sample[9];
                rotY = m_sample[10] << 8 | m_sample[11];
                rotZ = m_sample[12] << 8 | m_sample[13];
            }
            m_xfer_done = true;
            break;
        default:
            break;
    }
}

//turn the accelerometer on
void acc_turn_on()
{
    ret_code_t err_code;
    /* Writing to MPU6050 PWR_MGMT_1  "0" to wake the ACCmeter up. */
    uint8_t reg[2] = {MPU6050_PWR_ADDR, 0};
    err_code = nrf_drv_twi_tx(&m_twi, MPU6050_ADDR, reg, sizeof(reg), false);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false);

}

/**
 * @brief I2C initialization.
 */
void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_mpu6050_config = {
       .scl                = SCL_PIN,
       .sda                = SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_mpu6050_config, twi_handler, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void backdoor_command_received_callback(char * dataOut, uint16_t bufferSize, uint8_t source)
{
    NRF_LOG_INFO("Data rec");
    BLE_nus_send_string(dataOut,bufferSize);
}




/**@brief Function for application main entry.
 */
int main(void)
{
    ret_code_t ret;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };


    // Initialize.
    log_init();
    leds_init();
    timers_init();
    buttons_init();
    twi_init();
    app_usbd_serial_num_generate();

    
    apa12led_init(DOT_DATA, DOT_CLK);
    apa12led_clear();
    HF_initialise(MO,SCK,MI,CSPINS,&cube_data_struct_values,adc_read_finished_handler);
    ws2812b_init(D2);

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);

    power_management_init();
    BLE_main_initialise();
    BLE_register_nus_callback(backdoor_command_received_callback);
    app_timer_init();
    app_timer_create(&my_timer_id, APP_TIMER_MODE_REPEATED, periodicTimerHandler);
    app_timer_start(my_timer_id,50,NULL);
    acc_turn_on();
    // Start execution.
    //NRF_LOG_INFO("Blinky example started.");
    BLE_advertising_start();

    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);


    //mov_buff_0.buffer[0] = 50;
    // Enter main loop.
    for (;;)
    {
        while (app_usbd_event_queue_process())
        {
            /* Nothing to do */
        }
        idle_state_handle();
    }
}


/**
 * @}
 */
