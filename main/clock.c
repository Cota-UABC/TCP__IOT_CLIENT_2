#include "real_time.h"

static const char *TAG_CLK = "TIME";

SemaphoreHandle_t seconds_mutex;
uint32_t clock_seconds;

const char *REQUEST = "GET /api/timezone/America/Tijuana.txt HTTP/1.1\r\n"
                            "Host: worldtimeapi.org\r\n"
                            "Connection: close\r\n"
                            "\r\n";

uint32_t get_real_time() 
{
    struct sockaddr_in dest_addr = {
        .sin_addr.s_addr = inet_addr(REAL_TIME_IP), 
        .sin_family = AF_INET,
        .sin_port = htons(atoi(REAL_TIME_PORT))
    };

    char rx_buffer[1024], *date;
    int len, sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);

    if(sock < 0) 
    {
        ESP_LOGE(TAG_CLK, "Unable to create socket: errno %d", errno);
        return 0;
    }
    ESP_LOGI(TAG_CLK, "Socket created, connecting to %s:%s", REAL_TIME_IP, REAL_TIME_PORT);


    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;
    setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);

    if(connect(sock, (struct sockaddr *)&dest_addr, sizeof(dest_addr)) != 0) 
    {
        ESP_LOGE(TAG_CLK, "error connecting to server");
        shutdown(sock, 0);
        close(sock);
        return 0;
    }

    send(sock, REQUEST, strlen(REQUEST), 0);
    printf("Request send\n");
    
    len = recv(sock, rx_buffer, sizeof(rx_buffer) - 1, 0);
    printf("Recieved len: %d", len);
    if(len > 0) 
    {
        rx_buffer[len] = '\0';
        ESP_LOGI(TAG_CLK, "Rx len: %d", len);
        
        printf("Received: %s\n", rx_buffer);
    }
    else
    {
        ESP_LOGE(TAG_CLK, "No response");
    }

    //date = strstr(rx_buffer, "Date: ");
    /*if(date) 
    {
        date += 6; 
        char *hora = strchr(date, ' ');
        if(hora) 
        {
            hora = strchr(hora + 1, ' ');
            if (hora) 
            {
                hora = strchr(hora + 1, ' ');
                if (hora) 
                {
                    hora += 1;
                    char hour[128];
                    strncpy(hour, hora, 8);
                    hour[8] = '\0';
                    ESP_LOGI(TAG_CLK, "Hora: %s", hour);
                }
            }
        }
    } 
    else
        ESP_LOGE(TAG_CLK, "Could not get time data received...");
    */

    shutdown(sock, 0);
    close(sock);
    
    return 0;
}

void start_clock()
{
    seconds_mutex = xSemaphoreCreateMutex();

    if(xSemaphoreTake(seconds_mutex, portMAX_DELAY))
    {
        clock_seconds = get_real_time();  
        xSemaphoreGive(seconds_mutex);
    }

    xTaskCreate(clock_task, "clock_task", 4096, NULL, 4, NULL);
}

void clock_task(void *pvParameters)
{
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));

        if(xSemaphoreTake(seconds_mutex, portMAX_DELAY))
        {
            clock_seconds++;

            if(clock_seconds >= 86400)
                clock_seconds = 0;
            
            xSemaphoreGive(seconds_mutex);
        }
    }

    vTaskDelete(NULL);
}