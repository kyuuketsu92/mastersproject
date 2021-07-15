#include "accelerometer.h"
//i2c
#include "nrf_drv_twi.h"

/* Buffer for samples read from temperature sensor. */
static uint8_t m_sample[14] = {0};
static ACCEL_struct_t ACCEL_data_struct = {0};

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
                ACCEL_data_struct.accX = m_sample[0] << 8 | m_sample[1];
                ACCEL_data_struct.accY = m_sample[2] << 8 | m_sample[3];
                ACCEL_data_struct.accZ = m_sample[4] << 8 | m_sample[5];
                ACCEL_data_struct.temp = m_sample[6] << 8 | m_sample[7];
                ACCEL_data_struct.rotX = m_sample[8] << 8 | m_sample[9];
                ACCEL_data_struct.rotY = m_sample[10] << 8 | m_sample[11];
                ACCEL_data_struct.rotZ = m_sample[12] << 8 | m_sample[13];
            }
            m_xfer_done = true;
            break;
        default:
            break;
    }
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

void ACCEL_init()
{
    twi_init();
}
void ACCEL_start()
{
    acc_turn_on();
}
void ACCEL_read()
{
    read_sensor_data();
}

void ACCEL_get_data(ACCEL_struct_t * out_struct)
{
     *out_struct = ACCEL_data_struct;
}