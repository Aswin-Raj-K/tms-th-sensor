#include <stdio.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include "esp_log.h"

//External Components
#include "wifi_manager.h"
#include "https_client.h"
#include "data_storage.h"
#include "device.h"
#include "esp_time.h"

static const char *TAG = "main";

void wifi_connected(void){
    ESP_LOGI(TAG, "Starting https client");
    update_status(2);
    
    initialize_sntp();
    wait_for_time_sync();

    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);

    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S %Z", &timeinfo);
    ESP_LOGI(TAG, "Current time in EST: %s", strftime_buf);

    https_client_start();
}

void app_main(void)
{
    // Initialize NVS (Non-Volatile Storage)
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    ESP_ERROR_CHECK(esp_netif_init());

    // Creating default loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    

    if(!DEBUG){
        ESP_LOGI(TAG, "Setting up the devices");
        i2c_init();
        
        if(OLED_DISP){
            ssd1306_i2c_init();
            screen_init();
        }

        sht3x_init();
    }
    
    ESP_LOGI(TAG, "Setting up the devices");
    init_storage();
    // clear_storage();

    // Initialize Wi-Fi and connect
    wifi_init(wifi_connected);
    

}
