#ifndef TCP_CLIENT
#define TCP_CLIENT

#include <sys/socket.h>
#include <arpa/inet.h>
#include "esp_log.h"

#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <unistd.h>

#define STR_LEN 128
#define COMMANDS_MAX_QUANTITY 10

typedef struct 
{
    char host[STR_LEN];
    int port;
} task_tcp_params_t;

void tcp_client(char *host, int port);

void tcp_task(void *pvParameters);

uint8_t tcp_init_connect(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port);

void tcp_communicate(int *sock_ptr);

void build_command(char *string_com, ...);

#endif 