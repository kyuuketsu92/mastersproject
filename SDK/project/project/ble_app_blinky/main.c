#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "usb_main.h"


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

#define LEDBUTTON_BUTTON                BSP_BUTTON_0                            /**< Button that will trigger the notification event with the LED Button Service */


typedef struct {float L; float a; float b;}Lab;
typedef struct {float r; float g; float b;}RGB;
#define LUMINOSITY 0.5f
#define CHROME 0.08f
#define STEPS 720

#define LED_BLINK_INTERVAL 800

#define BUTTON_DETECTION_DELAY          APP_TIMER_TICKS(50)                     /**< Delay from a GPIOTE event until a button is reported as pushed (in number of timer ticks). */

#define DEAD_BEEF                       0xDEADBEEF                              /**< Value used as error code on stack dump, can be used to identify stack location on stack unwind. */

APP_TIMER_DEF(my_timer_id);

#define NUMLEDS 24
static WS2812B_LED ledColours[NUMLEDS] = {0};
double step = 6.28 / STEPS;


static uint32_t CSPINS[6] = {D13,D12,D11,D10,D9,D7}; //front bottom left top right  back

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


/**
 * @brief Function for reading data from temperature sensor.
 */
static void read_sensor_data()
{
    m_xfer_done = false;
    volatile ret_code_t err_code;
    uint8_t reg[1] = {MPU6050_DATA_START_ADDR};

    err_code = nrf_drv_twi_tx(&m_twi, MPU6050_ADDR, reg, sizeof(reg), true);
    APP_ERROR_CHECK(err_code);
    while (m_xfer_done == false)
    {
    
    }
    /* Read 14 byte from the specified address - skip 3 bits dedicated for fractional part of temperature. */
    err_code = nrf_drv_twi_rx(&m_twi, MPU6050_ADDR, m_sample, sizeof(m_sample)); 
    m_xfer_done = false;
    APP_ERROR_CHECK(err_code);
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

      ws2812b_shiftout(ledColours,NUMLEDS);

      //send sensor data for visualisation in arduino serial plotter
      memset(status,0,50);

      #define SIDE 0 
      #if 1
      sprintf(status,"%i,%i,%i,%i\r\n",
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[0],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[1],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[2],
        cube_data_struct_values_lowpass.sides[SIDE].sensors_measurement[3]);
      
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
      #endif

      USB_main_write(status,50);

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
    // Initialize.
    log_init();
    leds_init();
    timers_init();
    buttons_init();
    twi_init();
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
    acc_turn_on();
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
