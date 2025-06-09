#include "string_to_uint8.h"

char *TAG_STU = "STU";

esp_err_t string_to_uint8(char *str, uint8_t *result) 
{
    char *endptr;
    unsigned long num = strtoul(str, &endptr, 10); 

    if (*endptr != '\0') {
        ESP_LOGE(TAG_STU, "String has invalid numeric values: %s", endptr);
        return ESP_FAIL;
    }
    
    if (num > UINT8_MAX) {
        ESP_LOGE(TAG_STU, "Number exceeds uint8_t.");
        *result = 0;
        return ESP_FAIL;
    }

    *result = (uint8_t)num; 
    //ESP_LOGI(TAG_STU, "uint8_t number converted successfully: %u", *result);
    return ESP_OK;
}

