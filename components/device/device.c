#include "device.h"

static const char *TAG = "device";

THData data_buffer[DATA_BUFFER_MAX_SIZE];
int head  = 0;
int count = 0;

void add_sample(float temp, float hum) {
    time_t now;
    
    time(&now); // Get current time
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
    ESP_LOGI(TAG, "Current time in EST: %s", strftime_buf);

    data_buffer[head].temp = temp;
    data_buffer[head].hum = hum;
    data_buffer[head].time = (uint32_t)now;

    head = (head + 1) % get_buffer_size(); // Move head to the next position
    if (count < get_buffer_size()) {
        count++;
    }
}

esp_err_t get_average(float *avg_temp, float *avg_hum, uint32_t *last_time) {
    ESP_LOGI(TAG, "data count: %d", count);

    if(count != (int)get_buffer_size())
        return ESP_ERR_BUFFER_NOT_FULL;

    float temp_sum = 0.0, hum_sum = 0.0;
    for (int i = 0; i < count; i++) {
        temp_sum += data_buffer[i].temp;
        hum_sum += data_buffer[i].hum;
    }
    *avg_temp = temp_sum / count;
    *avg_hum = hum_sum / count;
    *last_time = data_buffer[(head - 1 + get_buffer_size()) % get_buffer_size()].time;
    return ESP_OK;
}

esp_err_t get_last_sample(THData *last_sample){
    if (count == 0) {
        // Return an error if the buffer is empty
        ESP_LOGE("get_last_sample", "Buffer is empty.");
        return ESP_ERR_BUFFER_EMPTY;
    }

    // Get the index of the last sample
    int last_index = (head - 1 + get_buffer_size()) % get_buffer_size();
    *last_sample = data_buffer[last_index];  // Return the last sample
    return ESP_OK;
}

void reset_counter(){
    count = 0;
    head = 0;
}


esp_err_t get_device_data(cJSON *data_array) {
    if (count == 0) {
        ESP_LOGW(TAG, "Data buffer is empty, no data to add to JSON array.");
        return ESP_ERR_BUFFER_EMPTY;
    }

    // Iterate through the buffer and add each entry to the JSON array
    for (int i = 0; i < count; i++) {
        int index = (head - count + i + get_buffer_size()) % get_buffer_size();

        cJSON *data_object = cJSON_CreateObject();
        if (data_object == NULL) {
            ESP_LOGE(TAG, "Failed to create JSON object for data entry.");
            continue;
        }

        // Add temperature, humidity, and timestamp to the JSON object
        cJSON_AddNumberToObject(data_object, VAL_TEMP, data_buffer[index].temp);
        cJSON_AddNumberToObject(data_object, VAL_HUM, data_buffer[index].hum);
        cJSON_AddNumberToObject(data_object,VAL_TIME, data_buffer[index].time);

        // Add the object to the JSON array
        cJSON_AddItemToArray(data_array, data_object);
    }

    ESP_LOGI(TAG, "Added %d entries to the JSON array.", count);
    return ESP_OK;
}