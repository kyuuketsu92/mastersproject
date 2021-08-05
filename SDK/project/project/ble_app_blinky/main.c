#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "usb_main.h"
#include "accelerometer.h"
#include "timing_lib.h"

#include "boards.h"

#include "nrf_pwr_mgmt.h"
#include "apa12.h"
#include "ws2812b.h"
#include "math.h"
#include "hallEffect.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "game_logic.h"

//ble
#include "BLE_main.h"

#include "movavg.h"
#include "nrf_drv_rng.h"

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
#define BUTTON_NOISE_THRESHOLD 10
#define BUTTON_WEAK_PRESS_THRESHOLD BUTTON_NOISE_THRESHOLD
#define BUTTON_NORMAL_PRESS_THRESHOLD BUTTON_WEAK_PRESS_THRESHOLD + 10
#define BUTTON_STRONG_PRESS_THRESHOLD BUTTON_NORMAL_PRESS_THRESHOLD + 10
static uint32_t averaging_buffer[24] = {0};


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
            gamelogic_button_state_change_event(buttonIndex, buttonState, 0);
        break;
        case BUTTONS_PRESS_STRONGER:
            sprintf(eventMessage,"STRONGER ON %i STATE %i.\r\n", buttonIndex, buttonState);
            gamelogic_button_state_change_event(buttonIndex, buttonState, buttonState);
        break;
        case BUTTONS_PRESS_RELEASE:
            sprintf(eventMessage,"RELEASE ON %i STATE %i.\r\n", buttonIndex, buttonState);
            gamelogic_button_state_change_event(buttonIndex, 0, buttonState);
        break;
        default:
        break;

    }
    USB_main_write(eventMessage,50);
    
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
                        buttonPressEvent(BUTTONS_PRESS_RELEASE, k*4+i, buttonPressBuffer[k*4+i][6]);                        
                        buttonPressBuffer[k*4+i][6] = 0;
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

void getAccelerometerData(void * p_context) //25ms
{
    ACCEL_get_data(&accelerometer_data);
    ACCEL_read();
}

void writeAccelerometerDataToBLE(void * p_context) //250ms
{
    cube_data_struct.acc_x = conv_acc_data(accelerometer_data.accX,2);
    cube_data_struct.acc_y = conv_acc_data(accelerometer_data.accY,2);
    cube_data_struct.acc_z = conv_acc_data(accelerometer_data.accZ,2);
    cube_data_struct.gyro_x = conv_acc_data(accelerometer_data.rotX,250);
    cube_data_struct.gyro_y = conv_acc_data(accelerometer_data.rotY,250);
    cube_data_struct.gyro_z = conv_acc_data(accelerometer_data.rotZ,250);
    cube_data_write(&cube_data_struct);
}

void checkButtonEvents(void * p_context) // 75 ms
{
    buttonEventChecker();
}

void writeLEDs(void * p_context) // 25 ms (blocking)
{
    ws2812b_shiftout(ledColours,NUMLEDS);
}

void readHallEffectSensors(void * p_context) //25ms
{
    HF_initiate_reading_all();
}

