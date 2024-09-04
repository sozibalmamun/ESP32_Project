#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"


static const char *TAG = "CLOUD";
uint8_t CPUBgflag;
extern  volatile uint8_t CmdEvent;

static QueueHandle_t xQueueCloudI = NULL;
TaskHandle_t detectionFaceProcesingTaskHandler=NULL;

TaskHandle_t cloudeTaskHandler = NULL;

static void cloudeHandlerTask(void *arg)
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
                ESP_LOGI(TAG, "Received image len: %d w: %d h: %d Name: %s id: %d", image->len, image->width, image->height, image->Name,image->id);

                // for(int i=0 ; i<256;i++){

                //     printf("%x",image->buf[i] );

                // }


                ESP_LOGE(TAG, "\ntest image storing.... ");

                save_face_data(image->id, image->Name, image->width, image->height, image->buf);
                // save_face_data(image->id, image->Name, 75, 69, &imageData);


                ESP_LOGE(TAG, "\ntest image saved ");

                // // Send the image data to the cloud
                // if(!imagesent(image->buf,image->len,image->height,image->width,image->Name ,image->id,"/app/cloud")){

                //     ESP_LOGE(TAG, "Fail Sending image data.");

                // }
                //     ESP_LOGE(TAG, "\ntest image data ");
                vTaskDelay(100);

                // Free the image buffer if it was dynamically allocated
                heap_caps_free(image->buf);
                image->buf = NULL;
                CPUBgflag=0;
            }
            else
            {
                ESP_LOGE(TAG, "Received NULL image data.");
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

static void attendanceHandlerTask(void *arg)
{
    const TickType_t xDelay = pdMS_TO_TICKS(500); // Run every 1 seconds

    while (true)
    {
        // Process attendance files
        if(networkStatus==STOMP_CONNECTED){

            if(CPUBgflag==0){

            if(CmdEvent!=IDLE_EVENT)eventFeedback();
            process_attendance_files();
            process_and_send_faces(PUBLISH_TOPIC);

            }
        }
		ESP_LOGI("HEAP", "Free heap: %d kb", heap_caps_get_free_size(MALLOC_CAP_8BIT)/1024);
        vTaskDelay(xDelay);

        // Delay to allow periodic checking
    }
}



void reconnect(){


        if(networkStatus==STOMP_CONNECTED){

            vTaskDelay(1000 / portTICK_PERIOD_MS); // Delay before retry

            if(CPUBgflag==0){

                if(!pendingData()){

                    // printf("\nno pending data");
                    if (cloudeTaskHandler != NULL) {
                    vTaskDelete(cloudeTaskHandler);   // Delete the task
                    cloudeTaskHandler = NULL;         // Clear the handle to avoid dangling references
                    ESP_LOGW("TAGSTOMP", "detectionFaceProcesing deleted");
                    }

                }else {

                    printf("\npending data");
                    if (cloudeTaskHandler == NULL) cloudHandel();

                } 

            }
        }else if(networkStatus<STOMP_CONNECTED && networkStatus>WIFI_CONNECTED){

        stomp_client_connect(); 
        }
}


void cloudHandel()
{

    xTaskCreatePinnedToCore(attendanceHandlerTask, "AttendanceTask", 4 * 1024, NULL,5, &cloudeTaskHandler, 0);

}



void facedataHandle(const QueueHandle_t input )
{
    ESP_LOGI("cloudHandel", "detectionFaceProcesing creat");
    xQueueCloudI = input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, detectionFaceProcesingTaskHandler, 0);


}

