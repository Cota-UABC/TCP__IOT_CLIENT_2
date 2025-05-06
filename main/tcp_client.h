#ifndef TCP_CLIENT
#define TCP_CLIENT

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

#include "gpio.h"
#include "adc.h"

#define STR_LEN 128
#define COMMANDS_MAX_QUANTITY 10

//#define CONNECT_MAX_RETRY 2
#define MAX_ERROR_COUNT 2

//server exit flags
#define UNDEFINED 0
#define COMMUNICATION_FAIL 1
#define STOP_SEMAPHORE 2

#define TRUE 1
#define FALSE 2

#define HOST_GOOGLE "142.250.188.14" //Google
#define PORT_GOOGLE 80

#define REMOTE_MS_WAIT 4000
#define SEMAPHORE_MS_WAIT 20
#define KEEP_ALIVE_MS_WAIT 10000

typedef struct 
{
    char host[STR_LEN];
    int port;
    char local_host[STR_LEN];
    int local_port;
    SemaphoreHandle_t activate_semaphore;
    SemaphoreHandle_t stop_semaphore;
} task_tcp_params_t;

void tcp_client_main(char *host, int port, char *local_host, int local_port);

void remote_server_task(void *pvParameters);

void local_server_task(void *pvParameters);

//uint8_t tcp_server_connect(char *host, int port);

void tcp_create_socket(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, char *host, int port);

esp_err_t tcp_connect_to_host(const char *LOCAL_FUNCTION_TAG, struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port);

uint8_t tcp_communicate_loop(const char *LOCAL_FUNCTION_TAG, int *sock_ptr, SemaphoreHandle_t stop_semaphore);

esp_err_t transmit_receive(const char *LOCAL_FUNCTION_TAG, char *tx_buffer, char *rx_buffer, int *sock_ptr);

esp_err_t login(const char *LOCAL_FUNCTION_TAG, char *tx_buffer, char *rx_buffer, int *sock_ptr);

esp_err_t send_keep_alive(const char *LOCAL_FUNCTION_TAG, char *tx_buffer, char *rx_buffer, int *sock_ptr);

void keep_alive_task(void *pvParameters);

esp_err_t check_ack(const char *LOCAL_FUNCTION_TAG, char *rx_buffer);

void build_command(char *string_com, ...);

esp_err_t check_internet_connection();

#endif 