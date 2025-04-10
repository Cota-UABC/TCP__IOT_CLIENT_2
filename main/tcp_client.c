#include "tcp_client.h"

static const char *TAG_T = "tcp_client";

void tcp_client(char *host, int port) 
{
    //tcp parameters
    task_tcp_params_t *tcp_params = malloc(sizeof(task_tcp_params_t));
    strncpy(tcp_params->host, host, sizeof(tcp_params->host)); 
    tcp_params->port = port;

    //create tcp task
    xTaskCreate(tcp_task, "tcp_task", 4096, tcp_params, 4, NULL);
}

void tcp_task(void *pvParameters)
{
    task_tcp_params_t *params = (task_tcp_params_t *)pvParameters;

    struct sockaddr_in dest_addr;
    int sock;
    struct timeval timeout;

    if(tcp_init_connect(&dest_addr, &sock, &timeout, params->host, params->port))
        tcp_communicate(&sock); //WIP   
    else
        ESP_LOGE(TAG_T, "Tcp init failed");

    free(params);

    ESP_LOGE(TAG_T, "Closing socket...");
    shutdown(sock, 0);
    close(sock);

    ESP_LOGE(TAG_T, "Closing tcp task...");
    vTaskDelete(NULL);
}

uint8_t tcp_init_connect(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port)
{
    dest_addr_ptr->sin_addr.s_addr = inet_addr(host);
    dest_addr_ptr->sin_family = AF_INET;
    dest_addr_ptr->sin_port = htons(port);

    *sock_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (*sock_ptr < 0) {
        ESP_LOGE(TAG_T, "Unable to create socket");
        return 0;
    }
    ESP_LOGW(TAG_T, "Socket created, connecting to %s:%d...", host, port);

    // Set timeout
    timeout_ptr->tv_sec = 1;
    timeout_ptr->tv_usec = 0;
    setsockopt(*sock_ptr, SOL_SOCKET, SO_RCVTIMEO, timeout_ptr, sizeof *timeout_ptr);

    if (connect(*sock_ptr, (struct sockaddr *)dest_addr_ptr, sizeof(*dest_addr_ptr)) != 0) {
        ESP_LOGE(TAG_T, "Socket unable to connect");
        close(*sock_ptr);
        return 0;
    }
    ESP_LOGI(TAG_T, "Successfully connected to %s:%d", host, port);

    return 1;
}

void transmit_receive(char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    send(*sock_ptr, tx_buffer, strlen(tx_buffer), 0);
    ESP_LOGI(TAG_T, "TX: %s", tx_buffer);

    int len = recv(*sock_ptr, rx_buffer, sizeof(rx_buffer) - 1, 0);
    
    if (len > 0) {
        rx_buffer[len] = '\0';
        ESP_LOGI(TAG_T, "RX: %s", rx_buffer);
    }
}

void tcp_communicate(int *sock_ptr)
{    
    char tx_buffer[STR_LEN], rx_buffer[STR_LEN];

    //login
    if(!login(tx_buffer, rx_buffer, sock_ptr))
        return;

    while(1)
    {
        keep_alive(tx_buffer, rx_buffer, sock_ptr);

        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

uint8_t login(char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    build_command(tx_buffer, "UABC", "a1264598", "L", "\0");
    transmit_receive(tx_buffer, rx_buffer, sock_ptr);

    if(check_ack(rx_buffer))
    {
        ESP_LOGI(TAG_T, "Login succesfull");
        return 1;
    }
    else
    {
        ESP_LOGE(TAG_T, "Login failed...");
        return 0;
    }
}

void keep_alive(char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    build_command(tx_buffer, "UABC", "a1264598", "K", "\0");

    transmit_receive(tx_buffer, rx_buffer, sock_ptr);

    if(strcmp(rx_buffer, "ACK") =! 0)
        ESP_LOGE(TAG_T, "Acknowledge not received...");

}

uint8_t check_ack(char *rx_buffer)
{
    if(strcmp(rx_buffer, "ACK") =! 0)
    {
        ESP_LOGE(TAG_T, "Acknowledge not received...");
        return 0;
    }
    else
    {
        ESP_LOGI(TAG_T, "Acknowledge received");
        return 1;
    }
}

void build_command(char *string_com, ...)
{
    strcpy(string_com, "\0");

    va_list args;
    va_start(args, string_com);
    char str_temp[STR_LEN];

    for(int i=0; i<COMMANDS_MAX_QUANTITY; i++)
    {
        sprintf(str_temp, va_arg(args, char *));
        if(!strcmp(str_temp, "\0")) //if last argument
        {
            string_com[strlen(string_com) - 1] = '\0';
            break;
        }

        strcat(string_com, str_temp);
        strcat(string_com, ":");
    }
    va_end(args);
}
