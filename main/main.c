#include "esp_log.h"
#include "esp_system.h"

#include <string.h>

#include "tcp_client.h"
#include "wifi.h"
#include "gpio.h"
#include "adc.h"
#include "clock.h"
#include "led_timer.h"

const char *TAG = "MAIN";

//wifi credentials
#define SSID "COTA_PC"
#define PASS "0402{V8z"
//#define SSID "IoT_AP"
//#define PASS "12345678"
//#define SSID "Totalplay-2.4G-b518"
//#define PASS "Qxm2EAzh99Ce7Lfk"

//tcp host and port
//#define REMOTE_IP_ADDR "192.168.100.13" 
#define REMOTE_IP_ADDR "82.180.173.228" 
#define REMOTE_PORT 8250

#define LOCAL_IP_ADDR "192.168.100.182" 
#define LOCAL_PORT 8250

#define RESET_TIME 7000

void app_main(void)
{
    init_gpio();
    adc_init();
    ESP_ERROR_CHECK(init_nvs());

    //wait for wifi connection, if fail restart
    if(wifi_connect(SSID, PASS) == ESP_FAIL)
    {
        ESP_LOGE(TAG, "Could not connect to wifi, restarting...");
        vTaskDelay(pdMS_TO_TICKS(RESET_TIME));
        esp_restart();
    }
   
    //start internal clock
    start_clock();

    //start tcp client
    tcp_client_main(REMOTE_IP_ADDR, REMOTE_PORT, LOCAL_IP_ADDR, LOCAL_PORT);

    //start led timer task
    xTaskCreate(led_timer_task, "led_timer_task", 4096, NULL, 3, NULL);
    
    while(1)
    {
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
