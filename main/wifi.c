#include "wifi.h"

const char *TAG_W = "WIFI";
uint8_t connected_w = WATING_CONEXION, ip_flag;

static int retry_count = 0;

esp_netif_ip_info_t ip_info;

char ip_addr[IP_LENGHT] = "\0";


void wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id)
    {
    case WIFI_EVENT_STA_START:
        ESP_LOGI(TAG_W, "WiFi connecting WIFI_EVENT_STA_START ...");
        esp_wifi_connect();
        break;
    case WIFI_EVENT_STA_CONNECTED:
        ESP_LOGI(TAG_W,"WiFi connected WIFI_EVENT_STA_CONNECTED ...");
        connected_w = CONNECTED;
        retry_count = 0;
        break;
    case WIFI_EVENT_STA_DISCONNECTED:
        if(retry_count < MAX_RETRY)
        {
            ESP_LOGE(TAG_W, "WiFi lost connection WIFI_EVENT_STA_DISCONNECTED. Retrying ... Attempt %d/%d", retry_count + 1, MAX_RETRY);
            esp_wifi_connect();
            retry_count++;
        }
        else
        {
            esp_wifi_stop();
            esp_wifi_deinit();
            ESP_LOGE(TAG_W,"WiFi stopped.");
            connected_w = FAILED;
        }
        connected_w = WATING_CONNEXION;
        break;
    case IP_EVENT_STA_GOT_IP:
        ESP_LOGI(TAG_W,"WiFi got IP");
        retry_count=0;

        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_get_ip_info(netif, &ip_info);
        snprintf(ip_addr, IP_LENGHT, "%d.%d.%d.%d", IP2STR(&ip_info.ip));
        ip_flag = 1;
        break;
    }
}

void wifi_connect()
{
    ip_flag = 0;
    esp_err_t error = nvs_flash_init();
    if(error == ESP_ERR_NVS_NO_FREE_PAGES || error == ESP_ERR_NVS_NEW_VERSION_FOUND) 
    {
        nvs_flash_erase();
        nvs_flash_init();
    }
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();


    wifi_init_config_t wifi_initiation = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&wifi_initiation);
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, wifi_event_handler, NULL);

    wifi_config_t wifi_configuration = {
        .sta = {
            .ssid = SSID,
            .password = PASS}};
    
    ESP_LOGI(TAG_W, "Wifi SSID: %s", (char*)wifi_configuration.sta.ssid);
    ESP_LOGI(TAG_W, "Wifi PASS: %s", (char*)wifi_configuration.sta.password);
    
    esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_configuration);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    ESP_LOGE(TAG, "Wating for connexion....");
    while(ip_flag == 0)
    {
        if(connected_w == FAILED)
        {
            ESP_LOGE(TAG, "Wifi conexion failed...");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}