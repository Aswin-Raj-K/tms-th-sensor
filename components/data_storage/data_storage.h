#ifndef DATA_STORAGE_H
#define DATA_STORAGE_H

#include <string.h>
#include <stdlib.h>
#include <inttypes.h>
#include "nvs_flash.h"
#include "esp_log.h"
#include "esp_err.h"


#define ST_HOLD_TIME "hold_time"
#define ST_DATA_BUFFER_SIZE "buffer_size"
#define ST_AVERAGE_DATA "average_data"
#define ST_TEMP_HIGH "temp_high"
#define ST_TEMP_LOW "temp_low"
#define ST_HUM_HIGH "hum_high"
#define ST_HUM_LOW "hum_low"

#define ST_STATE "state"


#define STATE_REBOOT 0
#define STATE_RESTART 1
#define STATE_UNKNOWN 2

#define SCREEN_UPDATE_TIME 5//Screen update time in seconds
#define DEFAULT_HOLD_TIME 5
#define RESTART_DELAY_MIN 10
#define DATA_BUFFER_SIZE 10 
#define AVERAGE_DATA 0

//temperature in F
#define TEMP_HIGH 80
#define TEMP_LOW 60

//humidity in %
#define HUM_HIGH 50
#define HUM_LOW 40


#define GET_STATE_NAME(state_num) \
    ((state_num) == STATE_REBOOT ? "REBOOT" : \
     (state_num) == STATE_RESTART ? "RESTART" : \
     "UNKNOWN")

typedef struct {
    int32_t hold_time;
    int32_t data_buffer_size;
    int32_t state;
    bool average_data;
    int32_t temp_high;
    int32_t temp_low;
    int32_t hum_high;
    int32_t hum_low;
} Storage;


void read_int(char *key, int32_t *data, int32_t default_data);
void write_int(char *key, int32_t data);

void log_storage();

esp_err_t read_data(nvs_handle *handle, int32_t *data, char *key);

void init_storage();

int32_t get_state();
void set_state(int32_t state);

int32_t get_hold_time();
void set_hold_time(int32_t hold_time);

bool is_data_average();
void set_data_average(bool is_average);

int32_t get_buffer_size();
void set_buffer_size(int32_t buffer_size);

void get_triggers(int32_t* triggers);
void set_triggers(int32_t *triggers);


void clear_storage();

#endif