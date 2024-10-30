#include "who_camera.h"
#include "who_human_face_recognition.hpp"
#include "who_lcd.h"
#include "who_button.h"
#include "event_logic.hpp"
#include "who_adc_button.h"

#include "Conectivity/blufi_example.h"

#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

//------------------------------
#include "driver/ledc.h"
#include "driver/mcpwm.h"
//-------------------------------

#include "timeLib.h"


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "CloudDataHandle.h"

//--------fatfs-------------------
#include "FATFS/fs.h"
//-------------------------------
#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp_clk.h"
#include "driver/gpio.h"


static const char *TAG = "app_main";

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
// static QueueHandle_t xQueueKeyState = NULL;
static QueueHandle_t xQueueEventLogic = NULL;

static QueueHandle_t xQueueCloud = NULL;


#define GPIO_WAKEUP_BUTTON GPIO_NUM_0
#define CAM_CONTROL GPIO_NUM_3
#define LCE_BL GPIO_NUM_14
#define RX GPIO_NUM_20


#define ESP_INTR_FLAG_DEFAULT 0
#define SLEEP_LCD 10
#define WAKE_LCD 70




#define MIN_BRIGHTNESS (8191)
#define BRIGHTNESS(x)  MIN_BRIGHTNESS-(((MIN_BRIGHTNESS/100)*x))

void PwmInt( ledc_channel_config_t *ledc_channel ,gpio_num_t pinNo );
void interruptInt(void);
void gpioInt(void);

extern "C" 
void app_main()
{
    ESP_LOGI(TAG, "Starting app_main");
    gpioInt();
    // Initialize Conectivity----------------------------
    bluFiStart();
    //--------------------------------------------------
    //-----------time int here-------------------------------------
    RtcInit();
    //--------------------------------------------------------------


    // esp_err_t ret;

    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));
    xQueueCloud = xQueueCreate(3, sizeof(int *));


    if (xQueueAIFrame == NULL || xQueueLCDFrame == NULL || xQueueEventLogic == NULL) {
        // ESP_LOGE(TAG, "Failed to create queues");
        esp_restart();
    }
    
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);//core 1    
    register_event(xQueueEventLogic);//core 1
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame,xQueueCloud ,false); //core 0+1
    register_lcd(xQueueLCDFrame, NULL, true);// core 1
    vTaskDelay(pdMS_TO_TICKS(10));

    //-------------------------
    // Initialize and mount FATFS
    if (init_fatfs()== ESP_OK) {
        
        print_memory_status();
        create_directories();
    }
    //-------------------------
    // Declare LEDC timer and channel configuration structs
    ledc_channel_config_t ledc_channel;
    // Initialize PWM using the PwmInt function
    PwmInt(&ledc_channel,(gpio_num_t)LCE_BL);


    // ledc_channel_config_t rx_channel;
    // PwmInt(&rx_channel,(gpio_num_t)RX);
    ESP_LOGI(TAG, "app_main finished");
    while(true){

        // Log or print the CPU frequency
        // int cpu_freq_mhz = esp_clk_cpu_freq() / 1000000;
        // ESP_LOGI("CPU Monitor", "Current CPU frequency: %d MHz", cpu_freq_mhz);


        reconnect();
        if(xTaskGetTickCount()-sleepTimeOut>3000 && /*xTaskGetTickCount()-sleepTimeOut< 3500 &&*/ sleepEnable == WAKEUP){

            sleepEnable=SLEEP;
            printf("\nsleepEnable");
            gpio_set_level((gpio_num_t)CAM_CONTROL, 1);
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, BRIGHTNESS(SLEEP_LCD));//8192
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
            // set_cpu_frequency_low();
            deinitBlufi();

        }

        if(sleepEnable == SLEEP){
            // enter_light_sleep();  // Enter light sleep mode
            if( gpio_get_level(GPIO_WAKEUP_BUTTON)==0){
                sleepTimeOut = xTaskGetTickCount();// imediate wake if display in sleep mode
                bluFiStart();
                sleepEnable = WAKEUP;
                gpio_set_level((gpio_num_t)CAM_CONTROL, 0);
                ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,BRIGHTNESS(WAKE_LCD));
                ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);    
            }
        }
    }
}



