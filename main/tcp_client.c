#include "tcp_client.h"

static const char *TAG_T = "tcp_client", *TAG_T_REMOTE = "tcp_remote_client", *TAG_T_LOCAL = "tcp_local_client";


void tcp_client_main(char *host, int port, char *local_host, int local_port) 
{
    task_tcp_params_t *tcp_params = malloc(sizeof(task_tcp_params_t));


    //tcp parameters
    strncpy(tcp_params->host, host, sizeof(tcp_params->host)); 
    tcp_params->port = port;
    strncpy(tcp_params->local_host, local_host, sizeof(tcp_params->local_host)); 
    tcp_params->local_port = local_port;
    tcp_params->activate_semaphore = xSemaphoreCreateBinary();
    tcp_params->stop_semaphore = xSemaphoreCreateBinary();

    //create tcp task
    xTaskCreate(remote_server_task, "remote_server_task", 4096, (void *)tcp_params, 4, NULL);

    vTaskDelay(pdMS_TO_TICKS(1000));

    xTaskCreate(local_server_task, "local_server_task", 4096, (void *)tcp_params, 4, NULL);

}

void remote_server_task(void *pvParameters)
{
    ESP_LOGI(TAG_T_REMOTE, "Remote client started");

    task_tcp_params_t *params = (task_tcp_params_t *)pvParameters;

    struct sockaddr_in dest_addr;
    int sock;
    struct timeval timeout;

    /* TESTING
    xSemaphoreGive(params->activate_semaphore);
    ESP_LOGI(TAG_T_REMOTE, "semaphore given...");


    while(1)
        vTaskDelay(pdMS_TO_TICKS(REMOTE_MS_WAIT));
    */
   
    while(1)
    {
        ESP_LOGI(TAG_T_REMOTE, "Checking internet connection...");
        while(check_internet_connection() == ESP_FAIL)
        {
            ESP_LOGE(TAG_T_REMOTE, "No internet connection detected...");
            xSemaphoreGive(params->activate_semaphore);
            xSemaphoreTake(params->stop_semaphore, pdMS_TO_TICKS(10));

            vTaskDelay(pdMS_TO_TICKS(REMOTE_MS_WAIT));   
        }
        ESP_LOGI(TAG_T_REMOTE, "Internet connection detected");

        tcp_create_socket(&dest_addr, &sock, &timeout, params->host, params->port);
        
        if(tcp_connect_to_host(NULL, &dest_addr, &sock, params->host, params->port) == ESP_FAIL)
        {
            ESP_LOGE(TAG_T_REMOTE, "Could not connect to remote server...");
            xSemaphoreGive(params->activate_semaphore);
            xSemaphoreTake(params->stop_semaphore, pdMS_TO_TICKS(10));
            vTaskDelay(pdMS_TO_TICKS(REMOTE_MS_WAIT));
            shutdown(sock, 0);
            continue;
        }

        xSemaphoreGive(params->stop_semaphore);
        xSemaphoreTake(params->activate_semaphore, pdMS_TO_TICKS(10));

        tcp_communicate_loop(TAG_T_REMOTE, &sock, NULL);

        shutdown(sock, 0);
    }


    //close resources
    free(params);

    ESP_LOGE(TAG_T_REMOTE, "Closing socket...");
    shutdown(sock, 0);
    close(sock);

    ESP_LOGE(TAG_T_REMOTE, "Closing remote task...");
    vTaskDelete(NULL);
}

