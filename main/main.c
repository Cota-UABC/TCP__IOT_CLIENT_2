#include "esp_log.h"
#include "esp_system.h"

#include <string.h>

#include "tcp_client.h"
#include "wifi.h"
#include "gpio.h"
#include "adc.h"

const char *TAG = "MAIN";

//wifi credentials
//#define SSID "Totalplay-2.4G-b518"
//#define PASS "Qxm2EAzh99Ce7Lfk"
//#define SSID "COTA_PC"
//#define PASS "0402{V8z"
#define SSID "IoT_AP"
#define PASS "12345678"

//tcp host and port
#define IOT_IP_ADDR "82.180.173.228" 
#define IOT_PORT 8090

#define LOCAL_IP_ADDR "192.168.0.183" 
#define LOCAL_PORT 8250

void app_main(void)
{
    init_gpio();
    adc_init();

    if(wifi_connect(SSID, PASS) == ESP_FAIL)
    {
        ESP_LOGE(TAG_W, "Could not connect to wifi, restarting...");
        vTaskDelay(pdMS_TO_TICKS(7000));
        esp_restart();
    }

    tcp_client_main(IOT_IP_ADDR, IOT_PORT, LOCAL_IP_ADDR, LOCAL_PORT);

    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
