#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define BUTTON_PIN GPIO_NUM_10
#define LED_PIN GPIO_NUM_15

#define DEBOUNCE_MS 80
#define TURNOFF_TIME 10000

typedef enum {
    DESLIGADO,
    LIGADO
} estado_led_fsm;

static estado_led_fsm estado = DESLIGADO;


static esp_timer_handle_t debounce_timer, turnoff_timer = NULL;

// Callback do timer
void debounce_timer_callback(void* arg){
    if(!gpio_get_level(BUTTON_PIN)){ // Botão ativo em nivel lógico baixo
        if(estado == LIGADO){
            estado = DESLIGADO;
            esp_timer_stop(turnoff_timer);
        } 
        else if(estado == DESLIGADO){
            estado = LIGADO;
            esp_timer_start_once(turnoff_timer, TURNOFF_TIME*1000);
        }
    }
    // Reabilita interrupt
    gpio_intr_enable(BUTTON_PIN);
}

void auto_turn_off_timer_callback(void* arg){
    estado = DESLIGADO;
}


// ISR handler do interrupt do GPIO
void IRAM_ATTR gpio_interrupt_handler(void* arg) {
    uint32_t gpio_num = (uint32_t)arg;
    
    // Desabilita interrupt durante debounce
    gpio_intr_disable(gpio_num);
    
    // Inicia timer para debounce
    esp_timer_start_once(debounce_timer, DEBOUNCE_MS*1000);
}

void app_main() {

    gpio_config_t gpio_button = {
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_ENABLE,
        .intr_type=GPIO_INTR_NEGEDGE,
        .pin_bit_mask=(0b1 << BUTTON_PIN) // Pino GPIO10
    };

    gpio_config_t gpio_led = {
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(0b1 << LED_PIN) // Pino GPIO15
        };

    gpio_config(&gpio_button);
    gpio_config(&gpio_led);

    // Configura o timer para debounce
    esp_timer_create_args_t timer_args_db = {
        .callback = debounce_timer_callback,
        .arg = NULL,
        .name = "debounce_timer"
    };

    // Timer para desligamento automático
    esp_timer_create_args_t timer_args_turnoff = {
        .callback = auto_turn_off_timer_callback,
        .arg = NULL,
        .name = "turnoff_led_timer"
    };

    esp_timer_create(&timer_args_db, &debounce_timer);
    esp_timer_create(&timer_args_turnoff, &turnoff_timer);
    
    gpio_install_isr_service(0);
    gpio_isr_handler_add(BUTTON_PIN, gpio_interrupt_handler, (void*)GPIO_NUM_10);

    gpio_set_level(LED_PIN, 0);

    while(1){
        if(estado == DESLIGADO) gpio_set_level(LED_PIN, 0);
        else if(estado == LIGADO) gpio_set_level(LED_PIN, 1);
    }
}