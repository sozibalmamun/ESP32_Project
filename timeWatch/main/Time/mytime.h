
#ifndef MYTIME_H
#define MYTIME_H

#include <stdio.h>
#include "time.h"
#include "esp_http_client.h" 
#include "cJSON.h"
#include <sys/time.h>
#include "esp_sntp.h"



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


// #include "pwm.h"
#include "Connectivity/blufi_example.h"
// #include "Time/mytime.h"



 void initialize_sntp();
 void obtain_and_update_local_time();
 void update_time(lv_obj_t *hour_label, lv_obj_t *min_label ,lv_obj_t * sec_label );
 void update_date(lv_obj_t *label);
 void time_tick_task(void *arg);


 #endif