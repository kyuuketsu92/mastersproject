#include "adc_driver.h"
#include "nrf_drv_spi.h"
#include "string.h"

static uint8_t rx_buff[3];
static uint8_t tx_buff[1];

adc_callback callback_handler = NULL;
static bool reading = false;



#define SPI_INSTANCE  1 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool adc_spi_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */

/**
 * @brief SPI user event handler.
 * @param event
 */
void adc_spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    volatile uint16_t data = 0;
    if(reading)
    {
        memset(rx_buff,0,3);
        nrf_drv_spi_transfer(&spi,tx_buff,0,rx_buff,3);
        reading = false;
    }
    else
    {
      adc_spi_xfer_done = true;
      //convert rxbuff into the uint16
      data = ((uint16_t)rx_buff[0]) << 4;
      data += rx_buff[1] >> 4;
      if(callback_handler != NULL)
      {
          callback_handler(data);
      }
    }
}  


uint32_t adc_init(uint32_t dataOutPin, uint32_t clockPin, uint32_t dataInPin, adc_callback handler)
{
    uint32_t ret_code = NRF_SUCCESS;
    
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.mode = NRF_DRV_SPI_MODE_0;
    spi_config.frequency = SPI_FREQUENCY_FREQUENCY_K125;
    spi_config.ss_pin   = NULL;
    spi_config.miso_pin = dataInPin;
    spi_config.mosi_pin = dataOutPin;
    spi_config.sck_pin  = clockPin;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, adc_spi_event_handler, NULL));
    callback_handler = handler;
    return  ret_code;
}



uint32_t adc_read(uint8_t channel)
{
    uint32_t ret_code = NRF_SUCCESS;
    if(adc_spi_xfer_done == true)
    {
        adc_spi_xfer_done = false;
        tx_buff[0] = 0x18;
        tx_buff[0] += (channel);
        nrf_drv_spi_transfer(&spi,tx_buff,1,rx_buff,0);
        reading = true;
    }
    else
    {
        ret_code = NRF_ERROR_BUSY;
    }
    return ret_code;
}