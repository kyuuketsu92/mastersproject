#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "usb_main.h"
#include "accelerometer.h"

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

//ble
#include "BLE_main.h"

#include "movavg.h"

#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */


#define LED_BLINK_INTERVAL 800

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

APP_TIMER_DEF(my_timer_id);

#define NUMLEDS 24
static WS2812B_LED ledColours[NUMLEDS] = {0};

static ACCEL_struct_t accelerometer_data = {0};
static ACCEL_struct_t accelerometer_data_lp = {0};

static uint32_t CSPINS[6] = {D13,D12,D11,D10,D9,D7}; //front bottom left top right  back

static ble_cube_data_struct_t cube_data_struct = {0};

//MATH RELATED

//not the most ideal system to auto generate the full cube's buffers at compilation but I cannot find a loop as a preprocessor directive
#define LOWPASS_ORDER 16
//buttons
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 0)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 1)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 2)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 3)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 4)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 5)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 6)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 7)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 8)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 9)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 10)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 11)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 12)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 13)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 14)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 15)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 16)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 17)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 18)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 19)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 20)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 21)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 22)
MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 23)
//accelerometers
//MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 24)
//MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 25)
//MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 26)
//MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 27)
//MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 28)
//MOV_STRUCT_GEN(LOWPASS_ORDER, mov_buff, 29)
//adding the pointers into a buffer for easier program handling
static movavg_struct * movavg_structs[24] = {&mov_buff_0,&mov_buff_1,&mov_buff_2,&mov_buff_3,&mov_buff_4,&mov_buff_5,&mov_buff_6,&mov_buff_7,&mov_buff_8,&mov_buff_9,&mov_buff_10,
                                             &mov_buff_11,&mov_buff_12,&mov_buff_13,&mov_buff_14,&mov_buff_15,&mov_buff_16,&mov_buff_17,&mov_buff_18,&mov_buff_19,&mov_buff_20,
                                             &mov_buff_21,&mov_buff_22,&mov_buff_23};


//BUFFER FOR BUTTON EVENTS
#define BUTTONPRESSBUFFERDATASIZE 5
static uint8_t buttonPressBuffer [24][BUTTONPRESSBUFFERDATASIZE + 2] = {0}; //0-4 data, 5 counter, 6 status

//static movavg_struct * movavg_structs_acc[6] = {&mov_buff_24,&mov_buff_25,&mov_buff_26,&mov_buff_27,&mov_buff_28,&mov_buff_29};

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

int32_t conv_acc_data(int16_t input, uint16_t acc_sensitivity)
{
    float input_f = (float)input;
    input_f = input_f * acc_sensitivity / 32768;
    return (int32_t)(input_f * 1000);
}

typedef enum{
  BUTTONS_PRESS_START,
  BUTTONS_PRESS_STRONGER,
  BUTTONS_PRESS_RELEASE
}buttonPressEvent_e;

void buttonPressEvent(buttonPressEvent_e eventType, uint8_t buttonIndex, uint8_t buttonState)
{
    static char eventMessage[50] = {0};

    switch(eventType)
    {
        case BUTTONS_PRESS_START:
            sprintf(eventMessage,"START ON %i STATE %i.\r\n", buttonIndex, buttonState);
        break;
        case BUTTONS_PRESS_STRONGER:
            sprintf(eventMessage,"STRONGER ON %i STATE %i.\r\n", buttonIndex, buttonState);
        break;
        case BUTTONS_PRESS_RELEASE:
            sprintf(eventMessage,"RELEASE ON %i STATE %i.\r\n", buttonIndex, buttonState);
        break;
        default:
        break;

    }
    //USB_main_write(eventMessage,50);
}

