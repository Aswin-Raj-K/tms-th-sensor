#ifndef I2C_H
#define I2C_H

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"

#define I2C_CLOCK_SPEED_HZ 400000 //400kHz
#define I2C_MASTER_TX_BUF_DISABLE 0 
#define I2C_MASTER_RX_BUF_DISABLE 0
#define WRITE_BIT I2C_MASTER_WRITE            
#define READ_BIT I2C_MASTER_READ

#define SDA_IO_NUM 22
#define SCL_IO_NUM 23


esp_err_t i2c_init(void);
esp_err_t i2c_add_device(uint16_t device_addr, i2c_master_dev_handle_t *dev_handle);
esp_err_t i2c_delete_handle();

#endif