void PwmInt( ledc_channel_config_t *ledc_channel ,gpio_num_t pinNo ) {

    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,   // Low-speed mode
        .duty_resolution = LEDC_TIMER_13_BIT,      // 13-bit duty resolution
        .timer_num       = LEDC_TIMER_0,           // Timer 0
        .freq_hz         = 5000,                   // 5kHz PWM frequency
        .clk_cfg         = LEDC_AUTO_CLK           // Auto select clock
    };
    ledc_timer_config(&ledc_timer);

    // Configure LEDC channel for GPIO 14
    ledc_channel->gpio_num   = pinNo;                // GPIO pin for PWM output
    ledc_channel->speed_mode = LEDC_LOW_SPEED_MODE;        // Low-speed mode
    ledc_channel->channel    = LEDC_CHANNEL_0;             // Channel: 0
    ledc_channel->intr_type  = LEDC_INTR_DISABLE;          // Disable fade interrupt
    ledc_channel->timer_sel  = LEDC_TIMER_0;               // Select Timer 0
    ledc_channel->duty       = 0;                          // Initial duty cycle: 0
    ledc_channel->hpoint     = 0;                          // Hpoint: 0
    ledc_channel->flags.output_invert = 0;                 // Disable output inversion

    // Initialize the channel
    ledc_channel_config(ledc_channel);

    // Set initial duty cycle to 50% (for 13-bit resolution, this is 4096 out of 8192)
    ledc_set_duty(ledc_channel->speed_mode, ledc_channel->channel, BRIGHTNESS(SLEEP));
    ledc_update_duty(ledc_channel->speed_mode, ledc_channel->channel);

    // Optional: Use fading
    ledc_fade_func_install(0);  // Install the fade function
    ledc_set_fade_time_and_start(ledc_channel->speed_mode, ledc_channel->channel, BRIGHTNESS(SLEEP), 1000, LEDC_FADE_NO_WAIT);
    for (int duty = 8191; duty >= 0; duty -= 512) {
        ledc_set_duty(ledc_channel->speed_mode, ledc_channel->channel, duty);
        ledc_update_duty(ledc_channel->speed_mode, ledc_channel->channel);
        vTaskDelay(300 / portTICK_PERIOD_MS);     // Delay to see the dimming effect
    }
}


void IRAM_ATTR gpio_isr_handler(void* arg) {
    int gpio_num = (int)arg;
    // Handle the interrupt (e.g., toggle a flag or send an event)
    ESP_LOGI("GPIO_ISR", "Interrupt on GPIO %d", gpio_num);
}
void interruptInt(void){

    // Configure the GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_WAKEUP_BUTTON), // Pin mask
        .mode = GPIO_MODE_INPUT,                  // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,         // Enable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE,    // Disable pull-down resistor
        .intr_type = GPIO_INTR_POSEDGE            // Set interrupt on rising edge
    };
    gpio_config(&io_conf);

    // Install GPIO ISR service
    gpio_install_isr_service(ESP_INTR_FLAG_DEFAULT);

    // Attach interrupt handler for the specified pin
    gpio_isr_handler_add(GPIO_WAKEUP_BUTTON, gpio_isr_handler, (void*) GPIO_WAKEUP_BUTTON);


}


void configure_wakeup() {
    // Configure the button GPIO for input with pull-up, active-low
    gpio_pad_select_gpio(GPIO_WAKEUP_BUTTON);
    gpio_set_direction(GPIO_WAKEUP_BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_en(GPIO_WAKEUP_BUTTON);  // Enable pull-up for stable input

    // Enable external wakeup on GPIO_WAKEUP_BUTTON, active low
    esp_sleep_enable_ext0_wakeup(GPIO_WAKEUP_BUTTON, 0);  // Wake on falling edge (button press)
}
// Function to enter light sleep mode
void enter_light_sleep() {

    configure_wakeup();
    vTaskDelay(200);
    ESP_LOGI("Sleep", "Entering light sleep...");
    gpio_set_level((gpio_num_t)CAM_CONTROL, 1);  // Ensure peripherals are powered off or set to sleep state

    // Set the wake-up source to external (GPIO_BOOT, active low)
    // esp_sleep_enable_ext0_wakeup(GPIO_WAKEUP_BUTTON, 0);  // Wake when GPIO_BOOT goes low

    // Enter light sleep
    esp_light_sleep_start();

    ESP_LOGI("Sleep", "Woke up from light sleep!");
    sleepEnable = false;  // Disable sleep mode after waking up
}



void set_cpu_frequency_low() {
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = 240,   // Set maximum frequency to 80 MHz
        .min_freq_mhz = 80,   // Set minimum frequency to 40 MHz (or as low as desired)
    };
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret == ESP_OK) {
        ESP_LOGI("Frequency", "CPU frequency configured to 40-80 MHz");
    } else {
        ESP_LOGE("Frequency", "Failed to configure CPU frequency: %s", esp_err_to_name(ret));
    }
    vTaskDelay(pdMS_TO_TICKS(10));
}

void gpioInt(void){

   
    gpio_pad_select_gpio(CAM_CONTROL);
    gpio_set_direction((gpio_num_t)CAM_CONTROL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)CAM_CONTROL, 0);

    gpio_set_level((gpio_num_t)LCE_BL, 0);

    gpio_pad_select_gpio(LCE_BL);
    gpio_set_direction((gpio_num_t)LCE_BL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LCE_BL, 0);

    // gpio_pad_select_gpio(RX);
    // gpio_set_direction((gpio_num_t)RX, GPIO_MODE_OUTPUT);
    // gpio_set_level((gpio_num_t)RX, 0);

// 

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_WAKEUP_BUTTON), // Pin mask
        .mode = GPIO_MODE_INPUT,                  // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,         // Enable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE     // Disable pull-down resistor
    };
    gpio_config(&io_conf);

}