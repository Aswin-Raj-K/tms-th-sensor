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
MIIEEDCCAvigAwIBAgIUQCBQ6V0fPvmecutk3Y+6dIUVzGMwDQYJKoZIhvcNAQEL\n\
BQAwgYsxCzAJBgNVBAYTAlVTMREwDwYDVQQIDAhOZXcgWW9yazERMA8GA1UEBwwI\n\
QnJvb2tseW4xDDAKBgNVBAoMA05ZVTEQMA4GA1UECwwHTmFub0ZhYjEXMBUGA1UE\n\
AwwOMTI4LjIzOC4zNS4xMTMxHTAbBgkqhkiG9w0BCQEWDmFyNzk5N0BueXUuZWR1\n\
MB4XDTI2MDMwMTE5NDg1NVoXDTI4MDIyOTE5NDg1NVowgYsxCzAJBgNVBAYTAlVT\n\
MREwDwYDVQQIDAhOZXcgWW9yazERMA8GA1UEBwwIQnJvb2tseW4xDDAKBgNVBAoM\n\
A05ZVTEQMA4GA1UECwwHTmFub0ZhYjEXMBUGA1UEAwwOMTI4LjIzOC4zNS4xMTMx\n\
HTAbBgkqhkiG9w0BCQEWDmFyNzk5N0BueXUuZWR1MIIBIjANBgkqhkiG9w0BAQEF\n\
AAOCAQ8AMIIBCgKCAQEAsPrliXykPWbGXHOV6uv0D55uGBNVVT4orOKSF93SFtzV\n\
B4EnLc5S1Z2zWT/bdirBbrhoz0ZBANYyGELteXjqW5AXo0XRcfARB92GBrcMJomd\n\
7lCffYagTgjiJrJbLL9nejzXmQJf2TkwiRE07aNQ58dwacS//Ltd3L8xqymGPtnF\n\
NSMunayT3zYAq+0yN9gYFaaLWHfneKAYTeSo933Lh2SKsk30nHEZrg7Nzsascb60\n\
n9SKGaUSICLHGn6kY9kDEuSxVQKi3WF9mEs5gytNcaTBCWY0gFieM1HRN3qWh3QP\n\
eFif9KMuXLsOH3k4WvfD2gvbpEPRetwrlPxS1+iGIQIDAQABo2owaDAdBgNVHQ4E\n\
FgQU/YHZ9OmEz9n+G1v4egqUvsnSW3IwHwYDVR0jBBgwFoAU/YHZ9OmEz9n+G1v4\n\
egqUvsnSW3IwDwYDVR0TAQH/BAUwAwEB/zAVBgNVHREEDjAMhwSA7iNxhwTAqABn\n\
MA0GCSqGSIb3DQEBCwUAA4IBAQBMqU88TpDnu9O4Cvw4VVbC08EYdt3QoT1vj3Ez\n\
jltXaV5HP7n+BwVJRFR2mIGfDTO/MUdeWrjsy8qfx10sWcnvo6YXzw0lI/nOVu7M\n\
2g5HHzxSwP0ODIHlQvUb4R4dtMRj9KeGTg3wVTh9of28DwkMLZ3D9AWfByd2mu19\n\
+NwikobaAFX3ZPSbuH1qSgFgatlAKA1eoc7D9WWb/ju5XpecBEyTN+HwzgWMIOux\n\
1EkqcEsPzwsbLX4LSsXGAWzuPA+/CQqZhxdrBZvTyIJ4vq+b7l2q1LQiszcnA6Dl\n\
TvVrQ+Y9yc23ArXEwdUhTai+wqIiQltvCtv4xGahHsARaWgC\n\
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
