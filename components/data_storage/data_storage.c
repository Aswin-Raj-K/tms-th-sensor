#include "data_storage.h"

static const char *TAG = "data_storage";

Storage storage = {DEFAULT_HOLD_TIME,DATA_BUFFER_SIZE,STATE_UNKNOWN,AVERAGE_DATA, TEMP_HIGH, TEMP_LOW, HUM_HIGH, HUM_LOW};

void read_int(char *key, int32_t *data, int32_t default_data){
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
        *data = default_data;
    } else {
        err = nvs_get_i32(my_handle, key, data);
        if (err == ESP_ERR_NVS_NOT_FOUND) {
            ESP_LOGI(TAG, "Key %s not found, using default", key);
            *data = default_data;
        } else if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) reading data for key %s", esp_err_to_name(err), key);
            *data = default_data;
        }
        nvs_close(my_handle);
    }
}


void write_int(char *key, int32_t data) {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    } else {
        err = nvs_set_i32(my_handle, key, data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Error (%s) writing data for key %s", esp_err_to_name(err), key);
        } else {
            err = nvs_commit(my_handle);
            if (err != ESP_OK) {
                ESP_LOGE(TAG, "Error (%s) committing data for key %s", esp_err_to_name(err), key);
            } else {
                ESP_LOGI(TAG, "Data written successfully for key %s", key);
            }
        }
        nvs_close(my_handle);
    }
}

int32_t get_state(){
    return storage.state;
}

void set_state(int32_t state){
    write_int(ST_STATE,state);
    storage.state = state;
}

int32_t get_hold_time(){
    return storage.hold_time;
}

void set_hold_time(int32_t hold_time){
    write_int(ST_HOLD_TIME,hold_time);
    storage.hold_time = hold_time;
}


bool is_data_average(){
    return storage.average_data;
}

void set_data_average(bool is_average){
    write_int(ST_AVERAGE_DATA, (int32_t)is_average);
    storage.average_data = is_average;
}

int32_t get_buffer_size(){
    return storage.data_buffer_size;

}

void set_buffer_size(int32_t buffer_size){
    write_int(ST_DATA_BUFFER_SIZE,buffer_size);
    storage.data_buffer_size = buffer_size;
}

void log_storage(){
    ESP_LOGI(TAG, "Storage Data:");
    ESP_LOGI(TAG, "  Hold Time: %" PRId32 " minutes", storage.hold_time);
    ESP_LOGI(TAG, "  Buffer Size: %" PRId32, storage.data_buffer_size);
    ESP_LOGI(TAG, "  State: %" PRId32, storage.state);
    ESP_LOGI(TAG, "  Average Data: %s", storage.average_data ? "true" : "false");

    ESP_LOGI(TAG, "  Temperature High: %" PRId32, storage.temp_high);
    ESP_LOGI(TAG, "  Temperature Low: %" PRId32, storage.temp_low);
    ESP_LOGI(TAG, "  Humidity High: %" PRId32, storage.hum_high);
    ESP_LOGI(TAG, "  Humidity Low: %" PRId32, storage.hum_low);
}


void init_storage(){

    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
        ESP_LOGE(TAG, "Error (%s) opening NVS handle!", esp_err_to_name(err));
    else {
        int32_t data;
        
        err = read_data(&my_handle, &data, ST_HOLD_TIME);
        if(err == ESP_OK)
            storage.hold_time = data;

        err = read_data(&my_handle, &data, ST_DATA_BUFFER_SIZE);
        if(err == ESP_OK)
            storage.data_buffer_size = data;
        
        err = read_data(&my_handle, &data, ST_AVERAGE_DATA);
        if(err == ESP_OK)
            storage.average_data = (data==1);
        
        err = read_data(&my_handle, &data, ST_STATE);
        if(err == ESP_OK)
            storage.state = data;

        err = read_data(&my_handle, &data, ST_TEMP_HIGH);
        if(err == ESP_OK)
            storage.temp_high = data;
        
        err = read_data(&my_handle, &data, ST_TEMP_LOW);
        if (err == ESP_OK) {
            storage.temp_low = data;
        }
        
        err = read_data(&my_handle, &data, ST_HUM_HIGH);
        if (err == ESP_OK) {
            storage.hum_high = data;
        }

        err = read_data(&my_handle, &data, ST_HUM_LOW);
        if (err == ESP_OK) {
            storage.hum_low = data;
        }

        nvs_close(my_handle);
    }

   log_storage();
}

esp_err_t read_data(nvs_handle *handle, int32_t *data, char *key){
    esp_err_t err;
    err = nvs_get_i32(*handle, key, data);
    if (err == ESP_ERR_NVS_NOT_FOUND) {
        ESP_LOGI(TAG, "Key %s not found, using default", key);
        return ESP_FAIL;
    }else if(err != ESP_OK){
        ESP_LOGE(TAG, "Error (%s) reading data for key %s", esp_err_to_name(err), key);
        return ESP_FAIL;
    }

    return ESP_OK;
}

void clear_storage() {
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) opening NVS handle for clearing!", esp_err_to_name(err));
        return;
    }

    // Erase all keys in the "storage" namespace
    err = nvs_erase_all(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) erasing all keys in storage!", esp_err_to_name(err));
    } else {
        ESP_LOGI(TAG, "All keys in storage erased successfully!");
    }

    // Commit changes
    err = nvs_commit(my_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Error (%s) committing after erase!", esp_err_to_name(err));
    }

    nvs_close(my_handle);

    // Reset the in-memory `storage` structure to defaults
    storage.hold_time = DEFAULT_HOLD_TIME;
    storage.data_buffer_size = DATA_BUFFER_SIZE;
    storage.state = STATE_UNKNOWN;
    storage.average_data = AVERAGE_DATA;
    storage.temp_high = TEMP_HIGH;
    storage.temp_low = TEMP_LOW;
    storage.hum_high = HUM_HIGH;
    storage.hum_low = HUM_LOW;

    ESP_LOGI(TAG, "In-memory storage reset to defaults.");
}

void get_triggers(int32_t * triggers) {    
    triggers[0] = storage.temp_high;  // temp_high
    triggers[1] = storage.temp_low;   // temp_low
    triggers[2] = storage.hum_high;   // hum_high
    triggers[3] = storage.hum_low;    // hum_low
}

void set_triggers(int32_t *triggers) {
    write_int(ST_TEMP_HIGH, triggers[0]);
    storage.temp_high = triggers[0];

    write_int(ST_TEMP_LOW, triggers[1]);
    storage.temp_low = triggers[1];

    write_int(ST_HUM_HIGH, triggers[2]);
    storage.hum_high = triggers[2];

    write_int(ST_HUM_LOW, triggers[3]);
    storage.hum_low = triggers[3];
}
