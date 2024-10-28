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



#define SLEEP 10
#define WAKE 70




#define MIN_BRIGHTNESS (8191)
#define BRIGHTNESS(x)  MIN_BRIGHTNESS-(((MIN_BRIGHTNESS/100)*x))

void PwmInt( ledc_channel_config_t *ledc_channel ,gpio_num_t pinNo );



void configure_wakeup() {
    // Configure the button GPIO for input with pull-up, active-low
    gpio_pad_select_gpio(GPIO_WAKEUP_BUTTON);
    gpio_set_direction(GPIO_WAKEUP_BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_en(GPIO_WAKEUP_BUTTON);  // Enable pull-up for stable input



    // gpio_pullup_en(GPIO_WAKEUP_BUTTON);
    // esp_sleep_enable_ext0_wakeup(GPIO_WAKEUP_BUTTON, 0); // Wake up when GPIO is low

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

extern "C" 
void app_main()
{


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

    //-----------time int here-------------------------------------
    RtcInit();
    vTaskDelay(100); // Wait for 1 second
    //--------------------------------------------------------------


    // esp_err_t ret;

    // ESP_LOGI(TAG, "Starting app_main");

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


    // Initialize Conectivity----------------------------
    bluFiStart();
    //--------------------------------------------------


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

        reconnect();
        if(xTaskGetTickCount()-sleepTimeOut>3000 && xTaskGetTickCount()-sleepTimeOut< 3500){

            sleepEnable=SLEEP;
            printf("\nsleepEnable");

        }

        if(sleepEnable){

            gpio_set_level((gpio_num_t)CAM_CONTROL, 1);
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, BRIGHTNESS(SLEEP));//8192
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

            // enter_light_sleep();  // Enter light sleep mode
            // ledc_set_duty(rx_channel.speed_mode, rx_channel.channel, 0);
            // ledc_update_duty(rx_channel.speed_mode, rx_channel.channel);

        }else{
            gpio_set_level((gpio_num_t)CAM_CONTROL, 0);
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,BRIGHTNESS(WAKE));
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

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