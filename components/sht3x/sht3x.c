#include "sht3x.h"

static const char *TAG = "sht3x";

static i2c_master_dev_handle_t dev_handle;

const float CORR_TEMP = -3.2;  // Correction value for temperature in F
const float CORR_HUM = 1.5;   // Correction value for humidity in %

esp_err_t sht3x_init()
{   
    ESP_ERROR_CHECK(i2c_add_device(SHT3x_DEVICE_ADDR, &dev_handle));
    return ESP_OK;
}

uint8_t sht3x_crc(uint8_t *data) {

    uint8_t crc = 0xff;
    int i, j;
    for(i = 0; i < 2; i++) {
        crc ^= data[i];
        for(j = 0; j < 8; j++) {
            if(crc & 0x80) {
                crc <<= 1;
                crc ^= 0x131;
            }
            else
                crc <<= 1;
        }
    }
    return crc;
}

esp_err_t sht3x_read_temp_humi(float *temp, float *humi)
{
    uint8_t data_wr[] = {0x24, 0x00};
    uint8_t data_rd[6];
    esp_err_t ret;
    int retry_count = 0;
    const int max_retries = 3;

    while (retry_count < max_retries) {
        // Write command
        ret = i2c_master_transmit(dev_handle, data_wr, sizeof(data_wr), 1000 / portTICK_PERIOD_MS);
        if (ret == ESP_OK) {
            // Delay 20 ms
            vTaskDelay(pdMS_TO_TICKS(20));

            // Read 6 bytes
            ret = i2c_master_receive(dev_handle, data_rd, sizeof(data_rd), 1000 / portTICK_PERIOD_MS);
            if (ret == ESP_OK) {
                // Check CRC
                if (data_rd[2] == sht3x_crc(data_rd) && 
                    data_rd[5] == sht3x_crc(data_rd + 3)) {
                    // Calculate temperature and humidity
                    *temp = -49.0 + (315 * ((float)(data_rd[0] << 8 | data_rd[1]) / 65535.0)) + CORR_TEMP;
                    *humi = 100.0 * ((float)(data_rd[3] << 8 | data_rd[4]) / 65535.0) + CORR_HUM;
                    return ESP_OK;
                } else {
                    ret = ESP_ERR_INVALID_CRC;
                }
            }
        }

        // Check if the error is a NACK
        if (ret == ESP_ERR_TIMEOUT) {
            ESP_LOGW(TAG, "SHT3x NACK detected. Retrying...");
            vTaskDelay(pdMS_TO_TICKS(10)); // Short delay before retry
            retry_count++;
            continue;
        }

        // If we reach here, there was an error. Implement recovery mechanism.
        retry_count++;
        ESP_LOGW(TAG, "SHT3x read attempt %d failed with error %d. Retrying...", retry_count, ret);
        
        // Perform a soft reset of the sensor
        uint8_t reset_cmd[] = {0x30, 0xA2};
        i2c_master_transmit(dev_handle, reset_cmd, sizeof(reset_cmd), 1000 / portTICK_PERIOD_MS);
        vTaskDelay(pdMS_TO_TICKS(15)); // Wait for reset to complete
    }

    ESP_LOGE(TAG, "SHT3x read failed after %d attempts", max_retries);
    return ret;
}

esp_err_t sht3x_delete_dev(){
    return i2c_master_bus_rm_device(dev_handle); 
}