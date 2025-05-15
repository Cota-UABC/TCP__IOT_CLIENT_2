#include "clock.h"

static const char *TAG_CLK = "TIME";

SemaphoreHandle_t seconds_mutex;
uint32_t clock_seconds;

uint32_t get_real_time() 
{
    uint8_t ntp_packet[48] = { 0 };
    uint32_t seconds = 0;

    setenv("TZ", "PST8PDT,M3.2.0/2,M11.1.0/2", 1);
    tzset();

    int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
    if(sock < 0) 
    {
        ESP_LOGE(TAG_CLK, "Socket creation failed");
        vTaskDelete(NULL);
        return seconds;
    }

    struct sockaddr_in server_addr = 
    {
        .sin_family = AF_INET,
        .sin_port = htons(NTP_PORT),
        .sin_addr.s_addr = inet_addr(NTP_HOST) 
    };

    ntp_packet[0] = 0b11100011;

    sendto(sock, ntp_packet, sizeof(ntp_packet), 0,(struct sockaddr *)&server_addr, sizeof(server_addr));

    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);
    int len = recvfrom(sock, ntp_packet, sizeof(ntp_packet), 0,(struct sockaddr *)&from_addr, &from_len);

    if(len == 48) 
    {
        uint32_t seconds_since_1900 = (ntp_packet[40] << 24) |
                                      (ntp_packet[41] << 16) |
                                      (ntp_packet[42] << 8) |
                                      (ntp_packet[43]);

        time_t timestamp = seconds_since_1900 - 2208988800U;
        struct timeval tv = { .tv_sec = timestamp };
        settimeofday(&tv, NULL);

        struct tm timeinfo;
        localtime_r(&tv.tv_sec, &timeinfo);

        char time_str[64];
        strftime(time_str, sizeof(time_str), "%c", &timeinfo);
        ESP_LOGI(TAG_CLK, "Local Time: %s", time_str);


        seconds = timeinfo.tm_hour * 3600 + timeinfo.tm_min * 60 + timeinfo.tm_sec;

        ESP_LOGI(TAG_CLK, "Seconds today: %d", (int)seconds);
    } 
    else
        ESP_LOGE(TAG_CLK, "Error receiving response from NTP");


    shutdown(sock, 0);
    close(sock);
    
    return seconds;
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
        if(clock_seconds % 10 == 0)
            ESP_LOGW(TAG_CLK, "%d seconds", (int)clock_seconds);

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