uint8_t randSeedGen(void)
{
    uint8_t value;
    nrf_drv_rng_block_rand(&value,1);

    return value;
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

ble_cube_game_data_struct_t provide_cube_game_data_struct(void)
{
    ble_cube_game_data_struct_t retval = {0};
    gamelogic_data_struct_t game_data_copy = gamelogic_getDataStruct();
    retval.gameLevel = game_data_copy.gameLevel;
    retval.gameLivesMax = game_data_copy.gameMaxLives;
    retval.gameSides = game_data_copy.gameSides;
    retval.buttonDisplayDuration = game_data_copy.gameButtonDisplayDuration;
    retval.gameLiveCurr = game_data_copy.gameCurrentLives;
    return retval;
}

void ble_update_life(uint8_t currlife)
{
    BLE_send_curr_lives(currlife);
}

void gamelogic_update_ble_settings(ble_cube_game_data_struct_t * newStruct)
{
    gamelogic_data_struct_t value = gamelogic_getDataStruct();
    value.gameLevel = newStruct->gameLevel;
    value.gameMaxLives = newStruct->gameLivesMax;
    value.gameCurrentLives = newStruct->gameLiveCurr;
    value.gameButtonDisplayDuration = newStruct->buttonDisplayDuration;
    value.gameSides = newStruct->gameSides;
    gamelogic_newSettings(&value);
}

bool calibrateCallback(bool newCalibrate)
{
    static bool isDone = true; //finished     
    static int16_t cube_startup_counter = -50;
    uint16_t measurement;

    if(newCalibrate)
    {
        cube_startup_counter = -50;
        isDone = false;
    }
    else
    {
        if(cube_startup_counter >= 0)
        {
            uint8_t i,k;
            for(k = 0; k < HALLEF_SIDES; k++)
            {
                for(i = 0; i < HALLEF_SENSOR_P_SIDE; i++)
                {
                    measurement = cube_data_struct_values_lowpass.sides[k].sensors_measurement[i];
                    averaging_buffer[k*HALLEF_SENSOR_P_SIDE+i] += measurement;
                }
            }
        }
        if(cube_startup_counter >= 150)
        {
            float average = 0;
            uint8_t i,k;

            for(k = 0; k < HALLEF_SIDES; k++)
            {
                for(i = 0; i < HALLEF_SENSOR_P_SIDE; i++)
                {
                    average = ((float)averaging_buffer[k*4+i])/(float)(cube_startup_counter-1);
                    cube_data_struct_config.sides[k].sensors_measurement[i] = (uint16_t)average;
                }
            }
            isDone = true;
        }
        cube_startup_counter++;
    }
    return isDone;
}

void update(void * p_context)
{
    gamelogic_update();
}

void oneSecond_gamelogic_update(void * p_context)
{
    gamelogic_second_has_passed_flag_set();
}

/**@brief Function for application main entry.
 */
int main(void)
{
    // Initialize.
    log_init();
    leds_init();
    //or timing might start here
    buttons_init();
    ACCEL_init();
    USB_main_serial_gen();
    nrf_drv_rng_config_t config = NRF_DRV_RNG_DEFAULT_CONFIG;
    nrf_drv_rng_init(&config);
    apa12led_init(DOT_DATA, DOT_CLK);
    apa12led_clear();
    HF_initialise(MO,SCK,MI,CSPINS,&cube_data_struct_values,adc_read_finished_handler);
    ws2812b_init(D2);
    USB_main_init();
    USB_register_cdc_callback(backdoor_command_received_callback);
    power_management_init();
    BLE_main_initialise();
    BLE_register_nus_callback(backdoor_command_received_callback);
    ble_cube_registerGameLogicGrabCallback(provide_cube_game_data_struct);
    ble_cube_registerGameLogicNewSettings(gamelogic_update_ble_settings);
    gamelogic_init_struct_t game_init = {
        .ledArray = ledColours,
        .ledCount = 24,
        .ble_update_callback = ble_update_life,
        .calibrate_callback = calibrateCallback,
        .randseedgen_callback = randSeedGen,
    };
    gamelogic_init(&game_init);
    //app_timer_init(); <= wass called twice???
    timing_lib_initialise();

    timing_lib_register_flagged_event(TIMING_LIB_25MS,&readHallEffectSensors);
    timing_lib_register_flagged_event(TIMING_LIB_25MS,&getAccelerometerData);
    timing_lib_register_blocking_event(TIMING_LIB_25MS,&writeLEDs);
    timing_lib_register_blocking_event(TIMING_LIB_1000MS,&oneSecond_gamelogic_update);
    timing_lib_register_flagged_event(TIMING_LIB_25MS,&update);

    timing_lib_register_flagged_event(TIMING_LIB_250MS,&writeAccelerometerDataToBLE);

    timing_lib_register_flagged_event(TIMING_LIB_75MS,&checkButtonEvents);
    
    timing_lib_start();
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
        timing_lib_handle_flagged_events();
        idle_state_handle();
    }
}


/**
 * @}
 */