void local_server_task(void *pvParameters)
{
    ESP_LOGI(TAG_T_LOCAL, "Local client started");
    task_tcp_params_t *params = (task_tcp_params_t *)pvParameters;

    struct sockaddr_in dest_addr;
    int sock;
    struct timeval timeout;

    uint8_t return_f = UNDEFINED;

    while(1)
    {
        ESP_LOGI(TAG_T_LOCAL, "Waiting for activate semaphore...");
        if(xSemaphoreTake(params->activate_semaphore, portMAX_DELAY) == pdTRUE)
        {
            ESP_LOGI(TAG_T_LOCAL, "Ativate semaphore taken...");
            while(1)
            {
                if(xSemaphoreTake(params->stop_semaphore, pdMS_TO_TICKS(SEMAPHORE_MS_WAIT)) == pdTRUE)
                    {ESP_LOGW(TAG_T_LOCAL, "Stop semaphore taken...");
                    break;}

                tcp_create_socket(&dest_addr, &sock, &timeout, params->local_host, params->local_port);

                if(tcp_connect_to_host(TAG_T_LOCAL, &dest_addr, &sock, params->local_host, params->local_port) == ESP_FAIL)
                {
                    ESP_LOGE(TAG_T_LOCAL, "Could not connect to server...");
                    vTaskDelay(pdMS_TO_TICKS(2000));
                    continue;
                }
                else
                {
                    return_f = tcp_communicate_loop(TAG_T_LOCAL, &sock, params->stop_semaphore);
                    if(return_f == STOP_SEMAPHORE)
                        break;
                }
                
                shutdown(sock, 0);
            }
        }
    }

    //close resources
    free(params);

    ESP_LOGE(TAG_T_LOCAL, "Closing socket...");
    shutdown(sock, 0);
    close(sock);

    ESP_LOGE(TAG_T_LOCAL, "Closing remote task...");
    vTaskDelete(NULL);
}

void tcp_create_socket(struct sockaddr_in *dest_addr_ptr, int *sock_ptr, struct timeval *timeout_ptr, char *host, int port)
{
    dest_addr_ptr->sin_addr.s_addr = inet_addr(host);
    dest_addr_ptr->sin_family = AF_INET;
    dest_addr_ptr->sin_port = htons(port);

    *sock_ptr = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (*sock_ptr < 0) {
        ESP_LOGE(TAG_T, "Unable to create socket");
        
        int err = errno;
        ESP_LOGE(TAG_T, "On socket(). Error num: %d (%s)", err, strerror(err));
        return;
    }
    
    // Set timeout
    timeout_ptr->tv_sec = SOCKET_TIMEOUT_SEC;
    timeout_ptr->tv_usec = 0;
    setsockopt(*sock_ptr, SOL_SOCKET, SO_RCVTIMEO, timeout_ptr, sizeof *timeout_ptr);

    return;
}

esp_err_t tcp_connect_to_host(const char *LOCAL_FUNCTION_TAG, struct sockaddr_in *dest_addr_ptr, int *sock_ptr, char *host, int port)
{
    if(LOCAL_FUNCTION_TAG != NULL)
        ESP_LOGW(LOCAL_FUNCTION_TAG, "Connecting to host: %s:%d...", host, port);

    if(connect(*sock_ptr, (struct sockaddr *)dest_addr_ptr, sizeof(*dest_addr_ptr)) != 0) 
    {
        if(LOCAL_FUNCTION_TAG != NULL)
        {
            int err = errno;
            ESP_LOGE(LOCAL_FUNCTION_TAG, "Socket unable to connect to host. Error num: %d (%s)", err, strerror(err));
        }

        close(*sock_ptr);
        return ESP_FAIL;
    }
    if(LOCAL_FUNCTION_TAG != NULL)
        ESP_LOGI(LOCAL_FUNCTION_TAG, "Successfully connected to %s:%d", host, port);

    return ESP_OK;
}

