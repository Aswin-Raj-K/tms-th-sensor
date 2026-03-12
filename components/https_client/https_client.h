#ifndef HTTPS_CLIENT_H
#define HTTPS_CLIENT_H

#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <inttypes.h>
#include "string.h"
#include "esp_log.h"
#include "esp_tls.h" 
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "cJSON.h"
#include "esp_err.h"
#include "freertos/semphr.h"


#include "device.h"
#include "data_storage.h"
#include "sht3x.h"
#include "ssd1306.h"
#include "i2c.h"

void https_listener_task(void *pvParameters);
void https_data_task();
void https_client_init(void *pvParameters);

void send_data(esp_tls_t *tls);
int send_json_data(esp_tls_t *tls, const char *json_data);

void check_trigger(esp_tls_t *tls, float temp, float hum);
void send_alarm_triggers(esp_tls_t *tls, cJSON *triggers_array);

void register_device(esp_tls_t *tls);
esp_err_t save_data(cJSON *data);

void get_data(float *temp, float *hum);
void update_status(int status);
void update_screen(float temp, float hum);
void screen_init();

void https_client_start(void);
void https_client_destroy(esp_tls_t *tls);


// Server details
#define SERVER_HOST "192.168.0.103"
#define SERVER_PORT 5000


#define CERT "-----BEGIN CERTIFICATE-----\n\
MIID3jCCAsagAwIBAgIUa8ULq1H9Sqn/HBXzDb0c8M6s/1QwDQYJKoZIhvcNAQEL\n\
BQAwgYsxCzAJBgNVBAYTAlVTMREwDwYDVQQIDAhOZXcgWW9yazERMA8GA1UEBwwI\n\
QnJvb2tseW4xDDAKBgNVBAoMA05ZVTEQMA4GA1UECwwHTmFub0ZhYjEXMBUGA1UE\n\
AwwOMTI4LjIzOC4zNS4xMTMxHTAbBgkqhkiG9w0BCQEWDmFyNzk5N0BueXUuZWR1\n\
MB4XDTI0MTIxOTIwMDI0MFoXDTI1MTIxOTIwMDI0MFowgYsxCzAJBgNVBAYTAlVT\n\
MREwDwYDVQQIDAhOZXcgWW9yazERMA8GA1UEBwwIQnJvb2tseW4xDDAKBgNVBAoM\n\
A05ZVTEQMA4GA1UECwwHTmFub0ZhYjEXMBUGA1UEAwwOMTI4LjIzOC4zNS4xMTMx\n\
HTAbBgkqhkiG9w0BCQEWDmFyNzk5N0BueXUuZWR1MIIBIjANBgkqhkiG9w0BAQEF\n\
AAOCAQ8AMIIBCgKCAQEAqqRkhxk3PR76eKhUWOdvE7Z8Cj7nYaJEaAe3NtS2O+Iq\n\
GWpTDiva0QwwZBnWDXajgXJmsjJ25hVZJouwo5kapxaynxQZYyfpcjewCD3/U0ak\n\
0rYtBwo6ytgA7cyGjkE7Vhk3pndCb7Q6hOHMAdsFSRAOpj1uKG8pkc2o54Z/cFq5\n\
PvUqHD06Pi+bjePdcf+QMsuJazTd392HVggX+Ujagnm3kUkk2OJZ1tz2hlLMBnkq\n\
upsd085+GZK5RIzz8kqpQ6Qr4LBnLUL4AUTqyVjBo9Gp0TKi3CeF4iyevVNHsH9y\n\
MDX8YjOqxXUOf5jnnxMi/Cub88hynQBeHsmE9BAxCwIDAQABozgwNjAVBgNVHREE\n\
DjAMhwSA7iNxhwTAqABnMB0GA1UdDgQWBBT6i6Un0WRs6oArQHm9DiMxY/583jAN\n\
BgkqhkiG9w0BAQsFAAOCAQEAWok4c3/W/WaGd0HyHHbkpi8ik+rmEFLYah94cQct\n\
uxOa4+RCMrSUt+YMCpawStDqPaUR1CbXbTJpruvdeyQeDTu+FXGbETtnIKLTWLBQ\n\
+aj/mT6eVeQ6hmn18jgJIvTKn5C3aXc8XNlxzZ9oEXxVkOHGe4+VSimADf8TY2IW\n\
ZT1Vj5P1QtTYjMXrwowft2vDxjSD5GdWi70iCHBqaZTrGzUZhuCgfo7zfLA6qOHT\n\
swWwrwptRTuU94cucyEU+X92recccGJ3yhLLbqoVPJxhYwhbK6GsPC5Fjn2TTUBE\n\
/Uufc9FDaNQp2vXn4FJIcScDDrRbveZ++jKZYMB7i4ByUA==\n\
-----END CERTIFICATE-----\n"


//Requests
#define REQUEST_CLIENT_ADD 1
#define REQUEST_DEVICE_ADD 2
#define REQUEST_DATA 3
#define REQUEST_DEVICE_STATUS 4
#define REQUEST_DEVICE_DATA 5
#define REQUEST_ALARM_TRIGGERED  6
#define REQUEST_SAVE 7

//JSON KEYS
#define KEY_REQUEST_ID "request_id"
#define KEY_REQUEST "request"
#define KEY_ID "id"
#define KEY_DEVICE_ID "device_id"
#define KEY_DATA "data"
#define KEY_INFO "info"
#define KEY_RESPONSE "response"
#define KEY_TRIGGERS "triggers"


#define RESPONSE_OK 1
#define RESPONSE_WRITE_FAILED 3
#define RESPONSE_NO_DATA 4
#define RESPONSE_INVALID_REQUEST 8


#endif // HTTPS_CLIENT_H
