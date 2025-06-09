#ifndef CLOCK
#define CLOCK

#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <sys/time.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"
#include "freertos/queue.h"

#include "lwip/inet.h"
#include "lwip/netdb.h"
#include "lwip/sockets.h"
#include "lwip/ip_addr.h"

#include "esp_netif.h"
#include "esp_event.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define NTP_HOST "132.248.30.3"
#define NTP_PORT 123

#define CLOCK_WAIT_TIME_MS 5000

#define SECOND_TO_PRINT 30

extern SemaphoreHandle_t seconds_mutex;
extern uint32_t clock_seconds;

uint32_t get_real_time();

void start_clock();

void clock_task(void *pvParameters);

#endif 