#include "esp_log.h"
#include <string.h>

#include "tcp_client.h"
#include "wifi.h"

const char *TAG = "MAIN";

//tcp host and port
#define HOST_IP_ADDR "192.168.1.100" 
#define PORT 8080

void app_main(void)
{
    wifi_connect(); //WIP return state

    tcp_client(HOST_IP_ADDR, PORT);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
