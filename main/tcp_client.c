#include "tcp_client.h"

static const char *TAG_T = "tcp_client";

void tcp_client_main(char *host, int port, char *local_host, int local_port) 
{
    task_tcp_params_t *tcp_params = malloc(sizeof(task_tcp_params_t));

    //tcp parameters
    strncpy(tcp_params->host, host, sizeof(tcp_params->host)); 
    tcp_params->port = port;
    strncpy(tcp_params->local_host, local_host, sizeof(tcp_params->local_host)); 
    tcp_params->local_port = local_port;

    //create tcp task
    xTaskCreate(tcp_task, "tcp_task", 4096, (void *)tcp_params, 4, NULL);
}

void tcp_task(void *pvParameters)
{
    task_tcp_params_t *params = (task_tcp_params_t *)pvParameters;

    uint8_t return_f = 0;

    while(1)
    {
        if(check_internet_connection() == ESP_OK)
        {
            ESP_LOGI(TAG_T, "Connecting to IOT server...");
            return_f = tcp_server_connect(params->host, params->port, FALSE);
        } 
        if(return_f == MAX_RETRIES)
        {
            ESP_LOGI(TAG_T, "Connecting to LOCAL server...");
            return_f = tcp_server_connect(params->local_host, params->local_port, TRUE);

            if(return_f == MAX_RETRIES)
                break;
        }
    }
    
    //close resources
    free(params);

    ESP_LOGE(TAG_T, "Closing tcp task...");
    vTaskDelete(NULL);
}

uint8_t tcp_server_connect(char *host, int port, uint8_t check_internet)
{
    uint8_t return_f = FAIL, counter = 0;

    struct sockaddr_in dest_addr;
    int sock;
    struct timeval timeout;
    
    while(counter < CONNECT_MAX_RETRY)
    {
        ESP_LOGI(TAG_T, "Connecting, attempt %d/%d", counter+1, CONNECT_MAX_RETRY);
        counter++;

        if(tcp_create_socket(&dest_addr, &sock, host, port) != ESP_OK)
        {
            ESP_LOGE(TAG_T, "Socket creation failed...");
            shutdown(sock, 0);
            close(sock);
            return FAIL;
        }
        else
        {
            if(tcp_connect_to_host(&dest_addr, &sock, &timeout, host, port) == ESP_FAIL)
                ESP_LOGE(TAG_T, "Host connection failed...");
            else
                tcp_communicate_loop(&sock, check_internet);
        }
        ESP_LOGE(TAG_T, "Closing socket...");
        shutdown(sock, 0);
        close(sock);
    }
    return_f = MAX_RETRIES;


    return return_f;
}

esp_err_t tcp_create_socket(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, char *host, int port)
{
    ESP_LOGW(TAG_T, "Creating socket...");

    dest_addr_ptr->sin_addr.s_addr = inet_addr(host);
    dest_addr_ptr->sin_family = AF_INET;
    dest_addr_ptr->sin_port = htons(port);

    *sock_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (*sock_ptr < 0) {
        ESP_LOGE(TAG_T, "Unable to create socket");
        return ESP_FAIL;
    }
    ESP_LOGI(TAG_T, "Socket created successfully");

    return ESP_OK;
}

esp_err_t tcp_connect_to_host(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port)
{
    ESP_LOGW(TAG_T, "Connecting to host: %s:%d...", host, port);

    // Set timeout
    timeout_ptr->tv_sec = 1;
    timeout_ptr->tv_usec = 0;
    setsockopt(*sock_ptr, SOL_SOCKET, SO_RCVTIMEO, timeout_ptr, sizeof *timeout_ptr);

    if (connect(*sock_ptr, (struct sockaddr *)dest_addr_ptr, sizeof(*dest_addr_ptr)) != 0) 
    {
        ESP_LOGE(TAG_T, "Socket unable to connect");
        close(*sock_ptr);
        return ESP_FAIL;
    }
    ESP_LOGI(TAG_T, "Successfully connected to %s:%d", host, port);

    return ESP_OK;
}

