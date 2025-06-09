#ifndef ADC
#define ADC

#include "esp_adc/adc_oneshot.h"
#include "math.h"

#define CHANNEL_0 ADC_CHANNEL_0 //GPIO 4


extern adc_oneshot_unit_handle_t adc1_handle;

void adc_init();

int read_adc_input(adc_channel_t channel);

#endif 