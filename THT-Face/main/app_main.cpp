#include "who_camera.h"
#include "who_human_face_recognition.hpp"
#include "who_lcd.h"
#include "who_button.h"
#include "event_logic.hpp"
#include "who_adc_button.h"

#include <esp_efuse.h>
#include "StomeClient/StomeClient.h"

static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
static QueueHandle_t xQueueKeyState = NULL;
static QueueHandle_t xQueueEventLogic = NULL;
// static button_adc_config_t buttons[4] = {{1, 2800, 3000}, {2, 2250, 2450}, {3, 300, 500}, {4, 850, 1050}};
 
#define GPIO_BOOT GPIO_NUM_0
uint32_t generate_unique_id(void);

extern "C" 
void app_main()
{
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueKeyState = xQueueCreate(1, sizeof(int *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));

    // uint32_t unique_id = generate_unique_id();
    // ESP_LOGI("MAIN", "Unique ID: 0x%08x", unique_id);


    // Continue with other initializations
    // register_button(GPIO_BOOT, xQueueKeyState);
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);
    // register_adc_button(buttons, 4, xQueueKeyState);
    // register_event(xQueueKeyState, xQueueEventLogic);
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame, false);
    register_lcd(xQueueLCDFrame, NULL, true);

    //------------wi-fi-----------
    nvs_flash_init();
    wifi_connection();
    //-------------wifi end---------------

}

uint32_t generate_unique_id(void)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    uint32_t unique_id = (mac[2] << 24) | (mac[3] << 16) | (mac[4] << 8) | mac[5];

    ESP_LOGI("mac", "MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
                 mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    return unique_id;
}
