#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#include "time.h"

#include "esp_http_client.h" // For HTTP requests
#include "cJSON.h"           // For JSON parsing

#include "esp_wifi.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"

#include <sys/time.h>
#include "esp_sntp.h"

#include "pwm.h"

#include "blufi_example.h"


#define WIFI_SSID "I"
#define WIFI_PASS "islam!@#"

static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_FAIL_BIT = BIT1;


SemaphoreHandle_t xGuiSemaphore;
static bool dot_on = true;  // State for toggling the colon
time_t custom_time;



LV_FONT_DECLARE(dseg_font_64);





static const char *TAG = "Time_Update";

// SNTP time synchronization callback
static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronized");
}

// Initialize SNTP and get time
// Initialize SNTP and get time
static void initialize_sntp() {
    ESP_LOGI(TAG, "Initializing SNTP");

    // Set server names
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.google.com");
    sntp_setservername(2, "time.windows.com");

    // Set operating mode for both IPv4 and IPv6
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    if (!sntp_enabled()) {
        ESP_LOGI(TAG, "Starting SNTP service");
        sntp_init();
    } else {
        ESP_LOGI(TAG, "SNTP is already initialized");
    }
}


// Function to obtain and update local time and date
void obtain_and_update_local_time() {
    time_t now;
    struct tm timeinfo;
    int retry = 0;
    const int retry_count = 15;  // Increased retry count

    // Set timezone to Bangladesh (UTC +6)
    setenv("TZ", "BDT-6", 1);
    tzset();

    // Wait for the time to be set via SNTP
    time(&now);
    localtime_r(&now, &timeinfo);

    // If time is valid, print the local time and date
    if (timeinfo.tm_year > (1970 - 1900)) {
        ESP_LOGI(TAG, "Local time and date: %s", asctime(&timeinfo));

        // Convert to time_t and store in global variable
        custom_time = mktime(&timeinfo);
        if (custom_time == -1)
        {
            ESP_LOGE("TIME", "Error setting custom time");
        }
        else
        {
            ESP_LOGI("TIME", "Custom time set: %s", asctime(&timeinfo));
        }

    } else {
        ESP_LOGE(TAG, "Failed to get valid time after multiple retries");
    }
}




static void lv_tick_task(void *arg) // LVGL tick task
{
   (void)arg;
   lv_tick_inc(10);
}



static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void *event_data) {
    if (event_base == WIFI_EVENT) {
        if (event_id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
    } else if (event_base == IP_EVENT) {
        if (event_id == IP_EVENT_STA_GOT_IP) {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI("WIFI", "Got IP:%s", ip4addr_ntoa(&event->ip_info.ip));
            xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        }
    }
}
static void wifi_init() {
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    s_wifi_event_group = xEventGroupCreate();

    // Initialize Wi-Fi
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    esp_wifi_init(&cfg);

    // Set Wi-Fi mode to STA (Station)
    esp_wifi_set_mode(WIFI_MODE_STA);

    // Configure Wi-Fi connection
    wifi_config_t wifi_config = {};
    strcpy((char *)wifi_config.sta.ssid, WIFI_SSID);
    strcpy((char *)wifi_config.sta.password, WIFI_PASS);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);

    // Register Wi-Fi event handler
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL);

    // Start Wi-Fi
    esp_wifi_start();
    esp_wifi_connect();
    
    // Wait for connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                pdFALSE, pdFALSE, portMAX_DELAY);
    
    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI("WIFI", "Successfully connected to %s", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGE("WIFI", "Failed to connect to %s", WIFI_SSID);
    }
}



static void update_time(lv_obj_t *time_label,lv_obj_t * sec_label ,lv_obj_t * ampm_label  ) {
    struct tm timeinfo;
    char time_str[10];  // Enough to hold "HH:MM AM/PM" plus null terminator
    char ampm_str[3];   // For "AM" or "PM"
    char sec_str[3];   // For "AM" or "PM"



    // Convert custom_time to struct tm
    localtime_r(&custom_time, &timeinfo);

    // Format hour and minute in 12-hour format
    int hour = timeinfo.tm_hour % 12; // Convert to 12-hour format
    if (hour == 0) hour = 12; // Handle midnight

    // Prepare the time string in the format "HH:MM"
    snprintf(time_str, sizeof(time_str), "%02d%s%02d", hour, dot_on?":":" "  , timeinfo.tm_min);

    snprintf(sec_str, sizeof(sec_str), "%02d",timeinfo.tm_sec);


    // Determine AM/PM
    if (timeinfo.tm_hour >= 12) {
        strcpy(ampm_str, "PM");
    } else {
        strcpy(ampm_str, "AM");
    }

    // Set the time label text
    lv_label_set_text(time_label, time_str);

    lv_label_set_text(sec_label, sec_str);

    lv_label_set_text(ampm_label, ampm_str);

    dot_on = !dot_on; // Toggle colon state
}






