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




#define TAG  "Time_Update"


void app_main(void)
{


    gpio_set_level((gpio_num_t)LCE_BL, 1);
    gpio_pad_select_gpio(LCE_BL);
    gpio_set_direction((gpio_num_t)LCE_BL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LCE_BL, 1);

    bluFiStart();
   // Initialize SNTP and get the time
    initialize_sntp();
    // Obtain and update the local time and date
    obtain_and_update_local_time();

    xTaskCreatePinnedToCore(time_tick_task, "time_tick_task", 2048, NULL, 5, NULL,1);
    xTaskCreatePinnedToCore(lvgl_task, "gui task", 1024 *6, NULL, 6, NULL, 0);

    PwmInt((gpio_num_t)LCE_BL);
    brightness(false);//sleep

}

