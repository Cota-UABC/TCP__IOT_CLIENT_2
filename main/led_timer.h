#ifndef LED_TIMER
#define LED_TIMER

#include <sys/socket.h>
#include <arpa/inet.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>

#include "nvs_esp.h"
#include "clock.h"
#include "gpio.h"


extern char *nvs_key_H, *nvs_key_N, *nvs_key_F;

void led_timer_task(void *pvParameters);

#endif 