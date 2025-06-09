#include "adc.h"

adc_oneshot_unit_handle_t adc1_handle;

void adc_init()
{
    adc_oneshot_unit_init_cfg_t init_config1 = 
    {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .bitwidth = ADC_BITWIDTH_12, 
        .atten = ADC_ATTEN_DB_12,
    };

    //channel 0, pin 14
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, CHANNEL_0, &config)); 
}

int read_adc_input(adc_channel_t channel)
{
    int adc_raw = 0;

    ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, CHANNEL_0, &adc_raw));

    return adc_raw; 
}