static void update_date(lv_obj_t *label)
{
   struct tm timeinfo;
   char date_str[64];

   // Convert custom_time to struct tm
   localtime_r(&custom_time, &timeinfo);

   // Format date as YYYY-MM-DD Day
   strftime(date_str, sizeof(date_str), "%d-%m-%y %a", &timeinfo);
   lv_label_set_text(label, date_str); // Set text to the date label
}

static void lvgl_task(void *arg) // GUI task
{
   xGuiSemaphore = xSemaphoreCreateMutex(); // Create GUI semaphore
   lv_init();                               // Initialize LVGL
   lvgl_driver_init();                      // Initialize display driver

   // Initialize display buffer
   static lv_disp_draw_buf_t draw_buf;
   lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_DMA);
   lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_DMA);
   lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * LV_VER_RES_MAX);

   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init(&disp_drv);
   disp_drv.draw_buf = &draw_buf;
   disp_drv.flush_cb = disp_driver_flush;
   disp_drv.hor_res = 240; // Horizontal resolution
   disp_drv.ver_res = 320; // Vertical resolution
   lv_disp_drv_register(&disp_drv);

   // Create a periodic timer for LVGL ticks
   const esp_timer_create_args_t periodic_timer_args = {
       .callback = &lv_tick_task,
       .name = "periodic_gui"};
   esp_timer_handle_t periodic_timer;
   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
   ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));

    // Initialize the screen UI
   // Set black background color for the screen
   lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);

//===============================================


    // Create a label for time
    lv_obj_t * time_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(time_label, lv_color_white(), 0); // White text
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0); // Set large font for time  lv_font_unscii_8
    lv_label_set_text(time_label, "12:34");

    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, -115); // Align the time at the center


    // Create a label for time
    lv_obj_t * time_sec_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(time_sec_label, lv_color_white(), 0); // White text
    lv_obj_set_style_text_font(time_sec_label, &lv_font_montserrat_16, 0); // Set large font for time  lv_font_unscii_8
    lv_label_set_text(time_sec_label, "12");

    lv_obj_align(time_sec_label, LV_ALIGN_CENTER, 78, -104); // Align the time at the center


//    lv_style_t segment;
//    lv_style_init(&segment);
//    lv_style_set_text_font(&segment,&tft7segment);
//    lv_obj_add_style(time_label, &segment, 0);



//========================================================
    // Create a label for the AM/PM indicator
    lv_obj_t * am_pm_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(am_pm_label, lv_color_white(), 0); // White text
    lv_obj_set_style_text_font(am_pm_label, &lv_font_montserrat_12, 0); // Set large font for time  lv_font_unscii_8
    lv_label_set_text(am_pm_label, "PM");
    lv_obj_align(am_pm_label, LV_ALIGN_CENTER, 80, -126); // Align the AM/PM beside the time

    // Create a label for the date
    lv_obj_t * date_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(date_label, lv_color_white(), 0); // White text
    lv_label_set_text(date_label, "24-09-2024");
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, -60); // Align date below the time


    // Create a label for the weather (optional)
    // lv_obj_t * weather_label = lv_label_create(main_screen);
    // lv_obj_set_style_text_color(weather_label, lv_color_white(), 0); // White text

    // lv_label_set_text(weather_label, "25°C ☀️");
    // lv_obj_align(weather_label, LV_ALIGN_CENTER, 0, 70); // Align weather info below the date


   while (1)
   {
      vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second

      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {

        update_time(time_label,   time_sec_label  ,am_pm_label);
        update_date(date_label);      // Update the date label

        obtain_and_update_local_time() ;
        lv_timer_handler();           // Handle LVGL tasks
        xSemaphoreGive(xGuiSemaphore);// Release semaphore
      }
   }
}

void print_custom_time()
{
    struct tm timeinfo;
    localtime_r(&custom_time, &timeinfo);
    // ESP_LOGI("TIME", "Current time: %02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
}

// Task to simulate time ticking (1-second updates)
void time_tick_task(void *arg)
{
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));  // 1 second delay
        custom_time += 1;  // Increment custom time by 1 second
        // print_custom_time();  // Print updated time
    }
}
void app_main(void)
{


    gpio_set_level((gpio_num_t)LCE_BL, 1);
    gpio_pad_select_gpio(LCE_BL);
    gpio_set_direction((gpio_num_t)LCE_BL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LCE_BL, 1);

//    esp_err_t ret = nvs_flash_init();
//    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
//       ESP_ERROR_CHECK(nvs_flash_erase());
//       ret = nvs_flash_init();
//    }
//    ESP_ERROR_CHECK(ret);

    bluFiStart();


    // Initialize Wi-Fi
    // wifi_init();
   // Initialize SNTP and get the time
    initialize_sntp();

    // Obtain and update the local time and date
    obtain_and_update_local_time();

    xTaskCreate(time_tick_task, "time_tick_task", 2048, NULL, 5, NULL);
    xTaskCreatePinnedToCore(lvgl_task, "gui task", 1024 * 4, NULL, 1, NULL, 0);

    PwmInt((gpio_num_t)LCE_BL);
    brightness(false);//sleep
}

