#include "sdk_common.h"
#include "ble.h"
#include "ble_cube_srv.h"
#include "ble_srv_common.h"

#define BLE_UUID_CUBE_GAME_LEVEL_CHARACTERISTIC 0x0002 
#define BLE_UUID_CUBE_NUM_RETRIES_CHARACTERISTIC 0x0003
#define BLE_UUID_CUBE_CURR_LIVES_CHARACTERISTIC 0x0004
#define BLE_UUID_CUBE_CUBE_SIDES_CHARACTERISTIC 0x0005
#define BLE_UUID_CUBE_BUTTON_DURATION_CHARACTERISTIC 0x0006
#define BLE_UUID_CUBE_ACC_X_CHARACTERISTIC 0x0007
#define BLE_UUID_CUBE_ACC_Y_CHARACTERISTIC 0x0008
#define BLE_UUID_CUBE_ACC_Z_CHARACTERISTIC 0x0009
#define BLE_UUID_CUBE_GYRO_X_CHARACTERISTIC 0x000A
#define BLE_UUID_CUBE_GYRO_Y_CHARACTERISTIC 0x000B
#define BLE_UUID_CUBE_GYRO_Z_CHARACTERISTIC 0x000C

#define CUBE_BASE_UUID                  {{0xB3, 0xF4, 0x45, 0x68, 0xC1, 0x54, 0x41, 0xD7, 0xBC, 0x98, 0x90, 0xB3, 0x00, 0x00, 0xE1, 0x7B}} /**< Used vendor specific UUID. */

ble_cube_grab_game_logic_callback gameGrabCallback = NULL;
ble_cube_receive_new_settings_callback gameNewSettingsCallback = NULL;

