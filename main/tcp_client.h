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

#define STR_LEN 128
#define COMMANDS_MAX_QUANTITY 10

#define CONNECT_MAX_RETRY 3

//server exit flags
#define FAIL 0
#define INTERNET_CONNECTED 1
#define INTERNET_DISCONNECTED 2
#define MAX_RETRY 3

#define FALSE 0
#define TRUE 1
#define UNDEFINED 2


#define HOST_GOOGLE "142.250.188.14" //Google
#define PORT_GOOGLE 80

#define INTERNET_CHECK_MS_WAIT 10000
#define KEEP_ALIVE_MS_WAIT 10000

typedef struct 
{
    char host[STR_LEN];
    int port;
    char local_host[STR_LEN]
    int local_port;
    SemaphoreHandle_t xSemaphore_internet;
} task_tcp_params_t;

void tcp_client_main(char *host, int port, char *local_host, int local_port);

void tcp_task(void *pvParameters);

uint8_t tcp_server_connect(uint8_t return_on_internet_connection, SemaphoreHandle_t xSemaphore_internet);

esp_err_t tcp_init_connect(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, char *host, int port);

esp_err_t tcp_connect_to_host(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port)

void tcp_communicate_loop(int *sock_ptr);

void transmit_receive(char *tx_buffer, char *rx_buffer, int *sock_ptr);

esp_err_t login(char *tx_buffer, char *rx_buffer, int *sock_ptr);

void send_keep_alive(char *tx_buffer, char *rx_buffer, int *sock_ptr);

void keep_alive_task(void *pvParameters);

uint8_t check_ack(char *rx_buffer);

void build_command(char *string_com, ...);

void check_internet_task(void *pvParameter);

uint8_t check_internet_mutex(SemaphoreHandle_t xSemaphore_internet, uint16_t ms_to_wait);

#endif 