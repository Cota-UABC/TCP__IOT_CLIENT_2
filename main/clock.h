#ifndef CLOCK
#define CLOCK

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

#define REAL_TIME_PORT "80"
#define REAL_TIME_IP "213.188.196.246"

extern SemaphoreHandle_t seconds_mutex;
extern uint32_t clock_seconds;

uint32_t get_real_time();

void start_clock();

void clock_task(void *pvParameters);

#endif 