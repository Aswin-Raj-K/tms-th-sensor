#include "https_client.h"
#include "esp_time.h"

static const char *TAG = "https_client";
SemaphoreHandle_t xMutex = NULL;

// For alarm triggers
bool temp_high_alarm_triggered = false;
bool temp_low_alarm_triggered = false;
bool hum_high_alarm_triggered = false;
bool hum_low_alarm_triggered = false;

// Send JSON data over TLS
int send_json_data(esp_tls_t *tls, const char *json_data)
{
    size_t json_len = strlen(json_data);
    char *json_data_with_newline = malloc(json_len + 2);

    if (!json_data_with_newline) {
        ESP_LOGE(TAG, "Memory allocation failed");
        return -1;
    }

    strcpy(json_data_with_newline, json_data);
    json_data_with_newline[json_len] = '\n';
    json_data_with_newline[json_len + 1] = '\0';  // Null terminator

    int written_bytes = esp_tls_conn_write(tls, json_data_with_newline, strlen(json_data_with_newline));

    free(json_data_with_newline);

    if (written_bytes > 0) {
        ESP_LOGI(TAG, "Sent JSON data: %s", json_data);
    } else {
        ESP_LOGE(TAG, "Failed to send JSON data");
    }

    return written_bytes;
}

void send_data(esp_tls_t *tls){
    if(xSemaphoreTake(xMutex,portMAX_DELAY) == pdTRUE){
        cJSON *root = cJSON_CreateObject();
        char *json_data;
        cJSON *data_array = cJSON_CreateArray();

        cJSON_AddNumberToObject(root, KEY_ID, DEVICE_ID);
        cJSON_AddNumberToObject(root, KEY_DEVICE_ID, DEVICE_ID);
        cJSON_AddNumberToObject(root, KEY_REQUEST, REQUEST_DEVICE_DATA);

        
        esp_err_t res = ESP_OK;
        if(AVERAGE_DATA){
            float avg_temp,avg_hum;
            uint32_t time;
            res = get_average(&avg_temp, &avg_hum, &time);

            //Adding Data
            cJSON *device_data = cJSON_CreateObject();
            cJSON_AddNumberToObject(device_data,VAL_TEMP, avg_temp);
            cJSON_AddNumberToObject(device_data,VAL_HUM, avg_hum);
            cJSON_AddNumberToObject(device_data,VAL_TIME, time);
            cJSON_AddItemToArray(data_array,device_data);
        }
        else{
            res = get_device_data(data_array);
        }


        if (res == ESP_OK){
            
            cJSON_AddItemToObject(root,KEY_DATA,data_array);
            cJSON_AddNumberToObject(root, KEY_RESPONSE, RESPONSE_OK);
            reset_counter();

            //Adding info
            cJSON *info = cJSON_CreateObject();
            cJSON_AddBoolToObject(info, KEY_AVERAGE_DATA, is_data_average());
            cJSON_AddNumberToObject(info, KEY_HOLD_TIME, get_hold_time());
            cJSON_AddNumberToObject(info, KEY_DATA_BUFFER_SIZE, get_buffer_size());
            cJSON_AddItemToObject(root,KEY_INFO,info);

        }else{
            cJSON_AddNumberToObject(root, KEY_RESPONSE, RESPONSE_NO_DATA);
        }

        json_data = cJSON_PrintUnformatted(root);

        // Send JSON data to the server
        send_json_data(tls, json_data);

        // Clean up the JSON object and data
        cJSON_Delete(root);
        free(json_data);

        xSemaphoreGive(xMutex);
    }else{
        ESP_LOGE(TAG, "Failed to take mutex in send_data");
    }
    
}

