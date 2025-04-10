#include "esp_log.h"
#include "esp_system.h"

#include <string.h>

#include "tcp_client.h"
#include "wifi.h"

const char *TAG = "MAIN";

//wifi credentials
#define SSID "Totalplay-2.4G-b518"
#define PASS "Qxm2EAzh99Ce7Lfk"

//tcp host and port
#define HOST_IP_ADDR "192.168.100.9" 
#define PORT 8090

void app_main(void)
{
    if(!wifi_connect(SSID, PASS))
    {
        ESP_LOGE(TAG_W, "Could not connect to wifi, restarting...");
        vTaskDelay(pdMS_TO_TICKS(5000));
        esp_restart();
    }

    //check internet task/timer

    tcp_client(HOST_IP_ADDR, PORT);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