void buttonEventChecker(void)
{
    int16_t difference;
    int8_t index;
    uint8_t i,k = 0;
    for(k = 0; k < HALLEF_SIDES; k++)
    {
        for(i = 0; i < HALLEF_SENSOR_P_SIDE; i++)
        {
            index = buttonPressBuffer[k*4+i][5];
            difference = cube_data_struct_values_lowpass.sides[k].sensors_measurement[i] - cube_data_struct_config.sides[k].sensors_measurement[i];
            if(difference >= BUTTON_STRONG_PRESS_THRESHOLD)
            {
                buttonPressBuffer[k*4+i][index] = 3;
            }
            else if(difference >= BUTTON_NORMAL_PRESS_THRESHOLD)
            {
                buttonPressBuffer[k*4+i][index] = 2;
            }
            else if(difference >= BUTTON_WEAK_PRESS_THRESHOLD)
            {
                buttonPressBuffer[k*4+i][index] = 1;
            }
            else
            {
                buttonPressBuffer[k*4+i][index] = 0;
            }

            

            //update state

            //event detection

            // 0 x x & s=0 button press start event send strength with event
            if(buttonPressBuffer[k*4+i][6] == 0)
            {
                //if state is 0
                if(buttonPressBuffer[k*4+i][index] != 0)
                {
                    index--;
                    if(index < 0)
                    {
                        index += BUTTONPRESSBUFFERDATASIZE;
                    }
                    //if state is 0
                    if(buttonPressBuffer[k*4+i][index] != 0)
                    {
                        //press has been going on for 2 cycles

                        //send event
                        index = buttonPressBuffer[k*4+i][5];
                        buttonPressBuffer[k*4+i][6] = buttonPressBuffer[k*4+i][index];
                        buttonPressEvent(BUTTONS_PRESS_START, k*4+i, buttonPressBuffer[k*4+i][6]); 
                    }
                }
            }
            else
            {
                //button press ongoing
                if(buttonPressBuffer[k*4+i][index] > buttonPressBuffer[k*4+i][6])
                {
                    //current step bigger than state recognised
                    uint8_t curr_state = buttonPressBuffer[k*4+i][index];
                    index--;
                    if(index < 0)
                    {
                        index += BUTTONPRESSBUFFERDATASIZE;
                    }
                    if(buttonPressBuffer[k*4+i][index] > buttonPressBuffer[k*4+i][6])
                    {
                        //prev_step bigger or equal than state
                        if(curr_state == buttonPressBuffer[k*4+i][index])
                        {
                            //if the button state has been the same for 2 cycles
                            buttonPressBuffer[k*4+i][6] = curr_state;
                            buttonPressEvent(BUTTONS_PRESS_STRONGER, k*4+i, buttonPressBuffer[k*4+i][6]);
                        }
                    }
                }
                else if(buttonPressBuffer[k*4+i][index] == 0)
                {
                    //no press
                    index--;
                    if(index < 0)
                    {
                        index += BUTTONPRESSBUFFERDATASIZE;
                    }
                    if(buttonPressBuffer[k*4+i][index] == 0)
                    {
                        //no press a cycle ago
                        buttonPressBuffer[k*4+i][6] = 0;
                        buttonPressEvent(BUTTONS_PRESS_RELEASE, k*4+i, buttonPressBuffer[k*4+i][6]);
                    }
                }
            }

            //housekeeping
            buttonPressBuffer[k*4+i][5]++;
            if(buttonPressBuffer[k*4+i][5] >= BUTTONPRESSBUFFERDATASIZE)
            {
                buttonPressBuffer[k*4+i][5] = 0;
            }
            
        }
    }
}

