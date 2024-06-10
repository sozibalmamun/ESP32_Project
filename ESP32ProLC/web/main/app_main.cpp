#include "who_camera.h"
#include "who_human_face_detection.hpp"
#include "who_human_face_recognition.hpp"
#include "app_wifi.h"
#include "app_mdns.h"
#include "who_lcd.h"
#include "App_OEMType.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_bt.h"
#include "string.h"
#include "App_OEMType.cpp"
#include "app_httpd.hpp"

#define CamerUDP		0
static QueueHandle_t xQueueAIFrame = NULL;
static QueueHandle_t xQueueLCDFrame = NULL;
extern "C" void app_main()
{
	vTaskDelay(1000/portTICK_PERIOD_MS);
	printf("delay.");
	IntialParser();
	//App_Ble_Start();
	vTaskDelay(1000/portTICK_PERIOD_MS);
	printf(".\r\n");
#if(CamerUDP==0)
	//LVGL_app_main();
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
	register_camera_LVGL(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);
	register_human_face_recognition(xQueueAIFrame, NULL, NULL, xQueueLCDFrame,false);
	DisplayFreeMemory((char *)"face_detection start");
	register_lcd(xQueueLCDFrame, NULL, true);
	//register_lcd_sang(xQueueLCDFrame, NULL, true);
	DisplayFreeMemory((char *)"Current");
#else
   // xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    register_camera(PIXFORMAT_JPEG, FRAMESIZE_XGA, 4, NULL);
   // register_httpd(xQueueAIFrame, NULL, true);
#endif
    DisplayFreeMemory("MemDisplay");
	App_Ble_Start();

	
}
