#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "string.h"
#include "esp_eap_client.h"  // New API for WPA2 Enterprise
#include "esp_err.h"


// Change TYPE to 1 for WPA2 Personal, and 0 for WPA2 Enterprise
#define TYPE 1

// #define WIFI_SSID "nyu-legacy"
// #define WIFI_PASS "yqbk!?Bz7Q&T5w#"  
#define WIFI_USERNAME "ar7997"  // For WPA2 Enterprise, this is your username

// #define WIFI_SSID "MyOptimum 6e921f"
// #define WIFI_PASS "pink-843-219"  // WPA2 Enterprise typically doesn't require this to be filled


#define WIFI_SSID "TMS_Server"
#define WIFI_PASS "94180114"  // WPA2 Enterprise typically doesn't require this to be filled


void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
void wifi_init(void (*callback)(void));

#endif // WIFI_MANAGER_H