/**@brief Function for handling the @ref BLE_GAP_EVT_CONNECTED event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_connect(ble_cube_t * p_cube, ble_evt_t const * p_ble_evt)
{
    ret_code_t                 err_code;
    ble_cube_evt_t              evt;
    ble_gatts_value_t          gatts_val;
    uint8_t                    cccd_value[2];
    ble_cube_client_context_t * p_client = NULL;
    uint8_t                    attribute_value;
    ble_cube_game_data_struct_t dataStruct = {0};
    ble_cube_game_data_struct_t dataStructRec = {0};

    err_code = blcm_link_ctx_get(p_cube->p_link_ctx_storage,
                                 p_ble_evt->evt.gap_evt.conn_handle,
                                 (void *) &p_client);
    if (err_code != NRF_SUCCESS)
    {
        int i = 0;
        i++; //used as a debug breakpoint
    }

    /* Check the hosts CCCD value to inform of readiness to send data using the RX characteristic */
    memset(&gatts_val, 0, sizeof(ble_gatts_value_t));
    gatts_val.p_value = cccd_value;
    gatts_val.len     = sizeof(cccd_value);
    gatts_val.offset  = 0;

    //ACC X
    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_cube->acc_x_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_cube->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled_acc_x = true;
        }

        memset(&evt, 0, sizeof(ble_cube_evt_t));
        evt.type        = BLE_CUBE_EVT_COMM_STARTED_ACC_X;
        evt.p_cube       = p_cube;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_cube->data_handler(&evt);
    }

    //ACC Y
    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_cube->acc_y_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_cube->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled_acc_y = true;
        }

        memset(&evt, 0, sizeof(ble_cube_evt_t));
        evt.type        = BLE_CUBE_EVT_COMM_STARTED_ACC_Y;
        evt.p_cube       = p_cube;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_cube->data_handler(&evt);
    }
    //ACC Z
    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_cube->acc_z_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_cube->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled_acc_z = true;
        }

        memset(&evt, 0, sizeof(ble_cube_evt_t));
        evt.type        = BLE_CUBE_EVT_COMM_STARTED_ACC_Z;
        evt.p_cube       = p_cube;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_cube->data_handler(&evt);
    }
    //GYRO X
    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_cube->gyro_x_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_cube->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled_gyro_x = true;
        }

        memset(&evt, 0, sizeof(ble_cube_evt_t));
        evt.type        = BLE_CUBE_EVT_COMM_STARTED_GYRO_X;
        evt.p_cube       = p_cube;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_cube->data_handler(&evt);
    }
    //GYRO Y
    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_cube->gyro_y_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_cube->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled_gyro_y = true;
        }

        memset(&evt, 0, sizeof(ble_cube_evt_t));
        evt.type        = BLE_CUBE_EVT_COMM_STARTED_GYRO_Y;
        evt.p_cube       = p_cube;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_cube->data_handler(&evt);
    }
    //GYRO Z
    err_code = sd_ble_gatts_value_get(p_ble_evt->evt.gap_evt.conn_handle,
                                      p_cube->gyro_z_handles.cccd_handle,
                                      &gatts_val);

    if ((err_code == NRF_SUCCESS)     &&
        (p_cube->data_handler != NULL) &&
        ble_srv_is_notification_enabled(gatts_val.p_value))
    {
        if (p_client != NULL)
        {
            p_client->is_notification_enabled_gyro_z = true;
        }

        memset(&evt, 0, sizeof(ble_cube_evt_t));
        evt.type        = BLE_CUBE_EVT_COMM_STARTED_GYRO_Z;
        evt.p_cube       = p_cube;
        evt.conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
        evt.p_link_ctx  = p_client;

        p_cube->data_handler(&evt);
    }

    //let's set the default values for the game settings for now

    //need a way to get the game logic...
    dataStruct.buttonDisplayDuration = 1;
    dataStruct.gameLevel = 0;
    dataStruct.gameLivesMax = 3;
    dataStruct.gameSides = 6;
    dataStruct.gameLiveCurr = 3;
    if(gameGrabCallback != NULL)
    {
        dataStructRec = gameGrabCallback();
        dataStruct.buttonDisplayDuration = dataStructRec.buttonDisplayDuration;
        dataStruct.gameLevel = dataStructRec.gameLevel;
        dataStruct.gameLivesMax = dataStructRec.gameLivesMax;
        dataStruct.gameSides = dataStructRec.gameSides;
        dataStruct.gameLiveCurr = dataStructRec.gameLiveCurr;
    }

    gatts_val.len = 1;
    gatts_val.offset = 0;
    gatts_val.p_value = &attribute_value;

    attribute_value = dataStruct.gameLevel;
    sd_ble_gatts_value_set(p_ble_evt->evt.gap_evt.conn_handle, p_cube->game_level_handles.value_handle,&gatts_val);

    attribute_value = dataStruct.gameLivesMax;
    sd_ble_gatts_value_set(p_ble_evt->evt.gap_evt.conn_handle, p_cube->num_retries_handles.value_handle,&gatts_val);

    attribute_value = dataStruct.gameLiveCurr;
    sd_ble_gatts_value_set(p_ble_evt->evt.gap_evt.conn_handle, p_cube->curr_lives_handles.value_handle,&gatts_val);

    attribute_value = dataStruct.gameSides;
    sd_ble_gatts_value_set(p_ble_evt->evt.gap_evt.conn_handle, p_cube->game_sides_handles.value_handle,&gatts_val);

    attribute_value = dataStruct.buttonDisplayDuration;
    sd_ble_gatts_value_set(p_ble_evt->evt.gap_evt.conn_handle, p_cube->button_display_duration_handles.value_handle,&gatts_val);
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_cube_t * p_cube, ble_evt_t const * p_ble_evt)
{
    ret_code_t                    err_code;
    ble_cube_evt_t                 evt;
    ble_cube_client_context_t    * p_client;
    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;
    uint8_t needToSendNewSettings = 0;
    ble_cube_game_data_struct_t dataStructRec = {0};

    err_code = blcm_link_ctx_get(p_cube->p_link_ctx_storage,
                                 p_ble_evt->evt.gatts_evt.conn_handle,
                                 (void *) &p_client);

    memset(&evt, 0, sizeof(ble_cube_evt_t));
    evt.p_cube       = p_cube;
    evt.conn_handle = p_ble_evt->evt.gatts_evt.conn_handle;
    evt.p_link_ctx  = p_client;

    if(gameGrabCallback != NULL)
    {
        dataStructRec = gameGrabCallback();
    }

    //CCCD
    //ACC X
    if ((p_evt_write->handle == p_cube->acc_x_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled_acc_x = true;
                evt.type                          = BLE_CUBE_EVT_COMM_STARTED_ACC_X;
            }
            else
            {
                p_client->is_notification_enabled_acc_x = false;
                evt.type                          = BLE_CUBE_EVT_COMM_STOPPED_ACC_X;
            }

            if (p_cube->data_handler != NULL)
            {
                p_cube->data_handler(&evt);
            }

        }
    }
    //ACC Y
    else if ((p_evt_write->handle == p_cube->acc_y_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled_acc_y = true;
                evt.type                          = BLE_CUBE_EVT_COMM_STARTED_ACC_Y;
            }
            else
            {
                p_client->is_notification_enabled_acc_y = false;
                evt.type                          = BLE_CUBE_EVT_COMM_STOPPED_ACC_Y;
            }

            if (p_cube->data_handler != NULL)
            {
                p_cube->data_handler(&evt);
            }

        }
    }
    //ACC Z
    else if ((p_evt_write->handle == p_cube->acc_z_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled_acc_z = true;
                evt.type                          = BLE_CUBE_EVT_COMM_STARTED_ACC_Z;
            }
            else
            {
                p_client->is_notification_enabled_acc_z = false;
                evt.type                          = BLE_CUBE_EVT_COMM_STOPPED_ACC_Z;
            }

            if (p_cube->data_handler != NULL)
            {
                p_cube->data_handler(&evt);
            }

        }
    }
    //GYRO X
    else if ((p_evt_write->handle == p_cube->gyro_x_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled_gyro_x = true;
                evt.type                          = BLE_CUBE_EVT_COMM_STARTED_GYRO_X;
            }
            else
            {
                p_client->is_notification_enabled_gyro_x = false;
                evt.type                          = BLE_CUBE_EVT_COMM_STOPPED_GYRO_X;
            }

            if (p_cube->data_handler != NULL)
            {
                p_cube->data_handler(&evt);
            }

        }
    }
    //GYRO Y
    else if ((p_evt_write->handle == p_cube->gyro_y_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled_gyro_y = true;
                evt.type                          = BLE_CUBE_EVT_COMM_STARTED_GYRO_Y;
            }
            else
            {
                p_client->is_notification_enabled_gyro_y = false;
                evt.type                          = BLE_CUBE_EVT_COMM_STOPPED_GYRO_Y;
            }

            if (p_cube->data_handler != NULL)
            {
                p_cube->data_handler(&evt);
            }

        }
    }
    //GYRO Z
    else if ((p_evt_write->handle == p_cube->gyro_z_handles.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        if (p_client != NULL)
        {
            if (ble_srv_is_notification_enabled(p_evt_write->data))
            {
                p_client->is_notification_enabled_gyro_z = true;
                evt.type                          = BLE_CUBE_EVT_COMM_STARTED_GYRO_Z;
            }
            else
            {
                p_client->is_notification_enabled_gyro_z = false;
                evt.type                          = BLE_CUBE_EVT_COMM_STOPPED_GYRO_Z;
            }

            if (p_cube->data_handler != NULL)
            {
                p_cube->data_handler(&evt);
            }

        }
    }
    //VALUE CHANGE
    //GAME LEVEL
    else if ((p_evt_write->handle == p_cube->game_level_handles.value_handle) &&
             (p_cube->data_handler != NULL))
    {
        evt.type                  = BLE_CUBE_EVT_RX_GAME_LEVEL;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_cube->data_handler(&evt);
        needToSendNewSettings = 1;
        if(gameGrabCallback != NULL)
        {
            dataStructRec.gameLevel = *evt.params.rx_data.p_data;
        }

    }
    //NUM RETRIES
    else if ((p_evt_write->handle == p_cube->num_retries_handles.value_handle) &&
             (p_cube->data_handler != NULL))
    {
        evt.type                  = BLE_CUBE_EVT_RX_RETRIES_COUNT;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_cube->data_handler(&evt);
        needToSendNewSettings = 1;
        if(gameGrabCallback != NULL)
        {
            dataStructRec.gameLivesMax = *evt.params.rx_data.p_data;
        }
    }
    //CUBE SIDES
    else if ((p_evt_write->handle == p_cube->game_sides_handles.value_handle) &&
             (p_cube->data_handler != NULL))
    {
        evt.type                  = BLE_CUBE_EVT_RX_CUBE_SIDE;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_cube->data_handler(&evt);
        needToSendNewSettings = 1;
        if(gameGrabCallback != NULL)
        {
            dataStructRec.gameSides = *evt.params.rx_data.p_data;
        }
    }
    //BUTTON DUR
    else if ((p_evt_write->handle == p_cube->button_display_duration_handles.value_handle) &&
             (p_cube->data_handler != NULL))
    {
        evt.type                  = BLE_CUBE_EVT_RX_DISPLAY_DUR;
        evt.params.rx_data.p_data = p_evt_write->data;
        evt.params.rx_data.length = p_evt_write->len;

        p_cube->data_handler(&evt);
        needToSendNewSettings = 1;
        if(gameGrabCallback != NULL)
        {
            dataStructRec.buttonDisplayDuration = *evt.params.rx_data.p_data;
        }
    }
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }

    if(needToSendNewSettings != 0)
    {
        if(gameGrabCallback != NULL)//we have a struct
        {
            if(gameNewSettingsCallback != NULL)
            {
                gameNewSettingsCallback(&dataStructRec);
            }
        }
    }
}