void tcp_communicate_loop(int *sock_ptr, uint8_t check_internet)
{    
    char tx_buffer[STR_LEN], rx_buffer[STR_LEN];
    uint8_t error_counter = 0;

    TaskHandle_t keep_alive_handle = NULL;
    SemaphoreHandle_t keep_alive_semaphore = xSemaphoreCreateBinary();

    if(login(tx_buffer, rx_buffer, sock_ptr) == ESP_FAIL)
        return;

    xTaskCreate(keep_alive_task, "keep_alive_task", 4096, (void *)keep_alive_semaphore, 4, &keep_alive_handle);

    while(error_counter < MAX_ERROR_COUNT)
    {
        if(xSemaphoreTake(keep_alive_semaphore, 20) == pdTRUE)
            if(send_keep_alive(tx_buffer, rx_buffer, sock_ptr) == ESP_FAIL)
                error_counter++;

        //CHECK COMMANDS
        rx_buffer[0] = '\0';

        int len = recv(*sock_ptr, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if(len > 0) 
        {
            rx_buffer[len] = '\0';
            ESP_LOGI(TAG_T, "RX: %s", rx_buffer);

            if(strcmp(rx_buffer, "UABC:a1264598:W:L:1") == 0)
                ESP_LOGW(TAG_T, "LED");
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_LOGE(TAG_T, "Max errors occured, exiting...");


    vTaskDelete(keep_alive_handle);
    vSemaphoreDelete(keep_alive_semaphore);
}

esp_err_t transmit_receive(char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    send(*sock_ptr, tx_buffer, strlen(tx_buffer), 0);
    ESP_LOGI(TAG_T, "TX: %s", tx_buffer);

    rx_buffer[0] = '\0';

    int len = recv(*sock_ptr, rx_buffer, sizeof(rx_buffer) - 1, 0);
    
    if(len > 0) 
    {
        rx_buffer[len] = '\0';
        ESP_LOGI(TAG_T, "RX: %s", rx_buffer);
        return ESP_OK;
    }
    else
    {
        ESP_LOGE(TAG_T, "Error occured in RX");
        return ESP_FAIL;
    }
}

esp_err_t login(char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    build_command(tx_buffer, "UABC", "a1264598", "L", "\0");
    if(transmit_receive(tx_buffer, rx_buffer, sock_ptr) == ESP_OK)
    {
        if(check_ack(rx_buffer) == ESP_OK)
        {
            ESP_LOGI(TAG_T, "Login succesfull");
            return ESP_OK;
        }
        else
        {
            ESP_LOGE(TAG_T, "Login failed...");
            return ESP_FAIL;
        }
    }
    else
        return ESP_FAIL;
}

esp_err_t send_keep_alive(char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    build_command(tx_buffer, "UABC", "a1264598", "K", "\0");

    if(transmit_receive(tx_buffer, rx_buffer, sock_ptr) == ESP_OK)
        return check_ack(rx_buffer);
    else
        return ESP_FAIL;
}

void keep_alive_task(void *pvParameters)
{
    SemaphoreHandle_t keep_alive_semaphore = (SemaphoreHandle_t)pvParameters;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(KEEP_ALIVE_MS_WAIT));

        xSemaphoreGive(keep_alive_semaphore);
    }

    vTaskDelete(NULL);
}

esp_err_t check_ack(char *rx_buffer)
{
    if(strcmp(rx_buffer, "ACK") != 0)
    {
        ESP_LOGE(TAG_T, "Acknowledge not received...");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(TAG_T, "Acknowledge received");
        return ESP_OK;
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

//check connection to google
esp_err_t check_internet_connection()
{
    uint8_t return_v;
    char tx_message[STR_LEN*2], rx_buffer[STR_LEN];
    
    struct sockaddr_in dest_addr;
    int sock;
    struct timeval timeout;

    strcpy(tx_message, "GET / HTTP/1.1\r\nHost: google.com\r\nConnection: close\r\n\r\n");

    ESP_LOGW(TAG_T, "Pinging Google...");

    if(tcp_create_socket(&dest_addr, &sock, HOST_GOOGLE, PORT_GOOGLE) != ESP_OK)
    {
        ESP_LOGE(TAG_T, "Failed to initialize internet socket...");
        return_v = ESP_FAIL;
    }
    else if(tcp_connect_to_host(&dest_addr, &sock, &timeout, HOST_GOOGLE, PORT_GOOGLE) != ESP_OK)
    {
        ESP_LOGE(TAG_T, "Failed to connect to google host. There is no internet connection...");
        return_v = ESP_FAIL;
    }
    else
    {
        //send hhtp request
        send(sock, tx_message, strlen(tx_message), 0);  
        ESP_LOGI(TAG_T, "Message send to %s (%d bytes)", HOST_GOOGLE, strlen(tx_message));

        //receive text
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if(len < 0) 
        {
            ESP_LOGE(TAG_T, "Error occurred during receiving: errno %d", errno);
            return_v = ESP_FAIL;
        }

        ESP_LOGI(TAG_T, "Received %d bytes", len);
        ESP_LOGI(TAG_T, "There is an internet connection.");
        return_v = ESP_OK;
    }
     
    //close resources

    ESP_LOGI(TAG_T, "Closing google socket...");
    shutdown(sock, 0);
    close(sock);

    return return_v;
}