void periodicTimerHandler(void * p_context)
{
    static uint32_t counter = 0;
    int16_t i = 0;
    int16_t j = 0;
    int16_t k = 0;

    if(counter % 5 == 0)
    {
        //25msec
        static char status[50] = {0};

      //change button led colours
      uint16_t measurement;
      int16_t difference = 0;
      float luminosity = 0;
      ACCEL_get_data(&accelerometer_data);

     
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
                  switch(buttonPressBuffer[k*4+i][6])
                  {
                      case 3:
                          ledColours[k*4+i].RED = 255;
                      break;
                      case 2:
                          ledColours[k*4+i].RED = 255;
                          ledColours[k*4+i].GREEN = 255;
                      break;
                      case 1:
                          ledColours[k*4+i].GREEN = 255;
                      break;
                      default:
                          ledColours[k*4+i].BLUE = 120;
                      break;
                  }
                  //lateral acceleration colours
                  #if 0                  
                  switch(k)
                  {
                      case 0:
                          luminosity = ((float)-conv_acc_data(accelerometer_data.accY,2))/2000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(255 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(120 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(120 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * -luminosity);
                          }
                      break;
                      case 1:
                          luminosity = ((float)-conv_acc_data(accelerometer_data.accZ,2))/2000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(200 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(255 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(127 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * -luminosity);
                          }
                      break;
                      case 2:
                          luminosity = ((float)-conv_acc_data(accelerometer_data.accX,2))/2000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(255 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(0 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(255 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * -luminosity);
                          }
                      break;
                      case 3:
                          luminosity = ((float)conv_acc_data(accelerometer_data.accZ,2))/2000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(200 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(255 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(127 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * -luminosity);
                          }
                      break;
                      case 4:
                          luminosity = ((float)conv_acc_data(accelerometer_data.accX,2))/2000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(255 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(0 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(255 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * -luminosity);
                          }
                      break;
                      case 5:
                          luminosity = ((float)conv_acc_data(accelerometer_data.accY,2))/2000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(255 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(120 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(120 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * -luminosity);
                          }
                      break;

                  }
                  #endif
                  //angular acceleration colours
                  #if 0
                  switch(k)
                  {
                      case 0:
                      case 5:
                          luminosity = ((float)-conv_acc_data(accelerometer_data.rotY,250))/250000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(255 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(120 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(120 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * -luminosity);
                          }
                      break;
                      case 1:
                      case 3:
                          luminosity = ((float)-conv_acc_data(accelerometer_data.rotZ,250))/250000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(200 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(255 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(127 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * -luminosity);
                          }
                      break;
                      case 2:
                      case 4:
                          luminosity = ((float)-conv_acc_data(accelerometer_data.rotX,250))/250000;
                          if(luminosity > 0)
                          {
                              ledColours[k*4+i].RED = (uint8_t)(255 * luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(0 * luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(0 * luminosity);
                          }
                          else
                          {
                              ledColours[k*4+i].RED = (uint8_t)(0 * -luminosity);
                              ledColours[k*4+i].GREEN = (uint8_t)(255 * -luminosity);
                              ledColours[k*4+i].BLUE = (uint8_t)(255 * -luminosity);
                          }
                      break;
                  }
                  #endif
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
                      average = ((float)averaging_buffer[k*4+i])/(float)(cube_startup_counter+1);
                      cube_data_struct_config.sides[k].sensors_measurement[i] = (uint16_t)average;
                  }
              }
          }
      }



      ws2812b_shiftout(ledColours,NUMLEDS);

      //send sensor data for visualisation in arduino serial plotter
      memset(status,0,50);
      

      //pointless, it just ruins data especially lateral acceleration data
      //accelerometer_data_lp.accX = movavg(accelerometer_data.accX,movavg_structs_acc[0]);
      //accelerometer_data_lp.accY = movavg(accelerometer_data.accY,movavg_structs_acc[1]);
      //accelerometer_data_lp.accZ = movavg(accelerometer_data.accZ,movavg_structs_acc[2]);
      //accelerometer_data_lp.rotX = movavg(accelerometer_data.rotX,movavg_structs_acc[3]);
      //accelerometer_data_lp.rotY = movavg(accelerometer_data.rotY,movavg_structs_acc[4]);
      //accelerometer_data_lp.rotZ = movavg(accelerometer_data.rotZ,movavg_structs_acc[5]);
      #define SIDE 0 
      #if 0
      sprintf(status,"%i,%i,%i,%i\r\n",
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[0],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[1],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[2],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[3]);
      
      #endif
      #if 1
      sprintf(status,"%i,%i\r\n",
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[0],
        cube_data_struct_config.sides[SIDE].sensors_measurement[0]);
      
      #endif

      #if 0
      sprintf(status,"%i,%i,%i\r\n",
        conv_acc_data(accelerometer_data.accX,2), conv_acc_data(accelerometer_data.accY,2), conv_acc_data(accelerometer_data.accZ,2));

      #endif
      #if 0
      sprintf(status,"%i,%i,%i\r\n",
        conv_acc_data(accelerometer_data.rotX,250), conv_acc_data(accelerometer_data.rotY,250), conv_acc_data(accelerometer_data.rotZ,250));

      #endif
      #if 0
      sprintf(status,"%i\r\n",
        accelerometer_data.temp);
      #endif

      USB_main_write(status,50);

      //new reading
      HF_initiate_reading_all();
      ACCEL_read();
    }

    if(counter % 15 == 0)
    {
        // 75 ms
        buttonEventChecker();
    }

    if(counter % 25 == 0)
    {
        //125ms
         
    }

    if(counter % 50 == 0)
    {
        //250 ms
        cube_data_struct.acc_x = conv_acc_data(accelerometer_data.accX,2);
        cube_data_struct.acc_y = conv_acc_data(accelerometer_data.accY,2);
        cube_data_struct.acc_z = conv_acc_data(accelerometer_data.accZ,2);
        cube_data_struct.gyro_x = conv_acc_data(accelerometer_data.rotX,250);
        cube_data_struct.gyro_y = conv_acc_data(accelerometer_data.rotY,250);
        cube_data_struct.gyro_z = conv_acc_data(accelerometer_data.rotZ,250);
        cube_data_write(&cube_data_struct);
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


void backdoor_command_received_callback(char * dataOut, uint16_t bufferSize, uint8_t source)
{
    NRF_LOG_INFO("Data rec");
    BLE_nus_send_string(dataOut,bufferSize);
}



/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    leds_init();
    timers_init();
    buttons_init();
    ACCEL_init();
    USB_main_serial_gen();
    
    apa12led_init(DOT_DATA, DOT_CLK);
    apa12led_clear();
    HF_initialise(MO,SCK,MI,CSPINS,&cube_data_struct_values,adc_read_finished_handler);
    ws2812b_init(D2);
    USB_main_init();
    USB_register_cdc_callback(backdoor_command_received_callback);
    power_management_init();
    BLE_main_initialise();
    BLE_register_nus_callback(backdoor_command_received_callback);
    app_timer_init();
    app_timer_create(&my_timer_id, APP_TIMER_MODE_REPEATED, periodicTimerHandler);
    app_timer_start(my_timer_id,50,NULL);
    ACCEL_start();
    // Start execution.
    USB_main_power_enable();
    BLE_advertising_start();
    


    //mov_buff_0.buffer[0] = 50;
    // Enter main loop.
    for (;;)
    {
        while (USB_main_queue_process())
        {
            /* Nothing to do */
        }
        idle_state_handle();
    }
}


/**
 * @}
 */
