#ifndef __USB_MAIN_H__
#define __USB_MAIN_H__

#include "stdbool.h"
#include "stdint.h"
#include "app_error.h"

#define UART_TX_BUF_SIZE                256                                         /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE                256                                         /**< UART RX buffer size. */

typedef void (*received_command_callback)(char * dataOut, uint16_t bufferSize, uint8_t source); //USB source 0, BLE NUS source 1

void USB_main_serial_gen(void);
void USB_main_init(void);
void USB_main_power_enable(void);
bool USB_main_queue_process(void);
ret_code_t USB_main_write(char * buffer, uint16_t size);
void USB_register_cdc_callback(received_command_callback callback);






#endif