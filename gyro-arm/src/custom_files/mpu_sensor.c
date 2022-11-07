
#include "mpu_sensor.h"
#include "nrfx_twim.h"

#define LOG_MODULE_NAME mpu_sensor
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define MPU_TWI_BUFFER_SIZE             14
#define MPU_TWI_TIMEOUT                 10000
#define MPU_ADDRESS                     0x68

const nrfx_twim_t my_twim_instance       = NRFX_TWIM_INSTANCE(0);
volatile static bool twi_xfer_done      = false;
uint8_t twi_tx_buffer[MPU_TWI_BUFFER_SIZE];

int 
app_mpu_tx(const nrfx_twim_t *  p_instance,
                uint8_t             address,
                uint8_t *           p_data,
                uint8_t             length,
                bool                no_stop)
{
    int err;

    nrfx_twim_xfer_desc_t xfer = NRFX_TWIM_XFER_DESC_TX(address, p_data, length);
    err = nrfx_twim_xfer(p_instance, &xfer, 0);
    if (err != NRFX_SUCCESS) {
        return err;
    }

    return 0;
}

int 
app_mpu_rx(const nrfx_twim_t *   p_instance,
               uint8_t               address,
               uint8_t *             p_data,
               uint8_t               length)
{
    int err;
    nrfx_twim_xfer_desc_t xfer = NRFX_TWIM_XFER_DESC_RX(address, p_data, length);

    err = nrfx_twim_xfer(p_instance, &xfer, 0);
    if (err != NRFX_SUCCESS) {
        return err;
    }
    return 0;
}

int 
wait_for_xfer_done(void)
{
    int timeout = MPU_TWI_TIMEOUT;
    while ((!twi_xfer_done) && --timeout)
    {
        // Wait...
    }
    if (timeout == 0) {
        return NRFX_ERROR_TIMEOUT;
    }
    return 0;
}

int 
app_mpu_write_single_register(uint8_t reg, uint8_t data)
{
    int err;

    uint8_t packet[2] = {reg, data};

    twi_xfer_done = false;  // reset for new xfer
    err = app_mpu_tx(&my_twim_instance, MPU_ADDRESS, packet, 2, false);
    if (err) {
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }
    
    return 0;
}

int 
app_mpu_write_registers(uint8_t reg, uint8_t * p_data, uint8_t length)
{
    int err;
    
    twi_tx_buffer[0] = reg;
    memcpy((twi_tx_buffer + 1), p_data, length);

    nrfx_twim_xfer_desc_t xfer = {0};
    xfer.address = MPU_ADDRESS;
    xfer.type = NRFX_TWIM_XFER_TX;
    xfer.primary_length = length+1;
    xfer.p_primary_buf = twi_tx_buffer;

    twi_xfer_done = false;  // reset for new xfer
    err = nrfx_twim_xfer(&my_twim_instance, &xfer,0);
    if (err != NRFX_SUCCESS) {
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }

    return 0;
    
}

int 
app_mpu_read_registers(uint8_t reg, uint8_t * p_data, uint8_t length)
{
    int err;

    twi_xfer_done = false;  // reset for new xfer
    err = app_mpu_tx(&my_twim_instance, MPU_ADDRESS, &reg, 1, false);
    if (err) {
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }

    twi_xfer_done = false;  // reset for new xfer
    err = app_mpu_rx(&my_twim_instance,MPU_ADDRESS, p_data, length);
    if (err) {
        LOG_ERR("app_mpu_rx returned %08x", err);
        return err;
    }
    err = wait_for_xfer_done();
    if (err == NRFX_ERROR_TIMEOUT) {
        return err;
    }

    return 0;
}

void 
my_twim_handler(nrfx_twim_evt_t const * p_event, void * p_context)
{
    // LOG_INF("TWIM callback");
    switch(p_event->type)
    {
        case NRFX_TWIM_EVT_DONE:
            twi_xfer_done = true;   // This is the event we are waiting for.
            break;
        case NRFX_TWIM_EVT_ADDRESS_NACK:
            LOG_ERR("address nack");
            break;
        case NRFX_TWIM_EVT_DATA_NACK:
            LOG_ERR("data nack");
            break;
        case NRFX_TWIM_EVT_OVERRUN:
            LOG_ERR("overrun");
            break;
        case NRFX_TWIM_EVT_BUS_ERROR:
            LOG_ERR("bus error");
            break;
        default:
            LOG_ERR("default (should never happen)");
            break;
    }
    
}


