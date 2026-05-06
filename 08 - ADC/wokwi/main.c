#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include <stdio.h>


#define POT_PIN GPIO_NUM_11
#define PUSH_BUTTON_PIN GPIO_NUM_12
#define LED_PIN GPIO_NUM_13
#define ADC_CHANNEL ADC_CHANNEL_0

static adc_oneshot_unit_handle_t adc2_handle;


void timer_uart_send_callback(void* arg){
}

void app_main() {

    gpio_config_t gpio_led = {
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(1ULL << LED_PIN) // Pino GPIO15
        };

    gpio_config(&gpio_led);

    gpio_config_t gpio_button = {
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(1ULL << PUSH_BUTTON_PIN) // Pino GPIO15
        };

    gpio_config(&gpio_button);

    // Configuração do ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_2,
    };
    adc_oneshot_new_unit(&init_config1, &adc2_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_0,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL, &config);
    

    ledc_timer_config_t ledc_timer_leds = {
        .duty_resolution = LEDC_TIMER_12_BIT,  
        .freq_hz = 1000,                   
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&ledc_timer_leds);

        ledc_channel_config_t ledc_channel_leds = {
            .channel = LEDC_CHANNEL_0,
            .duty = 0,
            .gpio_num = LED_PIN,                
            .intr_type = LEDC_INTR_DISABLE,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_sel = LEDC_TIMER_0
        };
        
        ledc_channel_config(&ledc_channel_leds);


    while(1){
        int adc_raw;
        adc_oneshot_read(adc2_handle, ADC_CHANNEL, &adc_raw);
        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, adc_raw);
        if(gpio_get_level(PUSH_BUTTON_PIN)) ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
        printf("adc: %d\n", adc_raw);
        vTaskDelay(pdMS_TO_TICKS(15));
    }
}