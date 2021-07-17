//Module for all BLE related code that functions as main BLE even handler or wrapper code

//Module prefix: BLE_

//singe inclusion definition
#ifndef __BLE_MAIN_H__
#define __BLE_MAIN_H__

#include "app_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_button.h"
//ble services
#include "ble_nus.h"
#include "ble_cube_srv.h"
#include "nrf_ble_gatt.h"
#include "app_timer.h"

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

///////////////////////////////////////////////////////////
// GLOBAL variables
///////////////////////////////////////////////////////////
static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

///////////////////////////////////////////////////////////
// EXTERNAL FUNCTIONS
///////////////////////////////////////////////////////////

//meant as a callback to handle both USB and BLE Nordic Uart service commands when/if backdoor access for testing will be implemented
typedef void (*received_command_callback)(char * dataOut, uint16_t bufferSize, uint8_t source); //USB source 0, BLE NUS source 1

//starting point of initialising the BLE services and code from main
void BLE_main_initialise(void);

//separate starter function for starting the advertising process
void BLE_advertising_start(void);

//callback registration function should have used a struct instead, but oh well.
void BLE_register_nus_callback(received_command_callback callback);

//send data over the nordic uart service max 20 chars
void BLE_nus_send_string(char * dataOut, uint16_t bufferSize);

//function wrapper for sending the accelerometer data
void cube_data_write(ble_cube_data_struct_t * data_struct);


#endif