/**@brief Function for handling the @ref BLE_GATTS_EVT_HVN_TX_COMPLETE event from the SoftDevice.
 *
 * @param[in] p_nus     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_hvx_tx_complete(ble_cube_t * p_cube, ble_evt_t const * p_ble_evt)
{
    //after each notification has been sent I guess this fires
}

void ble_cube_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    if ((p_context == NULL) || (p_ble_evt == NULL))
    {
        return;
    }

    ble_cube_t * p_cube = (ble_cube_t *)p_context;

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            on_connect(p_cube, p_ble_evt);
            break;

        case BLE_GATTS_EVT_WRITE:
            on_write(p_cube, p_ble_evt);
            break;

        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            on_hvx_tx_complete(p_cube, p_ble_evt);
            break;

        default:
            // No implementation needed.
            break;
    }
}

uint32_t ble_cube_init(ble_cube_t * p_cube, ble_cube_init_t const * p_cube_init)
{
    ret_code_t            err_code;
    ble_uuid_t            ble_uuid;
    ble_uuid128_t         cube_base_uuid = CUBE_BASE_UUID;
    ble_add_char_params_t add_char_params;

    VERIFY_PARAM_NOT_NULL(p_cube);
    VERIFY_PARAM_NOT_NULL(p_cube_init);

    // Initialize the service structure.
    p_cube->data_handler = p_cube_init->data_handler;

    /**@snippet [Adding proprietary Service to the SoftDevice] */
    // Add a custom base UUID.
    err_code = sd_ble_uuid_vs_add(&cube_base_uuid, &p_cube->uuid_type);
    VERIFY_SUCCESS(err_code);

    ble_uuid.type = p_cube->uuid_type;
    ble_uuid.uuid = BLE_UUID_CUBE_SERVICE;

    // Add the service.
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &ble_uuid,
                                        &p_cube->service_handle);
    /**@snippet [Adding proprietary Service to the SoftDevice] */
    VERIFY_SUCCESS(err_code);


    // Add the GAME LEVEL Characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_CUBE_GAME_LEVEL_CHARACTERISTIC;
    add_char_params.uuid_type                = p_cube->uuid_type;
    add_char_params.max_len                  = 1;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = false;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;
    add_char_params.char_props.read          = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->game_level_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the NUM RETRIES Characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_CUBE_NUM_RETRIES_CHARACTERISTIC;
    add_char_params.uuid_type                = p_cube->uuid_type;
    add_char_params.max_len                  = 1;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = false;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;
    add_char_params.char_props.read          = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->num_retries_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the CURR LIFE Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_CURR_LIVES_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(uint8_t);
    add_char_params.init_len          = sizeof(uint8_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;
    add_char_params.char_props.read   = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->curr_lives_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the CUBE SIDE characteristics
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_CUBE_CUBE_SIDES_CHARACTERISTIC;
    add_char_params.uuid_type                = p_cube->uuid_type;
    add_char_params.max_len                  = 1;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = false;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;
    add_char_params.char_props.read          = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->game_sides_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the button duration characteristic.
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid                     = BLE_UUID_CUBE_BUTTON_DURATION_CHARACTERISTIC;
    add_char_params.uuid_type                = p_cube->uuid_type;
    add_char_params.max_len                  = 1;
    add_char_params.init_len                 = sizeof(uint8_t);
    add_char_params.is_var_len               = false;
    add_char_params.char_props.write         = 1;
    add_char_params.char_props.write_wo_resp = 1;
    add_char_params.char_props.read          = 1;

    add_char_params.read_access  = SEC_OPEN;
    add_char_params.write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->button_display_duration_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the ACC X Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_ACC_X_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(int32_t);
    add_char_params.init_len          = sizeof(int32_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->acc_x_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the ACC Y Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_ACC_Y_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(int32_t);
    add_char_params.init_len          = sizeof(int32_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    // Add the ACC Z Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_ACC_Z_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(int32_t);
    add_char_params.init_len          = sizeof(int32_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->acc_z_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the GYRO X Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_GYRO_X_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(int32_t);
    add_char_params.init_len          = sizeof(int32_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->gyro_x_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the GYRO Y Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_GYRO_Y_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(int32_t);
    add_char_params.init_len          = sizeof(int32_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    err_code = characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->gyro_y_handles);
    if (err_code != NRF_SUCCESS)
    {
        return err_code;
    }

    // Add the GYRO Z Characteristic.
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
    memset(&add_char_params, 0, sizeof(add_char_params));
    add_char_params.uuid              = BLE_UUID_CUBE_GYRO_Z_CHARACTERISTIC;
    add_char_params.uuid_type         = p_cube->uuid_type;
    add_char_params.max_len           = sizeof(int32_t);
    add_char_params.init_len          = sizeof(int32_t);
    add_char_params.is_var_len        = false;
    add_char_params.char_props.notify = 1;

    add_char_params.read_access       = SEC_OPEN;
    add_char_params.write_access      = SEC_OPEN;
    add_char_params.cccd_write_access = SEC_OPEN;

    return characteristic_add(p_cube->service_handle, &add_char_params, &p_cube->gyro_z_handles);
    /**@snippet [Adding proprietary characteristic to the SoftDevice] */
}

void float2Bytes(float val,uint8_t * bytes_array){
  // Create union of shared memory space
  union {
    int32_t int32_variable;
    uint8_t temp_array[4];
  } u;
  // Overite bytes of union with float variable
  u.int32_variable = val;
  // Assign bytes to input array
  memcpy(bytes_array, u.temp_array, 4);
}

//////////////////////////////////////////////////////
uint32_t write_acc_x(ble_cube_t * p_cube,
                     ble_cube_data_struct_t   * p_data,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value[4] = {0};
    uint16_t length = 4;

    float2Bytes(p_data->acc_x,value);
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_cube->acc_x_handles.value_handle;
    hvx_params.p_data = value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
//////////////////////////////////////////////////////
uint32_t write_acc_y(ble_cube_t * p_cube,
                     ble_cube_data_struct_t   * p_data,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value[4] = {0};
    uint16_t length = 4;

    float2Bytes(p_data->acc_y,value);
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_cube->acc_y_handles.value_handle;
    hvx_params.p_data = value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
//////////////////////////////////////////////////////
uint32_t write_acc_z(ble_cube_t * p_cube,
                     ble_cube_data_struct_t   * p_data,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value[4] = {0};
    uint16_t length = 4;

    float2Bytes(p_data->acc_z,value);
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_cube->acc_z_handles.value_handle;
    hvx_params.p_data = value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
//////////////////////////////////////////////////////
uint32_t write_gyro_x(ble_cube_t * p_cube,
                     ble_cube_data_struct_t   * p_data,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value[4] = {0};
    uint16_t length = 4;

    float2Bytes(p_data->gyro_x,value);
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_cube->gyro_x_handles.value_handle;
    hvx_params.p_data = value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
//////////////////////////////////////////////////////
uint32_t write_gyro_y(ble_cube_t * p_cube,
                     ble_cube_data_struct_t   * p_data,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value[4] = {0};
    uint16_t length = 4;

    float2Bytes(p_data->gyro_y,value);
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_cube->gyro_y_handles.value_handle;
    hvx_params.p_data = value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
//////////////////////////////////////////////////////
uint32_t write_gyro_z(ble_cube_t * p_cube,
                     ble_cube_data_struct_t   * p_data,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value[4] = {0};
    uint16_t length = 4;

    float2Bytes(p_data->gyro_z,value);
    memset(&hvx_params, 0, sizeof(hvx_params));
    
    hvx_params.handle = p_cube->gyro_z_handles.value_handle;
    hvx_params.p_data = value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}
///////////////////////////////////////////////////////////
uint32_t ble_cube_data_send(ble_cube_t * p_cube,
                           ble_cube_data_struct_t   * p_data,
                           uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    ble_cube_client_context_t * p_client;

    VERIFY_PARAM_NOT_NULL(p_cube);

    err_code = blcm_link_ctx_get(p_cube->p_link_ctx_storage, conn_handle, (void *) &p_client);
    VERIFY_SUCCESS(err_code);

    if ((conn_handle == BLE_CONN_HANDLE_INVALID) || (p_client == NULL))
    {
        return NRF_ERROR_NOT_FOUND;
    }

    if (p_client->is_notification_enabled_acc_x)
    {
        write_acc_x(p_cube,p_data,conn_handle);
    }
    if (p_client->is_notification_enabled_acc_y)
    {
        write_acc_y(p_cube,p_data,conn_handle);
    }
    if (p_client->is_notification_enabled_acc_z)
    {
        write_acc_z(p_cube,p_data,conn_handle);
    }
    if (p_client->is_notification_enabled_gyro_x)
    {
        write_gyro_x(p_cube,p_data,conn_handle);
    }
    if (p_client->is_notification_enabled_gyro_y)
    {
        write_gyro_y(p_cube,p_data,conn_handle);
    }
    if (p_client->is_notification_enabled_gyro_z)
    {
        write_gyro_z(p_cube,p_data,conn_handle);
    }
}


void ble_cube_registerGameLogicGrabCallback(ble_cube_grab_game_logic_callback callback)
{
    gameGrabCallback = callback;
}

void ble_cube_registerGameLogicNewSettings(ble_cube_receive_new_settings_callback callback)
{
    gameNewSettingsCallback = callback;
}

//////////////////////////////////////////////////////
uint32_t write_curr_life(ble_cube_t * p_cube,
                     uint8_t currLife,
                     uint16_t    conn_handle)
{
    ret_code_t                 err_code;
    ble_gatts_hvx_params_t     hvx_params;
    uint8_t value = 0;
    uint16_t length = 1;

    memset(&hvx_params, 0, sizeof(hvx_params));
    
    value = currLife;
    hvx_params.handle = p_cube->curr_lives_handles.value_handle;
    hvx_params.p_data = &value;
    hvx_params.p_len  = &length;
    hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;

    return sd_ble_gatts_hvx(conn_handle, &hvx_params);
}

