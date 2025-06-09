#ifndef GPIO
#define GPIO

#include "esp_log.h"

#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#define LED GPIO_NUM_2

extern volatile uint8_t l_state;

void init_gpio(void);

uint8_t get_led_state();

void set_led(uint8_t state);

#endif 