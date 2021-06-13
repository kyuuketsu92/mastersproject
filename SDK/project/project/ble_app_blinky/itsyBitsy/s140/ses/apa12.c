#include "apa12.h"
#include "nrf_drv_spi.h"

#define SPI_INSTANCE  2 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE);  /**< SPI instance. */
static volatile bool spi_xfer_done = true;  /**< Flag used to indicate that SPI instance completed the transfer. */

static uint8_t       m_tx_buf[12]={0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,0xFF,0xFF,0xFF,0xFF}; //static, blue, green, red
static uint8_t       m_rx_buf[1];
/**
 * @brief SPI user event handler.
 * @param event
 */
void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}


uint32_t apa12led_init(uint32_t dataPin, uint32_t clockPin)
{
    uint32_t ret_code = NRF_SUCCESS;
    
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.mode = NRF_DRV_SPI_MODE_0;
    spi_config.frequency = NRF_SPI_FREQ_500K;
    spi_config.ss_pin   = NULL;
    spi_config.miso_pin = NULL;
    spi_config.mosi_pin = dataPin;
    spi_config.sck_pin  = clockPin;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spi_event_handler, NULL));

    return  ret_code;
}

uint32_t apa12led_transfer(void)
{
    uint32_t ret_code = NRF_SUCCESS;
    if(spi_xfer_done == true)
    {
        
        spi_xfer_done = false;
    
        APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, m_tx_buf, 12, m_rx_buf, 1));
    }
    return  ret_code;
}

void apa12led_add_blue(uint8_t amount)
{
  if(((int32_t)m_tx_buf[5] +(int32_t)amount) > 255)
  {
      m_tx_buf[5] = 0xFF;
  }
  else
  {
      m_tx_buf[5] += amount;
  }
}

void apa12led_sub_blue(uint8_t amount)
{
  if(((int32_t)m_tx_buf[5] -(int32_t)amount) < 0)
  {
      m_tx_buf[5] = 0;
  }
  else
  {
      m_tx_buf[5] -= amount;
  }
}

void apa12led_add_green(uint8_t amount)
{
  if(((int32_t)m_tx_buf[6] +(int32_t)amount) > 255)
  {
      m_tx_buf[6] = 0xFF;
  }
  else
  {
      m_tx_buf[6] += amount;
  }
}

void apa12led_sub_green(uint8_t amount)
{
  if(((int32_t)m_tx_buf[6] -(int32_t)amount) < 0)
  {
      m_tx_buf[6] = 0;
  }
  else
  {
      m_tx_buf[6] -= amount;
  }
}

void apa12led_add_red(uint8_t amount)
{
  if(((int32_t)m_tx_buf[7] +(int32_t)amount) > 255)
  {
      m_tx_buf[7] = 0xFF;
  }
  else
  {
      m_tx_buf[7] += amount;
  }
}

void apa12led_sub_red(uint8_t amount)
{
  if(((int32_t)m_tx_buf[7] -(int32_t)amount) < 0)
  {
      m_tx_buf[7] = 0;
  }
  else
  {
      m_tx_buf[7] -= amount;
  }
}

void apa12led_clear(void)
{
  m_tx_buf[5] = 0;
  m_tx_buf[6] = 0;
  m_tx_buf[7] = 0;
}

void apa12led_setRed(uint8_t amount)
{
    m_tx_buf[7] = amount;
}

void apa12led_setGreen(uint8_t amount)
{
    m_tx_buf[6] = amount;
}

void apa12led_setBlue(uint8_t amount)
{
    m_tx_buf[5] = amount;
}