#ifndef GPIO
#define GPIO

#include "esp_log.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#define LED GPIO_NUM_2

extern volatile uint8_t l_state, b_state, b_state_old, edge;

void init_gpio(void);

void set_led(uint8_t state);

#endif 