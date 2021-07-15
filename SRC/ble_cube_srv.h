//Module for a custom service for the cube project

//Module prefix: ble_cube_

//singe inclusion definition
#ifndef __BLE_CUBE_SRV_H__
#define __BLE_CUBE_SRV_H__

#include <stdint.h>
#include <stdbool.h>
#include "sdk_config.h"
#include "ble.h"
#include "ble_srv_common.h"
#include "nrf_sdh_ble.h"
#include "ble_link_ctx_manager.h"

#ifdef __cplusplus
extern "C" {
#endif


/**@brief   Macro for defining a ble_cube instance.
 *
 * @param     _name            Name of the instance.
 * @hideinitializer
 */
#define BLE_CUBE_DEF(_name)                      \
    BLE_LINK_CTX_MANAGER_DEF(CONCAT_2(_name, _link_ctx_storage),  \
                             (1), \
                             sizeof(ble_cube_client_context_t));   \
    static ble_cube_t _name =                                      \
    {                                                             \
        .p_link_ctx_storage = &CONCAT_2(_name, _link_ctx_storage) \
    };                                                            \
    NRF_SDH_BLE_OBSERVER(_name ## _obs,                           \
                         BLE_CUBE_BLE_OBSERVER_PRIO,               \
                         ble_cube_on_ble_evt,                      \
                         &_name)
///////////////////////////////////////////////////////////
// GLOBAL variables
///////////////////////////////////////////////////////////

/* Forward declaration of the ble_cube_t type. */
typedef struct ble_cube_s ble_cube_t;

///////////////////////////////////////////////////////////
// DEFINITIONS, TYPEDEFS, ENUMS
///////////////////////////////////////////////////////////

#define BLE_UUID_CUBE_SERVICE 0x0001 /**< The UUID of the Nordic UART Service. */

/**@brief   BLE cube event types. */
typedef enum
{
    BLE_CUBE_EVT_RX_GAME_LEVEL,      /**< Data received. */
    BLE_CUBE_EVT_RX_RETRIES_COUNT,   /**< Data received. */
    BLE_CUBE_EVT_RX_ACC_SENS,        /**< Data received. */
    BLE_CUBE_EVT_RX_GYRO_SENS,       /**< Data received. */
    BLE_CUBE_EVT_COMM_STARTED_ACC_X, /**< Notification has been enabled. */
    BLE_CUBE_EVT_COMM_STARTED_ACC_Y, /**< Notification has been enabled. */
    BLE_CUBE_EVT_COMM_STARTED_ACC_Z, /**< Notification has been enabled. */
    BLE_CUBE_EVT_COMM_STARTED_GYRO_X, /**< Notification has been enabled. */
    BLE_CUBE_EVT_COMM_STARTED_GYRO_Y, /**< Notification has been enabled. */
    BLE_CUBE_EVT_COMM_STARTED_GYRO_Z, /**< Notification has been enabled. */
    BLE_CUBE_EVT_COMM_STOPPED_ACC_X, /**< Notification has been disabled. */
    BLE_CUBE_EVT_COMM_STOPPED_ACC_Y, /**< Notification has been disabled. */
    BLE_CUBE_EVT_COMM_STOPPED_ACC_Z, /**< Notification has been disabled. */
    BLE_CUBE_EVT_COMM_STOPPED_GYRO_X, /**< Notification has been disabled. */
    BLE_CUBE_EVT_COMM_STOPPED_GYRO_Y, /**< Notification has been disabled. */
    BLE_CUBE_EVT_COMM_STOPPED_GYRO_Z, /**< Notification has been disabled. */
} ble_cube_evt_type_t;


/**@brief   BLE cube Service @ref BLE_CUBE_EVT_RX_DATA event data.
 *
 * @details This structure is passed to an event when @ref BLE_CUBE_EVT_RX_DATA occurs.
 */
typedef struct
{
    uint8_t const * p_data; /**< A pointer to the buffer with received data. */
    uint16_t        length; /**< Length of received data. */
} ble_cube_evt_rx_data_t;

/* BLE notification contexts */
typedef struct
{
    bool is_notification_enabled_acc_x; 
    bool is_notification_enabled_acc_y;
    bool is_notification_enabled_acc_z;
    bool is_notification_enabled_gyro_x;
    bool is_notification_enabled_gyro_y;
    bool is_notification_enabled_gyro_z;
} ble_cube_client_context_t;

/**@brief   BLE cube Service event structure.
 *
 * @details This structure is passed to an event coming from service.
 */
typedef struct
{
    ble_cube_evt_type_t         type;        /**< Event type. */
    ble_cube_t                * p_cube;       /**< A pointer to the instance. */
    uint16_t                   conn_handle; /**< Connection handle. */
    ble_cube_client_context_t * p_link_ctx;  /**< A pointer to the link context. */
    union
    {
        ble_cube_evt_rx_data_t rx_data; /**< @ref BLE_CUBE_EVT_RX_DATA event data. */
    } params;
} ble_cube_evt_t;

//callback typedef
typedef void (* ble_cube_data_handler_t) (ble_cube_evt_t * p_evt);

//event handler callback
typedef struct
{
    ble_cube_data_handler_t data_handler; /**< Event handler to be called for handling received data. */
} ble_cube_init_t;

//service struct
struct ble_cube_s
{
    uint8_t                         uuid_type;          /**< UUID type for Nordic UART Service Base UUID. */
    uint16_t                        service_handle;     /**< Handle of Nordic UART Service (as provided by the SoftDevice). */
    ble_gatts_char_handles_t        game_level_handles; 
    ble_gatts_char_handles_t        num_retries_handles; 
    ble_gatts_char_handles_t        acc_sens_handles;
    ble_gatts_char_handles_t        gyro_sens_handles;
    ble_gatts_char_handles_t        acc_x_handles;
    ble_gatts_char_handles_t        acc_y_handles;
    ble_gatts_char_handles_t        acc_z_handles;
    ble_gatts_char_handles_t        gyro_x_handles;
    ble_gatts_char_handles_t        gyro_y_handles;
    ble_gatts_char_handles_t        gyro_z_handles;
    blcm_link_ctx_storage_t * const p_link_ctx_storage; /**< Pointer to link context storage with handles of all current connections and its context. */
    ble_cube_data_handler_t         data_handler;       /**< Event handler to be called for handling received data. */
};

typedef struct{
    int32_t acc_x;
    int32_t acc_y;
    int32_t acc_z;
    int32_t gyro_x;
    int32_t gyro_y;
    int32_t gyro_z;
}ble_cube_data_struct_t;

///////////////////////////////////////////////////////////
// EXTERNAL FUNCTION DECLARATIONS
///////////////////////////////////////////////////////////

//Call in BLE services initialisation
uint32_t ble_cube_init(ble_cube_t * p_cube, ble_cube_init_t const * p_cube_init);

//needed for the macro initialisation, event handler
void ble_cube_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

//function call to send the sensor values over BLE
uint32_t ble_cube_data_send(ble_cube_t * p_cube,
                           ble_cube_data_struct_t   * p_data,
                           uint16_t    conn_handle);

#ifdef __cplusplus
}
#endif

#endif