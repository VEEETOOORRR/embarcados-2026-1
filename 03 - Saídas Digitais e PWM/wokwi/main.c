#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define DELAY_MS 10
#define MAX_DUTY 255

void led_duty(ledc_channel_t channel, int duty);
void buzzer_freq(ledc_timer_config_t *ledc_timer, int freq);

void app_main() {

    // Configuração do timer LEDC
    ledc_timer_config_t ledc_timer_leds = {
        .duty_resolution = LEDC_TIMER_8_BIT,  
        .freq_hz = 1000,                   
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };

        ledc_timer_config(&ledc_timer_leds);

    ledc_timer_config_t ledc_timer_buzzer = {
        .duty_resolution = LEDC_TIMER_10_BIT,
        .freq_hz = 500,            
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_1
    };

        ledc_timer_config(&ledc_timer_buzzer);

    for(int i = 0; i < 4; i++){
        ledc_channel_config_t ledc_channel_leds = {
            .channel = LEDC_CHANNEL_0 + i,
            .duty = 0,
            .gpio_num = 15 + i,                
            .intr_type = LEDC_INTR_DISABLE,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_sel = LEDC_TIMER_0
        };
        
        ledc_channel_config(&ledc_channel_leds);
    }

        ledc_channel_config_t ledc_channel_buzzer = {
            .channel = LEDC_CHANNEL_4,
            .duty = 0,                    // Duty cycle: 0-8191 (50% = 4095)
            .gpio_num = 21,                // Pino GPIO
            .intr_type = LEDC_INTR_DISABLE,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .timer_sel = LEDC_TIMER_1
        };
        
        ledc_channel_config(&ledc_channel_buzzer);

    while (1) {

        for(int i = 0; i<MAX_DUTY; i++){
            for(int l = 0; l<4; l++){
                led_duty(l, i);
            }
            vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

        }

        for(int i = MAX_DUTY; i>=0; i--){
            for(int l = 0; l<4; l++){
                led_duty(l, i);
            }
            vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        }

        for(int l = 0; l<4; l++){
            for(int i = 0; i<= MAX_DUTY; i++){
                led_duty(l, i);
                vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
            }
            for(int i = MAX_DUTY; i>=0; i--){
                led_duty(l, i);
                vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
            }
        }

        led_duty(4, 512);


        for(int freq = 500; freq <= 2000; freq += 10){
            buzzer_freq(&ledc_timer_buzzer, freq);
            vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        }

        for(int freq = 2000; freq >= 500; freq -= 10){ 
            buzzer_freq(&ledc_timer_buzzer, freq);
            vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
        }


        led_duty(4, 0);
    }
}

void led_duty(ledc_channel_t channel, int duty){
    ledc_set_duty(LEDC_LOW_SPEED_MODE, channel, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, channel);
}

void buzzer_freq(ledc_timer_config_t *ledc_timer, int freq){
    ledc_timer->freq_hz = freq;
    ledc_timer_config(ledc_timer);
}