void register_device(esp_tls_t *tls){
    ESP_LOGI(TAG, "Registering device");

    // Initial message to the server
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, KEY_ID, DEVICE_ID);
    cJSON_AddNumberToObject(root, KEY_DEVICE_ID, DEVICE_ID);
    cJSON_AddNumberToObject(root, KEY_REQUEST, REQUEST_DEVICE_ADD);

    cJSON *info = cJSON_CreateObject();
    cJSON_AddNumberToObject(info,KEY_DEVICE_ID,DEVICE_ID);
    cJSON_AddStringToObject(info,KEY_DESCRIPTION,DEVICE_DESCRIPTION);
    cJSON_AddStringToObject(info,KEY_DEVICE_NAME,DEVICE_NAME);
    cJSON_AddNumberToObject(info,KEY_DEVICE_TYPE,DEVICE_TYPE);
    cJSON_AddBoolToObject(info,KEY_AVERAGE_DATA,is_data_average());
    cJSON_AddNumberToObject(info, KEY_DATA_BUFFER_SIZE, get_buffer_size());
    cJSON_AddNumberToObject(info, KEY_HOLD_TIME, get_hold_time());

    cJSON *triggers = cJSON_CreateObject();
    int32_t trigger_val[4];
    get_triggers(trigger_val);

    cJSON_AddNumberToObject(triggers, KEY_TEMP_HIGH, trigger_val[0]);  
    cJSON_AddNumberToObject(triggers, KEY_TEMP_LOW, trigger_val[1]);   
    cJSON_AddNumberToObject(triggers, KEY_HUM_HIGH, trigger_val[2]);   
    cJSON_AddNumberToObject(triggers, KEY_HUM_LOW, trigger_val[3]);

    cJSON_AddItemToObject(root,KEY_INFO,info);
    cJSON_AddItemToObject(root,KEY_TRIGGERS,triggers);
    char *json_data = cJSON_PrintUnformatted(root);

    if (send_json_data(tls, json_data) <= 0) {
        ESP_LOGE(TAG, "Connection lost");
        cJSON_Delete(root);
        free(json_data);
        esp_tls_conn_destroy(tls);
        vTaskDelete(NULL);
        return;
    }

    // Clean up the JSON object and data
    cJSON_Delete(root);
    free(json_data);

    ESP_LOGI(TAG, "Registering device successfull");

}

void screen_init(){
    ssd1306_clear_screen(false);
    ssd1306_contrast(0xff);
    ssd1306_display_text(0, "St: Unknown", 10, false);
    ssd1306_display_text(1, "Temp: xx", 8, false);
    ssd1306_display_text(2, "Humidity: xx", 12, false);
}
void update_status(int status){
    ESP_LOGI(TAG, "Updating status: %d", status);
    ssd1306_clear_line(0,false);
    if(status == 0)
        ssd1306_display_text(0, "St: Standby", 11, false);
    else if(status == 1)
        ssd1306_display_text(0, "St: Connected", 13, false);
    else if(status == 2)
        ssd1306_display_text(0, "St: Wifi Connec", 15, false);
    else
        ssd1306_display_text(0, "St: Unknown", 10, false);
}
void update_screen(float temp, float hum){
    char temp_str[16];
    char hum_str[16];
    snprintf(temp_str, sizeof(temp_str), "Temp: %.1f F", temp);
    ssd1306_display_text(1, temp_str, strlen(temp_str), false);

    // Format humidity string
    snprintf(hum_str, sizeof(hum_str), "Humidity: %.1f %%", hum);
    ssd1306_display_text(2, hum_str, strlen(hum_str), false);
}

void get_data(float *temp, float *hum){
    sht3x_read_temp_humi(temp,hum);
    ESP_LOGI("SHT31", "Temperature: %.1f F, Humidity: %.1f %%", *temp, *hum);

}

void send_alarm_triggers(esp_tls_t *tls, cJSON *triggers_array){

    if (tls == NULL || triggers_array == NULL) {
        return;
    }

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, KEY_ID, DEVICE_ID);
    cJSON_AddNumberToObject(root, KEY_DEVICE_ID, DEVICE_ID);
    cJSON_AddNumberToObject(root, KEY_REQUEST, REQUEST_ALARM_TRIGGERED);
    
    cJSON *info = cJSON_CreateObject();
    cJSON_AddNumberToObject(info, KEY_DEVICE_ID, DEVICE_ID);
    cJSON_AddStringToObject(info, KEY_DEVICE_NAME, DEVICE_NAME);
    cJSON_AddNumberToObject(info, KEY_TIME, get_time());
    

    cJSON_AddItemToObject(root, KEY_TRIGGERS, triggers_array);
    cJSON_AddItemToObject(root, KEY_INFO, info);

    char *json_data = cJSON_PrintUnformatted(root);
    if (json_data != NULL) {
        send_json_data(tls, json_data);
        free(json_data);
    }

    cJSON_Delete(root);
}

