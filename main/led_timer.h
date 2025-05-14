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

#include "real_time.h"
#include "gpio.h"

void clock_task(void *pvParameters);

#endif 