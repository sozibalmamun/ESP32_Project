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


#include "timeLib.h"


#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#include "CloudDataHandle.h"

//--------fatfs-------------------
#include "FATFS/fs.h"
//-------------------------------

static const char *TAG = "app_main";

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
// static QueueHandle_t xQueueKeyState = NULL;
static QueueHandle_t xQueueEventLogic = NULL;

static QueueHandle_t xQueueCloud = NULL;


#define GPIO_BOOT GPIO_NUM_0


extern "C" 
void app_main()
{

    esp_err_t ret;

    // ESP_LOGI(TAG, "Starting app_main");

    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    // xQueueKeyState = xQueueCreate(1, sizeof(int *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));

    xQueueCloud = xQueueCreate(3, sizeof(int *));



    if (xQueueAIFrame == NULL || xQueueLCDFrame == NULL ||/* xQueueKeyState == NULL ||*/ xQueueEventLogic == NULL) {
        // ESP_LOGE(TAG, "Failed to create queues");
        esp_restart();
    }
    
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);//core 1    
    register_event(xQueueEventLogic);//core 1
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame,xQueueCloud ,false); //core 0+1

    // cloudHandel();// core 0

    register_lcd(xQueueLCDFrame, NULL, true);// core 1
    vTaskDelay(pdMS_TO_TICKS(10));


    // Initialize Conectivity----------------------------

    bluFiStart();
    //--------------------------------------------------

    //-----------time int here-------------------------------------
    time_library_time_t initial_time = {2024, 1, 1, 22, 58, 0};//     year, month, day, hour, minute, second;
    time_library_init(&initial_time);
    //--------------------------------------------------------------

    //-------------------------
    // Initialize and mount FATFS

    if (init_fatfs()== ESP_OK) {
        // Create directories
        // format_fatfs();
        // delete_all_directories("/storage");
        print_memory_status();
        create_directories();



    }
    //-------------------------



    ESP_LOGI(TAG, "app_main finished");

    while(true){


        reconnect();
        if(xTaskGetTickCount()-sleepTimeOut>6000 && xTaskGetTickCount()-sleepTimeOut< 6100){

            sleepEnable=SLEEP;
            printf("\nsleepEnable");

        }





















    }


}