void check_trigger(esp_tls_t *tls, float temp, float hum){

   if (tls == NULL) {
        return;
    }

    int32_t triggers[4];
    get_triggers(triggers);

    cJSON *triggers_array = cJSON_CreateArray();

    if (temp > triggers[0]) {
        if (!temp_high_alarm_triggered) {
            ESP_LOGW(TAG, "ALARM: Temperature exceeds high threshold. Current temperature: %.2f°C", temp);
            cJSON *trigger = cJSON_CreateObject();
            cJSON_AddStringToObject(trigger, KEY_TRIGGER_TYPE, KEY_TEMP_HIGH);
            cJSON_AddNumberToObject(trigger, KEY_TEMP_HIGH, triggers[0]);
            cJSON_AddNumberToObject(trigger, KEY_CURRENT, temp);
            cJSON_AddItemToArray(triggers_array, trigger);
            temp_high_alarm_triggered = true;
        }
    } else {
        temp_high_alarm_triggered = false;
    }

    if (temp < triggers[1]) {
        if (!temp_low_alarm_triggered) {
            ESP_LOGW(TAG, "ALARM: Temperature below low threshold. Current temperature: %.2f°C", temp);
            cJSON *trigger = cJSON_CreateObject();
            cJSON_AddStringToObject(trigger, KEY_TRIGGER_TYPE, KEY_TEMP_LOW);
            cJSON_AddNumberToObject(trigger, KEY_TEMP_LOW, triggers[1]);
            cJSON_AddNumberToObject(trigger, KEY_CURRENT, temp);
            cJSON_AddItemToArray(triggers_array, trigger);
            temp_low_alarm_triggered = true;
        }
    } else {
        temp_low_alarm_triggered = false;
    }

    if (hum > triggers[2]) {
        if (!hum_high_alarm_triggered) {
            ESP_LOGW(TAG, "ALARM: Humidity exceeds high threshold. Current humidity: %.2f%%", hum);
            cJSON *trigger = cJSON_CreateObject();
            cJSON_AddStringToObject(trigger, KEY_TRIGGER_TYPE, KEY_HUM_HIGH);
            cJSON_AddNumberToObject(trigger, KEY_HUM_HIGH, triggers[2]);
            cJSON_AddNumberToObject(trigger, KEY_CURRENT, hum);
            cJSON_AddItemToArray(triggers_array, trigger);
            hum_high_alarm_triggered = true;
        }
    } else {
        hum_high_alarm_triggered = false;
    }

    if (hum < triggers[3]) {
        if (!hum_low_alarm_triggered) {
            ESP_LOGW(TAG, "ALARM: Humidity below low threshold. Current humidity: %.2f%%", hum);
            cJSON *trigger = cJSON_CreateObject();
            cJSON_AddStringToObject(trigger, KEY_TRIGGER_TYPE, KEY_HUM_LOW);
            cJSON_AddNumberToObject(trigger, KEY_HUM_LOW, triggers[3]);
            cJSON_AddNumberToObject(trigger, KEY_CURRENT, hum);
            cJSON_AddItemToArray(triggers_array, trigger);
            hum_low_alarm_triggered = true;
        }
    } else {
        hum_low_alarm_triggered = false;
    }

    if (cJSON_GetArraySize(triggers_array) > 0) {
        send_alarm_triggers(tls, triggers_array);
    } else {
        cJSON_Delete(triggers_array);
    }

}

esp_err_t save_data(cJSON *data){
    if(data!=NULL && cJSON_IsObject(data)){
        // Saving buffer size
        
        int32_t val_buffer_size, val_sample_time;
        bool is_average;
        
        cJSON *buffer_size = cJSON_GetObjectItem(data,KEY_DATA_BUFFER_SIZE);
        if(buffer_size != NULL && cJSON_IsNumber(buffer_size))
            val_buffer_size = buffer_size->valueint;
        else
            return ESP_ERR_DATA_NOT_VALID;
        
        cJSON *sample_time = cJSON_GetObjectItem(data,KEY_HOLD_TIME);
        if(sample_time != NULL && cJSON_IsNumber(sample_time))
            val_sample_time = sample_time -> valueint;
        else
            return ESP_ERR_DATA_NOT_VALID;


        cJSON *average_data = cJSON_GetObjectItem(data,KEY_AVERAGE_DATA);
        if(average_data != NULL && cJSON_IsBool(average_data))
            is_average = cJSON_IsTrue(average_data);
        else
            return ESP_ERR_DATA_NOT_VALID;

        int32_t triggers[4];

        cJSON *temp_high = cJSON_GetObjectItem(data, KEY_TEMP_HIGH);
        if (temp_high != NULL && cJSON_IsNumber(temp_high)) {
            triggers[0] = temp_high->valueint;
        } else {
            return ESP_ERR_DATA_NOT_VALID;
        }

        cJSON *temp_low = cJSON_GetObjectItem(data, KEY_TEMP_LOW);
        if (temp_low != NULL && cJSON_IsNumber(temp_low)) {
            triggers[1] = temp_low->valueint;
        } else {
            return ESP_ERR_DATA_NOT_VALID;
        }

        cJSON *hum_high = cJSON_GetObjectItem(data, KEY_HUM_HIGH);
        if (hum_high != NULL && cJSON_IsNumber(hum_high)) {
            triggers[2] = hum_high->valueint;
        } else {
            return ESP_ERR_DATA_NOT_VALID;
        }

        cJSON *hum_low = cJSON_GetObjectItem(data, KEY_HUM_LOW);
        if (hum_low != NULL && cJSON_IsNumber(hum_low)) {
            triggers[3] = hum_low->valueint;
        } else {
            return ESP_ERR_DATA_NOT_VALID;
        }

        if(xSemaphoreTake(xMutex,portMAX_DELAY) == pdTRUE){
            set_buffer_size(val_buffer_size);
            set_hold_time(val_sample_time);
            set_data_average(is_average);
            set_triggers(triggers);
        }
        xSemaphoreGive(xMutex);

        return ESP_OK;
    }

    return ESP_ERR_DATA_NOT_VALID;

}

