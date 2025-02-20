#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"


#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"


#include "pwm.h"
#include "Connectivity/blufi_example.h"
#include "Time/mytime.h"
#include "mylvgl/mylvgl.h"




TaskHandle_t timeTask = NULL;
TaskHandle_t lvglTask = NULL;
extern uint8_t wifiStatus;

void app_main(void)
{
    //LCD Backlight
    gpio_set_level((gpio_num_t)LCE_BL, 0);
    gpio_pad_select_gpio(LCE_BL);
    gpio_set_direction((gpio_num_t)LCE_BL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LCE_BL, 0);
    
    //BUZZER
    // gpio_pad_select_gpio(GPIO_NUM_8);
    // gpio_set_direction((gpio_num_t)GPIO_NUM_8, GPIO_MODE_OUTPUT);
    // gpio_set_level((gpio_num_t)GPIO_NUM_8, 0);


    bluFiStart();
   // Initialize SNTP and get the time
    initialize_sntp();
    // Obtain and update the local time and date
    if(wifiStatus)obtain_and_update_local_time();
    vTaskDelay(100);

    xTaskCreatePinnedToCore(time_tick_task, "time_tick_task", 2048, NULL, 5, &timeTask,1);
    xTaskCreatePinnedToCore(lvgl_task, "gui task", 1024 *6, NULL, 6, &lvglTask, 0);

    PwmInt((gpio_num_t)LCE_BL);
    brightness(false);//sleep

    // buzzer_init();

    // while (1) {

    // // Nokia ringtone frequencies and delays in ms
    // uint16_t nokia_tone[] = {
    //     440, 200,  // A4, 200ms
    //     440, 200,  // A4, 200ms
    //     440, 200,  // A4, 200ms
    //     440, 200,  // A4, 200ms
    //     349, 200,  // F4, 200ms
    //     523, 200,  // C5, 200ms
    //     440, 200,  // A4, 200ms
    //     349, 200,  // F4, 200ms
    //     523, 200,  // C5, 200ms
    //     440, 200,  // A4, 200ms
    //     349, 200,  // F4, 200ms
    //     523, 200,  // C5, 200ms
    //     440, 200,  // A4, 200ms
    //     349, 200,  // F4, 200ms
    //     523, 200,  // C5, 200ms
    //     440, 200   // A4, 200ms
    // };

    // play_music(nokia_tone, sizeof(nokia_tone) / sizeof(nokia_tone[0]));
    // vTaskDelay(1000 / portTICK_PERIOD_MS); // Convert to ms


    // }

}







