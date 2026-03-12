#include "esp_time.h"
static const char *TAG = "esp_time";

void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Notification of a time synchronization event");
}

void initialize_sntp() {
    ESP_LOGI(TAG, "Initializing SNTP");

    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "time.google.com");
    esp_sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    esp_sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);
    esp_sntp_set_sync_interval(3600000); // Sync every hour
    esp_sntp_init();

    // Set timezone to UTC
    setenv("TZ", "EST5EDT,M3.2.0,M11.1.0", 1);
    tzset();
}

void wait_for_time_sync() {
    time_t now = 0;
    struct tm timeinfo = { 0 };
    int retry = 0;
    const int retry_count = 15;

    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count) {
        ESP_LOGI(TAG, "Waiting for system time to be set... (%d/%d)", retry, retry_count);
        vTaskDelay(pdMS_TO_TICKS(2000));
    }

    time(&now);
    localtime_r(&now, &timeinfo);

    if (timeinfo.tm_year < (2024 - 1900)) {
        ESP_LOGW(TAG, "Time is not set yet. Retry initialization.");
        esp_sntp_stop();
        initialize_sntp();
    } else {
        ESP_LOGI(TAG, "Time is synchronized: %s", asctime(&timeinfo));
    }
}

uint32_t get_time()
{
    time_t now;
    time(&now); // Get current time
    uint32_t time = (uint32_t)now;
    return time;
}