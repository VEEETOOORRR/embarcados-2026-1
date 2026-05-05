#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"


#define POT_PIN GPIO_NUM_11
#define PUSH_BUTTON_PIN GPIO_NUM_12
#define LED_PIN GPIO_NUM_13

static esp_timer_handle_t uart_timer = NULL;

static bool led_state = false;

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
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(1ULL << PUSH_BUTTON_PIN) // Pino GPIO15
        };

    gpio_config(&gpio_button);

    

    ledc_timer_config_t ledc_timer_leds = {
        .duty_resolution = LEDC_TIMER_8_BIT,  
        .freq_hz = 1000,                   
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK
    };

        ledc_timer_config(&ledc_timer_leds);



    // Timer para envio de mensagem via UART2
    esp_timer_create_args_t timer_args_uart_send = {
        .callback = timer_uart_send_callback,
        .arg = NULL,
        .name = "timer_uart_send_periodic"
    };

    esp_timer_create(&timer_args_uart_send, &uart_timer);
    esp_timer_start_periodic(uart_timer, 2000000);

    gpio_set_level(LED_PIN, 0);

    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}