#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_timer.h"
#include <stdio.h>

#define POT_PIN GPIO_NUM_11
#define PUSH_BUTTON_PIN GPIO_NUM_12
#define LED_PIN GPIO_NUM_13
#define ADC_CHANNEL ADC_CHANNEL_0

#define DEBOUNCE_MS 80
#define PRINT_DELAY 500
#define ADC_DELAY 15

static adc_oneshot_unit_handle_t adc2_handle;
static esp_timer_handle_t print_timer, debounce_timer = NULL;

static bool button_state = true;
static int adc_raw = 0;
static float tensao = 0;

void timer_print_callback(void* arg);
void debounce_timer_callback(void* arg);

void gpio_conf();
void adc_conf();
void pwm_conf();
void timer_conf();
void isr_conf();

void gpio_interrupt_handler(void* arg);


void task_ADC(QueueHandle_t queue);
void task_LED(QueueHandle_t queue);

void task_button(SemaphoreHandle_t semaphore);
void task_imu(SemaphoreHandle_t mutex);

QueueHandle_t queue_adc;


void app_main() {
    queue_adc = xQueueCreate(10, sizeof(uint16_t));

    gpio_conf();
    adc_conf();
    pwm_conf();
    timer_conf();
    isr_conf();

    while(1){
        vTaskDelay(pdMS_TO_TICKS(ADC_DELAY));
    }
}

void gpio_conf(){
    gpio_config_t gpio_led = {
        .mode=GPIO_MODE_OUTPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE,
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .pin_bit_mask =(1ULL << LED_PIN)
        };

    gpio_config(&gpio_led);

    gpio_config_t gpio_button = {
        .mode=GPIO_MODE_INPUT,
        .pull_up_en=GPIO_PULLUP_DISABLE, // Pullup externo
        .pull_down_en=GPIO_PULLDOWN_DISABLE,
        .intr_type=GPIO_INTR_NEGEDGE,
        .pin_bit_mask =(1ULL << PUSH_BUTTON_PIN)
        };

    gpio_config(&gpio_button);
}

void adc_conf(){
    // Configuração do ADC
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_2,
    };
    adc_oneshot_new_unit(&init_config1, &adc2_handle);

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_12,
    };
    adc_oneshot_config_channel(adc2_handle, ADC_CHANNEL, &config);
}

void pwm_conf(){
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
}

void timer_conf(){
    esp_timer_create_args_t timer_args_print = {
        .callback = timer_print_callback,
        .arg = NULL,
        .name = "timer_uart_send_periodic"
    };

    esp_timer_create_args_t timer_args_db = {
        .callback = debounce_timer_callback,
        .arg = NULL,
        .name = "debounce_timer"
    };
    esp_timer_create(&timer_args_db, &debounce_timer);
    esp_timer_create(&timer_args_print, &print_timer);

    esp_timer_start_periodic(print_timer, 500000);
}

void isr_conf(){
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PUSH_BUTTON_PIN, gpio_interrupt_handler, (void*)PUSH_BUTTON_PIN);
}

void timer_print_callback(void* arg){
    printf("\nValor: %d Tensão: %f", adc_raw, tensao);
    if(button_state){
        printf(" Estado: LIVE");
    } else {
        printf(" Estado: HOLD");
    }
}

void debounce_timer_callback(void* arg){
    if(!gpio_get_level(PUSH_BUTTON_PIN)){ 
        button_state = !button_state;
    }
    
    gpio_intr_enable(PUSH_BUTTON_PIN);
}

void IRAM_ATTR gpio_interrupt_handler(void* arg) {
    gpio_intr_disable(PUSH_BUTTON_PIN);
    
    esp_timer_stop(debounce_timer); 
    esp_timer_start_once(debounce_timer, DEBOUNCE_MS * 1000);
}

void task_ADC(QueueHandle_t queue){
    uint16_t adc_raw = 0;
    adc_oneshot_read(adc2_handle, ADC_CHANNEL, &adc_raw);
    tensao = adc_raw*3.3/4095;
    xQueueSend(queue, &adc_raw, 20);


}

void task_LED(QueueHandle_t queue, SemaphoreHandle_t semaphore){
    uint16_t adc_raw_receiver = 0;
    xQueueReceive(queue, adc_raw_receiver, 20*portTICK_PERIOD_MS);
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, adc_raw);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void task_button(SemaphoreHandle_t semaphore){

}
