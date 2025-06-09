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
#include <inttypes.h>

#include "gpio.h"
#include "adc.h"
#include "nvs_esp.h"
#include "clock.h"
#include "led_timer.h"
#include "string_to_uint8.h"
#include "pwm.h"

#define STR_LEN 128

#define TERMINATION_DELIMITER_CHR '\r'
#define TERMINATION_DELIMITER_STR "\r"

#define COMMANDS_MAX_QUANTITY 10

//max errors
#define MAX_ERROR_TCP_LOOP 3
#define MAX_RETRY_RECV 4

//exit flags
#define UNDEFINED 0
#define COMMUNICATION_OK 1
#define COMMUNICATION_FAIL 2
#define CONNECTION_CLOSED 3
#define CONNECTION_TIMEOUT 4
#define STOP_SEMAPHORE 5

#define TRUE 1
#define FALSE 2

//Google host
#define HOST_GOOGLE "142.250.188.14" 
#define PORT_GOOGLE 80

//client wait time
#define CONNECT_WAIT_S 5
#define SOCKET_TIMEOUT_SEC 1
#define REMOTE_MS_WAIT 4000
#define LOCAL_MS_WAIT 1000
#define SEMAPHORE_MS_WAIT 20

//restart
#define RESET_TIME_S 10

//keep alive
#define KEEP_ALIVE_MS 10000


//command parts
#define ID_C 0
#define USER_C 1
#define OPERATION_C 2
#define RESOURCE_C 3
#define VALUE_C 4

#define ID_TCP "UABC"
#define USER_TCP "a1264598"

//operations
#define WRITE_O "W"
#define READ_O "R"

//resources
#define LED_R "L"
#define ADC_R "A"
#define PWM_R "P"
#define HABILITAR_R "H"
#define ENCENDER_R "N"
#define APAGAR_R "F"
#define RESET_R "I"
#define CANCEL_RESET_R "U"

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


void tcp_create_socket(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port);

esp_err_t tcp_connect_to_host(const char *LOCAL_FUNCTION_TAG, struct sockaddr_in *dest_addr_ptr, int sock, char *host, int port);

uint8_t tcp_communicate_loop(const char *LOCAL_FUNCTION_TAG, int sock, SemaphoreHandle_t stop_semaphore);

uint8_t receive_data(const char *LOCAL_FUNCTION_TAG, int sock, char *rx_buffer, size_t buffer_size);

void transmit_data(const char *LOCAL_FUNCTION_TAG, int sock, char *tx_buffer);

void keep_alive_notification_task(void *pvParameters);

void build_command(char *string_com, ...);

void seperate_commands(char *rx_buffer, char command[][STR_LEN/2]);

void decode_string(char *str);

void encode_string(char *str);


void reset_esp_task(void *pvParameters);

esp_err_t check_internet_connection();

#endif 