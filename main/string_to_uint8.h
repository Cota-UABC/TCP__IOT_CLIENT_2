#ifndef STU
#define STU

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "esp_err.h"
#include "esp_log.h"

#include "esp_log.h"

esp_err_t string_to_uint8(char *str, uint8_t *result);

esp_err_t string_to_uint16(char *str, uint16_t *result);

#endif