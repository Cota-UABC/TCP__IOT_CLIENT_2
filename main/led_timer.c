#include "led_timer.h"

static const char *TAG_LT = "LED_TIMER";

char *nvs_key_H = "Habilitar", *nvs_key_N = "Prender", *nvs_key_F = "Apagar";

void led_timer_task(void *pvParameters)
{
    char local_buffer[128];
    uint32_t local_seconds = 0, start_time, stop_time;

    local_buffer[0] = "\0";

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        
        read_nvs((char *)nvs_key_H, local_buffer, sizeof(local_buffer), 0);

        if(strcmp(local_buffer, "1"))
        {
            if(xSemaphoreTake(seconds_mutex, portMAX_DELAY))
            {
                local_seconds = clock_seconds;
                xSemaphoreGive(seconds_mutex);
            }

            read_nvs((char *)nvs_key_N, local_buffer, sizeof(local_buffer), 0);
            start_time = atoi(local_buffer);

            read_nvs((char *)nvs_key_F, local_buffer, sizeof(local_buffer), 0);
            stop_time = atoi(local_buffer);

            if(local_buffer >= start_time)
                set_led(1);

            else if(local_buffer >= start_time)
                set_led(0);
        }
    }

    vTaskDelete(NULL);
}