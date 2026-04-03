#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define DEBOUNCE_MS 20

static esp_timer_handle_t debounce_timer = NULL;
static uint8_t button_pressed = 0;

static uint8_t cont = 0;

// Callback do timer
void debounce_timer_callback(void* arg) {
    if(!gpio_get_level(button_pressed)){ // Botão ativo em nivel lógico baixo
        if(button_pressed == GPIO_NUM_10) {
            cont = cont + 1;
        } else if(button_pressed == GPIO_NUM_11) {
            cont = cont + 2;
        }
    }
    
    // Reabilita interrupt
    gpio_intr_enable(button_pressed);
}


// ISR handler do interrupt do GPIO
void IRAM_ATTR gpio_interrupt_handler(void* arg) {
    uint32_t gpio_num = (uint32_t)arg;
    button_pressed = gpio_num;
    
    // Desabilita interrupt durante debounce
    gpio_intr_disable(gpio_num);
    
    // Inicia timer para debounce
    esp_timer_start_once(debounce_timer, DEBOUNCE_MS*1000);
}

void app_main() {

    gpio_config_t gpio_buttons = {
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE,
        .intr_type=GPIO_INTR_NEGEDGE,
        .pin_bit_mask=(0b11 << 10) // Pinos GPIO10 e GPIO11
    };

    gpio_config_t gpio_leds = {
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(0b1111 << 15) // Pinos GPIO15, 16, 17, 18
    };

    gpio_config(&gpio_buttons);
    gpio_config(&gpio_leds);

    // Configura o timer para debounce
    esp_timer_create_args_t timer_args = {
        .callback = debounce_timer_callback,
        .arg = NULL,
        .name = "debounce_timer"
    };
    esp_timer_create(&timer_args, &debounce_timer);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(GPIO_NUM_10, gpio_interrupt_handler, (void*)GPIO_NUM_10);
    gpio_isr_handler_add(GPIO_NUM_11, gpio_interrupt_handler, (void*)GPIO_NUM_11);


    while(1){
        for(int i = 0; i < 4; i++){
            gpio_set_level(GPIO_NUM_15 + i, ((cont >> i) & 1));
        }
    }
}