void https_listener_task(void *pvParameters)
{   
    ESP_LOGI(TAG, "HTTPS listener task started");
    esp_tls_t *tls = (esp_tls_t *)pvParameters;
    char recv_buf[1024];
    update_status(1);
    while (1) {
        int len = esp_tls_conn_read(tls, (char *)recv_buf, sizeof(recv_buf) - 1);

        if (len > 0) {
            recv_buf[len] = 0;  // Null-terminate the received data
            ESP_LOGI(TAG, "Received: %s", recv_buf);
            
            cJSON *root = cJSON_Parse(recv_buf);
            char *json_data = NULL;
            if (root == NULL) {
                ESP_LOGE(TAG, "Error parsing JSON: %s", recv_buf);
            } else {
                // Extract the 'request' key from the JSON object
                cJSON *request = cJSON_GetObjectItem(root, KEY_REQUEST);
                if (request != NULL && cJSON_IsNumber(request)) {
                    ESP_LOGI(TAG, "Extracted DATA: %d", request->valueint);
                    switch (request->valueint)
                    {
                        case REQUEST_DEVICE_DATA:
                                send_data(tls);
                                break;
                        case REQUEST_SAVE:
                            
                                cJSON *data = cJSON_GetObjectItem(root,KEY_DATA);
                                esp_err_t res = save_data(data);

                                if(res == ESP_OK){
                                    ESP_LOGI(TAG, "Data updated successfully");
                                    log_storage();
                                    reset_counter();
                                    cJSON_AddNumberToObject(root,KEY_RESPONSE,RESPONSE_OK);
                                }
                                else{
                                    ESP_LOGE(TAG, "Data updation failed, invalid data");
                                    cJSON_AddNumberToObject(root,KEY_RESPONSE,RESPONSE_WRITE_FAILED);
                                }
    
                                cJSON_AddNumberToObject(root, KEY_ID, DEVICE_ID);
                                json_data = cJSON_PrintUnformatted(root);
                                if(json_data!=NULL){
                                    send_json_data(tls,json_data);
                                    free(json_data);
                                }
                                break;

                        default:
                            ESP_LOGE(TAG, "Unknown request type: %d", request->valueint);
                            cJSON_AddNumberToObject(root, KEY_RESPONSE, RESPONSE_INVALID_REQUEST);
                            cJSON_AddNumberToObject(root, KEY_ID, DEVICE_ID);
                            json_data = cJSON_PrintUnformatted(root);
                            if(json_data!=NULL){
                                send_json_data(tls,json_data);
                                free(json_data);
                            }
                            break;
                    }
                    
                } else {
                    ESP_LOGE(TAG, "'REQUEST' key not found or invalid format");
                }
                
            }
            cJSON_Delete(root);

        } else if (len == 0) {
            // No data received, the connection is still alive
            ESP_LOGD(TAG, "No data received, continuing to listen...");
            
        } else {
            if (len == ESP_TLS_ERR_SSL_WANT_READ || len == ESP_TLS_ERR_SSL_WANT_WRITE) {
                // These are not critical errors, just waiting for more data, suppress log
                ESP_LOGD(TAG, "TLS connection waiting for more data (non-critical), continuing...");
            } 
            else {
                // Log other errors and continue listening
                ESP_LOGE(TAG, "esp_tls_conn_read error: %d", len);
                ESP_LOGE(TAG, "Server closed connection. Restarting...");
                esp_restart();
            }
        }

        // Delay before checking for more incoming data, can be adjusted or removed as needed
        vTaskDelay(pdMS_TO_TICKS(100));  // 100 ms delay to avoid tight looping
    }

    // This point should not be reached since we are always listening.
    ESP_LOGI(TAG, "HTTPS listener task terminating");
    vTaskDelete(NULL);
}


