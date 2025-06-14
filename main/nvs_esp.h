#ifndef NVS_ESP
#define NVS_ESP

#include <string.h>
#include "esp_log.h"

#include "nvs.h"
#include "nvs_flash.h"

extern char *TAG_NVS;

esp_err_t init_nvs();

esp_err_t read_nvs(char *key, char *value, size_t len, uint8_t print);

esp_err_t write_nvs(char *key, char *value, uint8_t print);

#endif