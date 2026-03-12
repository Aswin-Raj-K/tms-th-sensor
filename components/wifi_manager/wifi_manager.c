
#include "wifi_manager.h"

// Callback function pointer
static void (*wifi_connected_callback)(void) = NULL;
static const char *TAG = "wifi_manager";


// Event handler for Wi-Fi and IP events
void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        ESP_LOGI(TAG, "Wi-Fi disconnected, trying to reconnect");
        esp_wifi_connect();
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *) event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        if(wifi_connected_callback !=NULL)
            wifi_connected_callback();
    }
}


void wifi_init(void (*callback)(void))
{   
    ESP_LOGI(TAG, "WiFi Starting");
    
    wifi_connected_callback = callback;

    // Create default Wi-Fi STA
    esp_netif_create_default_wifi_sta();

    // Register Wi-Fi and IP event handlers
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    if (TYPE) {
        // WPA2 Personal Configuration
        wifi_config_t wifi_config = {
            .sta = {
                .ssid = WIFI_SSID,
                .password = WIFI_PASS,
            },
        };

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
        ESP_ERROR_CHECK(esp_wifi_start());

        ESP_LOGI(TAG, "Wi-Fi connecting with WPA2 Personal");

    } else {
        // WPA2 Enterprise Configuration using esp_eap_client.h
        wifi_config_t wifi_config = {
            .sta = {
                .ssid = WIFI_SSID,
                .threshold.authmode = WIFI_AUTH_WPA2_ENTERPRISE,
            }
        };

        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));

        ESP_ERROR_CHECK(esp_eap_client_set_ttls_phase2_method(ESP_EAP_TTLS_PHASE2_MSCHAPV2));

        // Configure EAP client
        ESP_ERROR_CHECK(esp_eap_client_set_identity((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
        ESP_ERROR_CHECK(esp_eap_client_set_username((uint8_t *)WIFI_USERNAME, strlen(WIFI_USERNAME)));
        ESP_ERROR_CHECK(esp_eap_client_set_password((uint8_t *)WIFI_PASS, strlen(WIFI_PASS)));


        // Enable WPA2 Enterprise mode
        ESP_ERROR_CHECK(esp_wifi_sta_enterprise_enable());

        // Start Wi-Fi
        ESP_ERROR_CHECK(esp_wifi_start());
        ESP_LOGI(TAG, "Wi-Fi connecting with WPA2 Enterprise");
    }
}