#include "who_camera.h"
#include "who_human_face_recognition.hpp"
#include "who_lcd.h"
#include "who_button.h"
#include "event_logic.hpp"
#include "who_adc_button.h"
#include "StomeClient/StomeClient.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"

static const char *TAG = "app_main";

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
static QueueHandle_t xQueueKeyState = NULL;
static QueueHandle_t xQueueEventLogic = NULL;

#define GPIO_BOOT GPIO_NUM_0

extern "C" 
void app_main()
{
    esp_err_t ret;

    ESP_LOGI(TAG, "Starting app_main");

    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueKeyState = xQueueCreate(1, sizeof(int *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));

    if (xQueueAIFrame == NULL || xQueueLCDFrame == NULL || xQueueKeyState == NULL || xQueueEventLogic == NULL) {
        ESP_LOGE(TAG, "Failed to create queues");
        esp_restart();
    }


    // Continue with other initializations
    
    // register_button(GPIO_BOOT, xQueueKeyState);
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);
     // register_adc_button(buttons, 4, xQueueKeyState);
    // register_event(xQueueKeyState, xQueueEventLogic);
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame, false);
    register_lcd(xQueueLCDFrame, NULL, true);

    vTaskDelay(pdMS_TO_TICKS(10));

    // Initialize NVS
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_LOGW(TAG, "NVS partition was truncated, erasing...");
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    vTaskDelay(pdMS_TO_TICKS(10));

    // Initialize Wi-Fi
    wifi_connection();

    ESP_LOGI(TAG, "app_main finished");
}
