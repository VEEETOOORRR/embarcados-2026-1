#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_timer.h"
#include <string.h>

#define LED_PIN GPIO_NUM_15

#define TX_PIN GPIO_NUM_4
#define RX_PIN GPIO_NUM_5

static esp_timer_handle_t uart_timer = NULL;

static bool led_state = false;

void timer_uart_send_callback(void* arg){
    const char* ligar = "ON\r\n";
    const char* desligar = "OFF\r\n";

    if(led_state){
        uart_write_bytes(UART_NUM_2, desligar, strlen(desligar));
        printf("String enviada: OFF\n");
    } else {
        uart_write_bytes(UART_NUM_2, ligar, strlen(ligar));
        printf("String enviada: ON\n");
    }
}

// Task para ler dados da UART
void uart_read_task(void* pvParameters) {
    char data[128];
    int len = 0;

    while(1) {
        len = uart_read_bytes(UART_NUM_2, data, 256, 20 / portTICK_PERIOD_MS);
        
        if(len > 0) {
            printf("String recebida: %s\n", data);
            data[len] = '\0';
            // Processa a mensagem
            if(strstr(data, "ON")) {
                led_state = true;
                gpio_set_level(LED_PIN, 1);
            }
            else if(strstr(data, "OFF")) {
                led_state = false;
                gpio_set_level(LED_PIN, 0);
            }
        }
    }
}

void app_main() {

    gpio_config_t gpio_led = {
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(1ULL << LED_PIN) // Pino GPIO15
        };

    gpio_config(&gpio_led);

    // Timer para envio de mensagem via UART2
    esp_timer_create_args_t timer_args_uart_send = {
        .callback = timer_uart_send_callback,
        .arg = NULL,
        .name = "timer_uart_send_periodic"
    };

    esp_timer_create(&timer_args_uart_send, &uart_timer);
    esp_timer_start_periodic(uart_timer, 2000000);

    // Configuração da UART2
    uart_config_t uart_config = {
    .baud_rate = 115200,
    .data_bits = UART_DATA_8_BITS,
    .parity = UART_PARITY_DISABLE,
    .stop_bits = UART_STOP_BITS_1,
    .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    .rx_flow_ctrl_thresh = 122,
    };

    uart_param_config(UART_NUM_2, &uart_config);
    uart_driver_install(UART_NUM_2, 1024 * 2, 0, 0, NULL, 0);
    uart_set_pin(UART_NUM_2,
                 TX_PIN,
                 RX_PIN,
                 UART_PIN_NO_CHANGE,
                 UART_PIN_NO_CHANGE);

    gpio_set_level(LED_PIN, 0);

    xTaskCreate(uart_read_task, "uart_read_task", 2048, NULL, 5, NULL);

    while(1){
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}