uint8_t tcp_communicate_loop(const char *LOCAL_FUNCTION_TAG, int *sock_ptr, SemaphoreHandle_t stop_semaphore)
{    
    char tx_buffer[STR_LEN], rx_buffer[STR_LEN], command[COMMANDS_MAX_QUANTITY][STR_LEN/2], local_buffer[STR_LEN*2], local_buffer_2[STR_LEN/2],
        keep_alive[STR_LEN];
    uint8_t error_counter = 0, return_f;
    uint32_t temp_32;
    float adc_value = 0;
    int len;

    char ack_msg[5] = "ACK";
    char nack_msg[5] = "NACK";
    build_command(keep_alive, "UABC", "a1264598", "K", "\0");

    TaskHandle_t keep_alive_handle = NULL;
    SemaphoreHandle_t keep_alive_semaphore;


    //send login
    build_command(tx_buffer, "UABC", "a1264598", "L", "\0");

    return_f = transmit_receive(LOCAL_FUNCTION_TAG, tx_buffer, rx_buffer, sock_ptr);
    if(return_f == COMMUNICATION_OK && check_ack(LOCAL_FUNCTION_TAG, rx_buffer) == ESP_OK)
        ESP_LOGI(LOCAL_FUNCTION_TAG, "Login succesfull");
    else
    {
        ESP_LOGE(LOCAL_FUNCTION_TAG, "Login failed...");
        return COMMUNICATION_FAIL;
    }
    return_f = UNDEFINED;

    //keep alive
    keep_alive_semaphore = xSemaphoreCreateBinary();
    xTaskCreate(keep_alive_task, "keep_alive_task", 4096, (void *)keep_alive_semaphore, 4, &keep_alive_handle);

    while(error_counter < MAX_ERROR_TCP_LOOP)
    {
        //check stop semaphore
        if(stop_semaphore != NULL)
        {
            if(xSemaphoreTake(stop_semaphore, pdMS_TO_TICKS(SEMAPHORE_MS_WAIT)) == pdTRUE)
            {
                ESP_LOGW(LOCAL_FUNCTION_TAG, "Stop semaphore taken...");
                return_f = STOP_SEMAPHORE;
                break;
            }
        }

        //if time to send keep alive
        if(xSemaphoreTake(keep_alive_semaphore, pdMS_TO_TICKS(SEMAPHORE_MS_WAIT)) == pdTRUE)
        {
            return_f = transmit_receive(LOCAL_FUNCTION_TAG, keep_alive, rx_buffer, sock_ptr);

            if(return_f == CONNECTION_CLOSED)
                return return_f;
            else if(return_f == COMMUNICATION_OK)
            {
                if(check_ack(LOCAL_FUNCTION_TAG, rx_buffer) == ESP_FAIL)
                    error_counter++;
            }
            else
            {
                ESP_LOGE(LOCAL_FUNCTION_TAG, "Keep alive failed...");
                error_counter++;
            }

            return_f = UNDEFINED;
        }

        //Check commands
        rx_buffer[0] = '\0';

        len = recv(*sock_ptr, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if(len > 0) 
        {
            rx_buffer[len] = '\0';
            ESP_LOGI(LOCAL_FUNCTION_TAG, "RX: %s", rx_buffer);

            seperate_commands(rx_buffer, command);

            if(strcmp(command[OPERATION_C], WRITE_O) == 0)
            {
                if(strcmp(command[RESOURCE_C], LED_R) == 0)
                {
                    if(strcmp(command[VALUE_C], "1") == 0)
                        set_led(1);
                    else if(strcmp(command[VALUE_C], "0") == 0)
                        set_led(0);

                    sprintf(local_buffer, "%s:%s", ack_msg, command[VALUE_C]);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else if(strcmp(command[RESOURCE_C], HABILITAR_R) == 0)
                {
                    if(*command[VALUE_C] == '1' || *command[VALUE_C] == '0')
                    {
                        write_nvs((char *)nvs_key_H, (char[]){*command[VALUE_C], '\0'}, TRUE );

                        sprintf(local_buffer, "%s:%c", ack_msg, *command[VALUE_C]);
                        send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                        ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                    }
                    else
                    {
                        send(*sock_ptr, nack_msg, strlen(nack_msg), 0);
                        ESP_LOGE(LOCAL_FUNCTION_TAG, "H value invalid: %s. TX: %s", command[VALUE_C], nack_msg);
                    }
                }
                else if(strcmp(command[RESOURCE_C], ENCENDER_R) == 0)
                {
                    write_nvs((char *)nvs_key_N, command[VALUE_C], TRUE );

                    temp_32 = (uint32_t)atoi(command[VALUE_C]);

                    sprintf(local_buffer, "%s:%d", ack_msg, (int)temp_32);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else if(strcmp(command[RESOURCE_C], APAGAR_R) == 0)
                {
                    write_nvs((char *)nvs_key_F, command[VALUE_C], TRUE );

                    temp_32 = (uint32_t)atoi(command[VALUE_C]);

                    sprintf(local_buffer, "%s:%d", ack_msg, (int)temp_32);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else
                {
                    send(*sock_ptr, nack_msg, strlen(nack_msg), 0);
                    ESP_LOGE(LOCAL_FUNCTION_TAG, "TX: %s", nack_msg);
                }
            }
            else if(strcmp(command[OPERATION_C], READ_O) == 0)
            {
                if(strcmp(command[RESOURCE_C], ADC_R) == 0)
                {
                    adc_value = read_adc_input(CHANNEL_0);
                    sprintf(local_buffer, "%s:%.2f", ack_msg, adc_value);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else if(strcmp(command[RESOURCE_C], HABILITAR_R) == 0)
                {
                    strncpy(local_buffer_2, "NULL", sizeof(local_buffer_2));
                    read_nvs((char *)nvs_key_H, local_buffer_2, sizeof(local_buffer_2), TRUE);

                    sprintf(local_buffer, "%s:%s", ack_msg, local_buffer_2);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else if(strcmp(command[RESOURCE_C], ENCENDER_R) == 0)
                {
                    strncpy(local_buffer_2, "NULL", sizeof(local_buffer_2));
                    read_nvs((char *)nvs_key_N, local_buffer_2, sizeof(local_buffer_2), TRUE);

                    sprintf(local_buffer, "%s:%s", ack_msg, local_buffer_2);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else if(strcmp(command[RESOURCE_C], APAGAR_R) == 0)
                {
                    strncpy(local_buffer_2, "NULL", sizeof(local_buffer_2));
                    read_nvs((char *)nvs_key_F, local_buffer_2, sizeof(local_buffer_2), TRUE);

                    sprintf(local_buffer, "%s:%s", ack_msg, local_buffer_2);
                    send(*sock_ptr, local_buffer, strlen(local_buffer), 0);
                    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", local_buffer);
                }
                else
                {
                    send(*sock_ptr, nack_msg, strlen(nack_msg), 0);
                    ESP_LOGE(LOCAL_FUNCTION_TAG, "TX: %s", nack_msg);
                }
            }
            else
            {
                send(*sock_ptr, nack_msg, strlen(nack_msg), 0);
                ESP_LOGE(LOCAL_FUNCTION_TAG, "TX: %s", nack_msg);
            }
        }
        else if(len == 0)
        {
            ESP_LOGE(LOCAL_FUNCTION_TAG, "Connection closed by peer.");
            return_f = CONNECTION_CLOSED;
            break;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }

    if(error_counter >= MAX_ERROR_TCP_LOOP)
    {
        ESP_LOGE(LOCAL_FUNCTION_TAG, "Max errors occured, exiting...");
        return_f = COMMUNICATION_FAIL;
    }

    
    ESP_LOGE(LOCAL_FUNCTION_TAG, "Closing client...");
    vTaskDelete(keep_alive_handle);
    vSemaphoreDelete(keep_alive_semaphore);
    
    return return_f;
}

uint8_t transmit_receive(const char *LOCAL_FUNCTION_TAG, char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    int len, err;
    uint8_t return_f = UNDEFINED, retry_cnt = 0;

    send(*sock_ptr, tx_buffer, strlen(tx_buffer), 0);
    ESP_LOGI(LOCAL_FUNCTION_TAG, "TX: %s", tx_buffer);

    rx_buffer[0] = '\0';

    while(retry_cnt < MAX_ERROR_RECV)
    {
        len = recv(*sock_ptr, rx_buffer, sizeof(rx_buffer) - 1, 0);
        
        if(len > 0) 
        {
            rx_buffer[len] = '\0';
            ESP_LOGI(LOCAL_FUNCTION_TAG, "RX: %s", rx_buffer);
            return_f = COMMUNICATION_OK;
            break;
        }
        else if(len == 0)
        {
            ESP_LOGE(LOCAL_FUNCTION_TAG, "Connection closed by peer.");
            return_f = CONNECTION_CLOSED;
            break;
        }
        else
        {
            err = errno;

            if(err == EAGAIN || err == EWOULDBLOCK)
                ESP_LOGW(LOCAL_FUNCTION_TAG, "recv() timeout, attempt... (%d/%d)", retry_cnt + 1, MAX_ERROR_RECV);
            else
            {
                ESP_LOGE(LOCAL_FUNCTION_TAG, "recv() error: %d (%s)", err, strerror(err));
                return_f = COMMUNICATION_FAIL;
                break;
            }
        }
        retry_cnt++;
    }

    return return_f;
}

/* DEPRECATED
esp_err_t login(const char *LOCAL_FUNCTION_TAG, char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    build_command(tx_buffer, "UABC", "a1264598", "L", "\0");
    if(transmit_receive(LOCAL_FUNCTION_TAG, tx_buffer, rx_buffer, sock_ptr) == ESP_OK)
    {
        if(check_ack(LOCAL_FUNCTION_TAG, rx_buffer) == ESP_OK)
        {
            ESP_LOGI(LOCAL_FUNCTION_TAG, "Login succesfull");
            return ESP_OK;
        }
        else
        {
            ESP_LOGE(LOCAL_FUNCTION_TAG, "Login failed...");
            return ESP_FAIL;
        }
    }
    else
        return ESP_FAIL;
}
*/

/* DEPRECATED
esp_err_t send_keep_alive(const char *LOCAL_FUNCTION_TAG, char *tx_buffer, char *rx_buffer, int *sock_ptr)
{
    build_command(tx_buffer, "UABC", "a1264598", "K", "\0");

    if(transmit_receive(LOCAL_FUNCTION_TAG, tx_buffer, rx_buffer, sock_ptr) == ESP_OK)
        return check_ack(LOCAL_FUNCTION_TAG, rx_buffer);
    else
        return ESP_FAIL;
}
*/

void keep_alive_task(void *pvParameters)
{
    SemaphoreHandle_t keep_alive_semaphore = (SemaphoreHandle_t)pvParameters;

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(KEEP_ALIVE_MS));

        xSemaphoreGive(keep_alive_semaphore);
    }

    vTaskDelete(NULL);
}

esp_err_t check_ack(const char *LOCAL_FUNCTION_TAG, char *rx_buffer)
{
    if(strcmp(rx_buffer, "ACK") != 0)
    {
        ESP_LOGE(LOCAL_FUNCTION_TAG, "Acknowledge not received...");
        return ESP_FAIL;
    }
    else
    {
        ESP_LOGI(LOCAL_FUNCTION_TAG, "Acknowledge received");
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

void seperate_commands(char *rx_buffer, char command[][STR_LEN/2])
{
    char *local_ptr= rx_buffer;;
    uint16_t counter_cmmd= 0, counter_word= 0;

    //initialize commands
    for(int i=0; i<COMMANDS_MAX_QUANTITY; i++)
        *command[i] = '\0';

    //separate by delimiter
    while(*local_ptr != '\0')
    {
        if(*local_ptr == ':')
        {
            command[counter_cmmd][counter_word] = '\0'; // terminator
            counter_cmmd++;
            if(counter_cmmd == COMMANDS_MAX_QUANTITY)
            {
                counter_cmmd--;
                break;
            }
            counter_word = 0;
        }
        else
        {
            if(counter_word == sizeof(command[0])) counter_word=0; //buffer overflow fix
            command[counter_cmmd][counter_word] = *local_ptr;
            counter_word++;
        }
        local_ptr++;
    }
    command[counter_cmmd][counter_word] = '\0'; // terminator
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

    //ESP_LOGW(TAG_T, "Pinging Google...");

    tcp_create_socket(&dest_addr, &sock, &timeout, HOST_GOOGLE, PORT_GOOGLE);
    
    if(tcp_connect_to_host(NULL, &dest_addr, &sock, HOST_GOOGLE, PORT_GOOGLE) != ESP_OK)
    {
        //ESP_LOGE(TAG_T, "Failed to connect to google host. There is no internet connection...");
        return_v = ESP_FAIL;
    }
    else
    {
        //send hhtp request
        send(sock, tx_message, strlen(tx_message), 0);  
        //ESP_LOGI(TAG_T, "Message send to %s (%d bytes)", HOST_GOOGLE, strlen(tx_message));

        //receive text
        int len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
        if(len < 0) 
        {
            //ESP_LOGE(TAG_T, "Error occurred during receiving: errno %d", errno);
            ESP_LOGE("Internet", "Error occured in RX. Lenght: %d", len);

            int err = errno;
            ESP_LOGE("Internet", "On recv(). Error num: %d (%s)", err, strerror(err));
            return_v = ESP_FAIL;
        }

        //ESP_LOGI(TAG_T, "Received %d bytes", len);
        //ESP_LOGI(TAG_T, "There is an internet connection.");
        return_v = ESP_OK;
    }
     
    //close resources

    //ESP_LOGI(TAG_T, "Closing google socket...");
    shutdown(sock, 0);
    close(sock);

    return return_v;
}