int
twi_init(void)
{
        const nrfx_twim_config_t twim_config = {                      
        .scl                = 4,                                       // These particular gpios are used because they are close to both VDD and GND.
        .sda                = 3,                                       // These particular gpios are used because they are close to both VDD and GND.
        .frequency          = NRF_TWIM_FREQ_400K,                     
        .interrupt_priority = NRFX_TWIM_DEFAULT_CONFIG_IRQ_PRIORITY,  
        .hold_bus_uninit    = false,                                  
    };

    // Setup peripheral interrupt.
    IRQ_CONNECT(DT_IRQN(DT_NODELABEL(i2c0)),DT_IRQ(DT_NODELABEL(i2c0), priority), nrfx_isr, nrfx_twim_0_irq_handler,0);
    irq_enable(DT_IRQN(DT_NODELABEL(i2c0)));

    int err = nrfx_twim_init(&my_twim_instance, 
                             &twim_config, 
                             my_twim_handler, 
                             NULL);

    if (err != NRFX_SUCCESS) {
        LOG_ERR("Error initializing twim (err: %x)", err);
        return err;
    }

    nrfx_twim_enable(&my_twim_instance);

    return 0;
}

int
app_mpu_config(void)
{
    app_mpu_config_t mpu_config = {
        .smplrt_div                     = 19,             \
        .sync_dlpf_config.dlpf_cfg      = 1,              \
        .sync_dlpf_config.ext_sync_set  = 0,              \
        .gyro_config.fs_sel             = GFS_2000DPS,    \
        .accel_config.afs_sel           = AFS_2G,         \
        .accel_config.za_st             = 0,              \
        .accel_config.ya_st             = 0,              \
        .accel_config.xa_st             = 0,              \
    };

    uint8_t* data;
    data = (uint8_t*)&mpu_config;

    int err = app_mpu_write_registers(MPU_REG_SMPLRT_DIV, data, 4);
    return err;
}

int 
mpu_sensor_init(void) 
{
    LOG_INF("Initializing MPU Sensor");
    
    int err = twi_init();
    if (err) {
        return err; // I know that this seems unnecessary, since we return err right below. But we will add more in between later.
    }

    err = app_mpu_config();
    err = app_mpu_write_single_register(MPU_REG_SIGNAL_PATH_RESET, 7);
    if (err == NRFX_ERROR_TIMEOUT) {
        LOG_ERR("MPU register write timed out trying to reset accel, gyro and temp");
        return err;
    }

    err = app_mpu_write_single_register(MPU_REG_PWR_MGMT_1, 1);
    if (err == NRFX_ERROR_TIMEOUT) {
        LOG_ERR("MPU register write timed out trying to set power management to pll with x axis gyroscope reference");
        return err;
    }

    err = app_mpu_config();

    return err;
}

int
read_accel_values(struct accel_values* accel_values)
{
    int err;
    uint8_t raw_values[6];
    err = app_mpu_read_registers(MPU_REG_ACCEL_XOUT_H, raw_values, 6);
    if (err) {
        LOG_ERR("Could not read accellerometer data. err: %d", err);
        return err;
    }

    accel_values->x = ((raw_values[0]<<8) + raw_values[1]);
    accel_values->y = ((raw_values[2]<<8) + raw_values[3]);
    accel_values->z = ((raw_values[4]<<8) + raw_values[5]);

    return 0;
}

int
read_gyro_values(struct gyro_values* gyro_values)
{
    int err;
    uint8_t raw_values[6];
    err = app_mpu_read_registers(MPU_REG_GYRO_XOUT_H, raw_values, 6);
    if (err) {
        LOG_ERR("Could not read gyro data. err: %d", err);
        return err;
    }

    gyro_values->x = ((raw_values[0]<<8) + raw_values[1]);
    gyro_values->y = ((raw_values[2]<<8) + raw_values[3]);
    gyro_values->z = ((raw_values[4]<<8) + raw_values[5]);

    return 0;
}