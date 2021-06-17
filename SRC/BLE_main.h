#ifndef __BLE_MAIN_H__
#define __BLE_MAIN_H__

#include "app_error.h"
#include "nrf_sdh.h"
#include "nrf_sdh_ble.h"
#include "app_button.h"
//ble services
#include "ble_nus.h"
#include "nrf_ble_gatt.h"
#include "app_timer.h"

#define APP_BLE_OBSERVER_PRIO           3                                       /**< Application's BLE observer priority. You shouldn't need to modify this value. */
#define APP_BLE_CONN_CFG_TAG            1                                       /**< A tag identifying the SoftDevice BLE configuration. */

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        /**< Handle of the current connection. */

typedef void (*received_command_callback)(char * dataOut, uint16_t bufferSize, uint8_t source); //USB source 0, BLE NUS source 1


void BLE_main_initialise(void);
void BLE_advertising_start(void);
void BLE_register_nus_callback(received_command_callback callback);
void BLE_nus_send_string(char * dataOut, uint16_t bufferSize);



#endif