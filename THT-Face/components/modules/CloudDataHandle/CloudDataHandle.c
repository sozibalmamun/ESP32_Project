#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"


#define TAG  "CLOUD"
uint8_t CPUBgflag;
extern  volatile uint8_t CmdEvent;
volatile TickType_t resetWifiTimeout;

static QueueHandle_t xQueueCloudI = NULL;
TaskHandle_t detectionFaceProcesingTaskHandler = NULL; // Handle for the stompSenderTask
TaskHandle_t stompSenderTaskHandler = NULL; // Handle for the stompSenderTask


static void detectionFaceProcesing(void *arg)
{
	const TickType_t xDelay = pdMS_TO_TICKS(500); // Run every 1 seconds

    while (true)
    {
        imageData_t *image = NULL;

        if (xQueueReceive(xQueueCloudI, &image, portMAX_DELAY))
        {
            // Check if image is valid
            if (image != NULL)
            {
                // Log the image details
                // ESP_LOGI(TAG, "Received image len: %d w: %d h: %d Name: %s id: %d", image->len, image->width, image->height, image->Name,image->id);

                // ESP_LOGE(TAG, "\ntest image storing.... ");
                save_face_data(image->id, image->Name, image->width, image->height, image->buf);
                // save_face_data(image->id, image->Name, 75, 69, &imageData);
                ESP_LOGE(TAG, "\ntest image saved ");
                vTaskDelay(100);
                // Free the image buffer if it was dynamically allocated
                heap_caps_free(image->buf);
                image->buf = NULL;
                CPUBgflag=0;
                
            }

            if (detectionFaceProcesingTaskHandler != NULL) {
                vTaskDelete(detectionFaceProcesingTaskHandler);   // Delete the task
                detectionFaceProcesingTaskHandler = NULL;         // Clear the handle to avoid dangling references
                ESP_LOGW("TAGSTOMP", "detectionFaceProcesingTaskHandler deleted");
            }

        }
		vTaskDelay(xDelay);

    }
}

 void fileProcessingTask(void){

    // Process attendance files
    if(networkStatus==STOMP_CONNECTED){

        if(CPUBgflag==0){
        if(CmdEvent!=IDLE_EVENT)eventFeedback();

        if(!pendingData()){

            // printf("\nno pending data");
            if (stompSenderTaskHandler != NULL) {
            vTaskDelete(stompSenderTaskHandler);   // Delete the task
            stompSenderTaskHandler = NULL;         // Clear the handle to avoid dangling references
            ESP_LOGW("TAGSTOMP", "detectionFaceProcesing deleted");
            }

        }else {

            printf("\npending data");
            if (stompSenderTaskHandler == NULL) initStompSender();
            process_and_send_faces(PUBLISH_TOPIC);
            vTaskDelay(500);
            process_attendance_files();
        } 

        }
    }else if(networkStatus<STOMP_CONNECTED && networkStatus>WIFI_CONNECTED){
        stomp_client_connect(); 
    }

    if(resetWifiTimeout > xTaskGetTickCount()+TIMEOUT_15_S && resetWifiTimeout < xTaskGetTickCount()+TIMEOUT_15_S+20){
        // reset wss
        // resetWSS();

    }else if(resetWifiTimeout>xTaskGetTickCount()+TIMEOUT_5_MIN){
        // reset wifi
        // resetWifi();

    }

    // Print heap status

    ESP_LOGI("HEAP", "Free heap: %d kb", heap_caps_get_free_size(MALLOC_CAP_8BIT)/1024);

}








void cloudHandel(const QueueHandle_t input )
{
    
    ESP_LOGI("cloudHandel", "detectionFaceProcesing creat");

    xQueueCloudI = input;
    xTaskCreatePinnedToCore(detectionFaceProcesing, TAG, 4 * 1024, NULL, 5, &detectionFaceProcesingTaskHandler, 0);

}

