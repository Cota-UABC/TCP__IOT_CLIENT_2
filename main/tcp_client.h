#ifndef TCP_CLIENT
#define TCP_CLIENT

#include <sys/socket.h>
#include <arpa/inet.h>
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/semphr.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define STR_LEN 128
#define COMMANDS_MAX_QUANTITY 10

#define HOST_GOOGLE "142.250.188.14" //Google
#define PORT_GOOGLE 80

typedef struct 
{
    char host[STR_LEN];
    int port;
    char local_host[STR_LEN]
    int local_port;
} task_tcp_params_t;

void tcp_client_main(char *host, int port, char *local_host, int local_port);

void tcp_task(void *pvParameters);

esp_err_t tcp_init_connect(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port);

void tcp_communicate_loop(int *sock_ptr);

void transmit_receive(char *tx_buffer, char *rx_buffer, int *sock_ptr);

uint8_t login(char *tx_buffer, char *rx_buffer, int *sock_ptr);

void keep_alive(char *tx_buffer, char *rx_buffer, int *sock_ptr);

uint8_t check_ack(char *rx_buffer)

void build_command(char *string_com, ...);

#endif 