void https_data_task(void *pvParameters)
{   

    esp_tls_t *tls = NULL;
    tls = (esp_tls_t *)pvParameters;

    vTaskDelay(pdMS_TO_TICKS(1 * 1000));//Delay before reading
    ESP_LOGI(TAG, "Data task started");
    
    int32_t hold_time = get_hold_time();
    ESP_LOGI(TAG, "Hold time set to: %" PRId32, hold_time);

    int i = 1;
    ESP_LOGI(TAG, "Starting Read");

    TickType_t last_sample_time = xTaskGetTickCount();
    TickType_t sample_interval = pdMS_TO_TICKS(hold_time * 60 * 1000); // Convert minutes to ticks

    while (1) {
        TickType_t current_time = xTaskGetTickCount();

        if (!DEBUG){

            if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE) {
                float temp,hum;
                get_data(&temp,&hum);

                temp = roundf(temp * 100) / 100;
                hum = roundf(hum * 100) / 100;  

                if ((current_time - last_sample_time) >= sample_interval) {
                    add_sample(temp, hum);
                    last_sample_time = current_time;
                }

                
                if(OLED_DISP){
                    update_screen(temp,hum);
                    vTaskDelay(pdMS_TO_TICKS(20));
                }
                xSemaphoreGive(xMutex);
            
                check_trigger(tls, temp, hum);

            }else{
                ESP_LOGE(TAG, "Failed to take mutex");
            }
        }else{
            i = i + 1;
        }
        vTaskDelay(pdMS_TO_TICKS(SCREEN_UPDATE_TIME * 1000));
    }
}



void https_client_init(void *pvParameters)
{   
    xMutex = xSemaphoreCreateMutex();
    
    esp_tls_t *tls = NULL;
    tls = esp_tls_init();

    TaskHandle_t readTaskHandle = NULL;
    BaseType_t xReturned;
    xReturned = xTaskCreate(&https_data_task, "https_data_task", 8192, tls, 5, &readTaskHandle);
    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "Data task created successfully");
    } else {
        ESP_LOGE(TAG, "Failed to create data task");
        return;
    }

    int try = 1;
    while(1){
        ESP_LOGI(TAG, "Standby Mode, Try: %d", try);
        if (get_state() == STATE_RESTART) {
            update_status(0);
            ESP_LOGI(TAG, "Device in RESTART state. Waiting for RESTART_DELAY minutes before continuing");
            vTaskDelay(pdMS_TO_TICKS(RESTART_DELAY_MIN * 60 * 1000));
            // After delay, set state back to REBOOT
            set_state(STATE_REBOOT);
        }

        esp_tls_cfg_t cfg = {
            .cacert_buf = (const unsigned char *)CERT,
            .cacert_bytes = strlen(CERT)+1,
            .timeout_ms = 20000
        };

        
        ESP_LOGI(TAG, "Connecting to server %s:%d",SERVER_HOST,SERVER_PORT);

        if (esp_tls_conn_new_sync(SERVER_HOST, strlen(SERVER_HOST), SERVER_PORT, &cfg, tls) != 1) {
            ESP_LOGE(TAG, "Failed to connect to server");
            // esp_tls_conn_destroy(tls);

            if (get_state() != STATE_RESTART)
                set_state(STATE_RESTART);

            // esp_restart();

            esp_tls_conn_destroy(tls);
            try+=1;
            continue;
            // return;
        }

        ESP_LOGI(TAG, "Connected to server");
        break;

    }


    // Register Device
    register_device(tls);
    

    https_listener_task(tls);
    update_status(0);
    if(readTaskHandle!=NULL){ // Delete Read Task
        vTaskDelete(readTaskHandle);
        readTaskHandle = NULL;
    }
    https_client_destroy(tls);
}

void https_client_start(void){
    
    BaseType_t xReturned;
    xReturned = xTaskCreate(&https_client_init, "https_client_init", 16384, NULL, 5, NULL);

    if (xReturned == pdPASS) {
        ESP_LOGI(TAG, "Client init successfull");
    } else {
        ESP_LOGE(TAG, "Failed to init the client");
        return;
    }
}

void https_client_destroy(esp_tls_t *tls){
    // Cleanup after the loop
    esp_tls_conn_destroy(tls);
    vSemaphoreDelete(xMutex);
    if(!DEBUG){
        if(OLED_DISP)
            ssd1306_i2c_delete_dev();
        sht3x_delete_dev();
        i2c_delete_handle();
    }
    vTaskDelete(NULL);
}

