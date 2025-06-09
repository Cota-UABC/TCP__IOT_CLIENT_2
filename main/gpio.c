#include "gpio.h"

volatile uint8_t l_state = 0;

void init_gpio(void)
{
    gpio_set_direction(LED, GPIO_MODE_OUTPUT);
    gpio_set_level(LED, 0);
}

int get_led_state()
{
    return gpio_get_level(LED);
}

void set_led(uint8_t state)
{
    if(state)
    {
        gpio_set_level(LED, 1);
        l_state = 1;
    }
    else
    {
        gpio_set_level(LED, 0);
        l_state = 0;
    }
}
