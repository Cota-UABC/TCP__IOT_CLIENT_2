#include "led_timer.h"

static const char *TAG_LT = "LED_TIMER";

char *nvs_key_H = "Habilitar", *nvs_key_N = "Prender", *nvs_key_F = "Apagar";

void led_timer_task(void *pvParameters)
{
    ESP_LOGI(TAG_LT, "Led timer task started");

    char local_buffer[128];
    uint32_t local_seconds = 0, start_time, stop_time;

    local_buffer[0] = '\0';

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        read_nvs((char *)nvs_key_H, local_buffer, sizeof(local_buffer), 0);

        if(strcmp(local_buffer, "1") == 0)
        {
            if(xSemaphoreTake(seconds_mutex, portMAX_DELAY))
            {
                local_seconds = clock_seconds;
                xSemaphoreGive(seconds_mutex);
            }

            if(read_nvs((char *)nvs_key_N, local_buffer, sizeof(local_buffer), 0) != ESP_OK)
                continue;
            start_time =(uint32_t)atoi(local_buffer);

            if(read_nvs((char *)nvs_key_F, local_buffer, sizeof(local_buffer), 0) != ESP_OK)
                continue;
            stop_time = (uint32_t)atoi(local_buffer);

            if(start_time >= stop_time)
            {
                if(local_seconds % 10 == 0)
                    ESP_LOGI(TAG_LT, "Time frame invalid: Start: %d, Stop: %d", (int)start_time, (int)stop_time);
                continue;
            }

            if(local_seconds >= start_time && local_seconds <= stop_time)
                set_led(1);

            else
                set_led(0);
        }
    }

    vTaskDelete(NULL);
}