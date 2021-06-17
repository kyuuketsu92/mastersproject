#include "usb_main.h"

//usb
#include "nrf_drv_usbd.h"
#include "app_usbd_core.h"
#include "app_usbd.h"
#include "app_usbd_string_desc.h"
#include "app_usbd_cdc_acm.h"
#include "app_usbd_serial_num.h"

#define ENDLINE_STRING "\r\n"

//usb definitions
// USB DEFINES START
static void cdc_acm_user_ev_handler(app_usbd_class_inst_t const * p_inst,
                                    app_usbd_cdc_acm_user_event_t event);

#define CDC_ACM_COMM_INTERFACE  0
#define CDC_ACM_COMM_EPIN       NRF_DRV_USBD_EPIN2

#define CDC_ACM_DATA_INTERFACE  1
#define CDC_ACM_DATA_EPIN       NRF_DRV_USBD_EPIN1
#define CDC_ACM_DATA_EPOUT      NRF_DRV_USBD_EPOUT1

static char m_cdc_data_array[20];

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

static received_command_callback cdc_callback = NULL;


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
            //ret = app_timer_stop(m_blink_cdc);
            //APP_ERROR_CHECK(ret);
            //bsp_board_led_on(LED_CDC_ACM_CONN);
            //NRF_LOG_INFO("CDC ACM port opened");
            break;
        }

        case APP_USBD_CDC_ACM_USER_EVT_PORT_CLOSE:
            //NRF_LOG_INFO("CDC ACM port closed");
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
                        //NRF_LOG_DEBUG("Ready to send data over BLE NUS");
                        //NRF_LOG_HEXDUMP_DEBUG(m_cdc_data_array, index);

                        do
                        {
                            uint16_t length = (uint16_t)index;
                            if (length + sizeof(ENDLINE_STRING) < 20)
                            {
                                memcpy(m_cdc_data_array + length, ENDLINE_STRING, sizeof(ENDLINE_STRING));
                                length += sizeof(ENDLINE_STRING);
                            }

                            // callback to main where the commands will get evaluated and sorted out
                            if(cdc_callback != NULL)
                            {
                                cdc_callback(m_cdc_data_array, 20, 0);
                            }

                            if (ret == NRF_ERROR_NOT_FOUND)
                            {
                                //NRF_LOG_INFO("BLE NUS unavailable, data received: %s", m_cdc_data_array);
                                break;
                            }

                            if (ret == NRF_ERROR_RESOURCES)
                            {
                                //NRF_LOG_ERROR("BLE NUS Too many notifications queued.");
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
                //NRF_LOG_DEBUG("RX: size: %lu char: %c", size, m_cdc_data_array[index - 1]);

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
            //NRF_LOG_INFO("USB power detected");

            if (!nrf_drv_usbd_is_enabled())
            {
                app_usbd_enable();
            }
            break;

        case APP_USBD_EVT_POWER_REMOVED:
        {
            //NRF_LOG_INFO("USB power removed");
            //ret_code_t err_code = app_timer_stop(m_blink_cdc);
            //APP_ERROR_CHECK(err_code);
            //bsp_board_led_off(LED_CDC_ACM_CONN);
            m_usb_connected = false;
            app_usbd_stop();
        }
            break;

        case APP_USBD_EVT_POWER_READY:
        {
            //NRF_LOG_INFO("USB ready");
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

ret_code_t USB_main_write(char * buffer, uint16_t size)
{
    static char inbuffer[50] = {0};

    memcpy(inbuffer,buffer,size);
    ret_code_t ret;
    ret = app_usbd_cdc_acm_write(&m_app_cdc_acm,inbuffer,size);
    if(ret != NRF_SUCCESS)
    {
        //NRF_LOG_INFO("CDC ACM unavailable");
    }
}

bool USB_main_queue_process(void)
{
    return app_usbd_event_queue_process();
}

void USB_main_power_enable(void)
{
    ret_code_t ret;
    ret = app_usbd_power_events_enable();
    APP_ERROR_CHECK(ret);
}

void USB_main_serial_gen(void)
{
    app_usbd_serial_num_generate();
}

void USB_main_init(void)
{
    ret_code_t ret;
    static const app_usbd_config_t usbd_config = {
        .ev_state_proc = usbd_user_ev_handler
    };

    ret = app_usbd_init(&usbd_config);
    APP_ERROR_CHECK(ret);

    app_usbd_class_inst_t const * class_cdc_acm = app_usbd_cdc_acm_class_inst_get(&m_app_cdc_acm);
    ret = app_usbd_class_append(class_cdc_acm);
    APP_ERROR_CHECK(ret);
}

//A way to pass out the commands that are received through the BLE NUS service
void USB_register_cdc_callback(received_command_callback callback)
{
    cdc_callback = callback;
}