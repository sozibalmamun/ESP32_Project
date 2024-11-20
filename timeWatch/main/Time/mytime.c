


#include "mytime.h"

#define TAG "time"






// static EventGroupHandle_t s_wifi_event_group;
const int WIFI_CONNECTED_BIT = BIT0;
const int WIFI_FAIL_BIT = BIT1;


SemaphoreHandle_t xGuiSemaphore;
time_t custom_time;



LV_FONT_DECLARE(dseg_font_64);



// SNTP time synchronization callback
static void time_sync_notification_cb(struct timeval *tv) {
    ESP_LOGI(TAG, "Time synchronized");
}

// Initialize SNTP and get time
// Initialize SNTP and get time
 void initialize_sntp() {
    ESP_LOGI(TAG, "Initializing SNTP");

    // Set server names
    sntp_setservername(0, "pool.ntp.org");
    sntp_setservername(1, "time.google.com");
    sntp_setservername(2, "time.windows.com");

    // Set operating mode for both IPv4 and IPv6
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    if (!sntp_enabled()) {
        // ESP_LOGI(TAG, "Starting SNTP service");
        sntp_init();
    } else {
        // ESP_LOGI(TAG, "SNTP is already initialized");
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
        // ESP_LOGI(TAG, "Local time and date: %s", asctime(&timeinfo));

        // Convert to time_t and store in global variable
        custom_time = mktime(&timeinfo);
        if (custom_time == -1)
        {
            // ESP_LOGE("TIME", "Error setting custom time");
        }
        else
        {
            // ESP_LOGI("TIME", "Custom time set: %s", asctime(&timeinfo));
        }

    } else {
        // ESP_LOGE(TAG, "Failed to get valid time after multiple retries");
    }
}




void update_time(lv_obj_t *hour_label, lv_obj_t *min_label ,lv_obj_t * sec_label ) {

    struct tm timeinfo;
    char time_hour[4]; 
    char time_min[3]; 
    char sec_str[3];   

    // Convert custom_time to struct tm
    localtime_r(&custom_time, &timeinfo);
    // Format hour and minute in 12-hour format
    int hour = timeinfo.tm_hour % 12; // Convert to 12-hour format
    if (hour == 0) hour = 12; // Handle midnight

    snprintf(time_hour, sizeof(time_hour), "%02d",hour);
    snprintf(time_min, sizeof(time_min), "%02d",timeinfo.tm_min);
    snprintf(sec_str, sizeof(sec_str), "%02d",timeinfo.tm_sec);

    // Set the time label text
    lv_label_set_text(sec_label, sec_str);// sec
    lv_label_set_text(min_label, time_min); //min
    lv_label_set_text(hour_label, time_hour); //hour

}


void update_date(lv_obj_t *label)
{
   struct tm timeinfo;
   char date_str[15];

   // Convert custom_time to struct tm
   localtime_r(&custom_time, &timeinfo);

   // Format date as YYYY-MM-DD Day
   strftime(date_str, sizeof(date_str), "%02d-%02m-%y %a", &timeinfo);
   lv_label_set_text(label, date_str); // Set text to the date label
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