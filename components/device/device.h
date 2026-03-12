#ifndef DEVICE_H
#define DEVICE_H

#include <stdint.h>
#include "esp_err.h"
#include "esp_log.h"
#include <time.h>
#include "data_storage.h"
#include "cJSON.h"

#define ESP_ERR_BUFFER_NOT_FULL (ESP_FAIL + 1)  // Increment from base error code
#define ESP_ERR_BUFFER_EMPTY (ESP_FAIL + 2)  // Increment from base error code
#define ESP_ERR_DATA_NOT_VALID (ESP_FAIL + 2)  // Increment from base error code

#define DEBUG 0
#define OLED_DISP 1

#define KEY_DESCRIPTION "description"
#define KEY_DEVICE_TYPE "device_type"
#define KEY_DEVICE_NAME "device_name"
#define KEY_AVERAGE_DATA "average_data"
#define KEY_DATA_BUFFER_SIZE "buffer_size"
#define KEY_HOLD_TIME "hold_time"
#define KEY_TEMP_HIGH "temp_high"
#define KEY_TEMP_LOW "temp_low"
#define KEY_HUM_HIGH "hum_high"
#define KEY_HUM_LOW "hum_low"
#define KEY_TRIGGER_TYPE "trigger_type"
#define KEY_CURRENT "current"
#define KEY_TIME "time"




// Sensor 1
// #define DEVICE_ID 4000
// #define DEVICE_DESCRIPTION "Gowning room 819 A"
// #define DEVICE_NAME "TH_GR"

//Sensor 2 
// #define DEVICE_ID 4001
// #define DEVICE_DESCRIPTION "Deposition room 819C"
// #define DEVICE_NAME "TH_DR"

//Sensor 3
// #define DEVICE_ID 4002
// #define DEVICE_DESCRIPTION "Etch room 819D"
// #define DEVICE_NAME "TH_ER"

//Sensor 4
// #define DEVICE_ID 4003
// #define DEVICE_DESCRIPTION "EBL room 819E"
// #define DEVICE_NAME "TH_EBLR"

//Sensor 5
#define DEVICE_ID 4004
#define DEVICE_DESCRIPTION "Yellow room 819F"
#define DEVICE_NAME "TH_YR"




#define DEVICE_TYPE 4

#define DATA_BUFFER_MAX_SIZE 100


#define VAL_TEMP "temp"
#define VAL_HUM "hum"
#define VAL_TIME "time"


typedef struct {
    float temp;
    float hum;
    uint32_t time;
} THData;

esp_err_t get_average(float *avg_temp, float *avg_hum, uint32_t *last_time);
void add_sample(float temp, float hum);
esp_err_t get_last_sample(THData *last_sample);
void reset_counter();
esp_err_t get_device_data(cJSON *data_array);

#endif