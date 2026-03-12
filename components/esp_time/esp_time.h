#ifndef TIME_H
#define TIME_H

#include <time.h>
#include "esp_sntp.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

void initialize_sntp();
void wait_for_time_sync();
uint32_t get_time();

#endif