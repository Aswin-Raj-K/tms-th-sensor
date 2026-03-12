#ifndef SHT3x_H
#define SHT3x_H

#include "esp_log.h"
#include "driver/i2c_master.h"
#include "freertos/FreeRTOS.h"
#include "i2c.h"


extern const float CORR_TEMP;  // Correction value for temperature
extern const float CORR_HUM;   // Correction value for humidity

#define SHT3x_DEVICE_ADDR 0x44


#define ACK_CHECK_EN 0x1
#define ACK_CHECK_DIS 0x0
#define ACK_VAL 0x0
#define NACK_VAL 0x1


esp_err_t sht3x_init(void);

uint8_t sht3x_crc(uint8_t *data);

esp_err_t sht3x_read_temp_humi(float *temp, float *humi);

esp_err_t sht3x_delete_dev();

#endif