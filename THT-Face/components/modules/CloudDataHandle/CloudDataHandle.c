#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"


static const char *TAG = "CLOUD";
uint8_t CPUBgflag;
bool dataAvailable = false;

extern  volatile uint8_t CmdEvent;
extern volatile TickType_t sleepTimeOut; 


static QueueHandle_t xQueueCloudI = NULL;
TaskHandle_t detectionFaceProcesingTaskHandler=NULL;

void ensureLogDelivery(){

    if(networkStatus==WSS_CONNECTED){

        if(CPUBgflag==0){

            if(CmdEvent!=IDLE_EVENT)eventFeedback();
            vTaskDelay(100 / portTICK_PERIOD_MS);
            
            if(!pendingData()){

                dataAvailable = false;

            }else {
                // printf("pending data\n");
                if(networkStatus==WSS_CONNECTED){

                    if(CPUBgflag == 0){
                        CPUBgflag=1;
                        if(lisence)process_attendance_files();
                        else ESP_LOGE(TAG, "Lisence not found");
                        CPUBgflag = 0;
                    }
                }
            }

        }
    } else if(networkStatus==WIFI_DISS){

        CPUBgflag=0;
        if(pendingData())dataAvailable = true;

    }else if(networkStatus==WIFI_CONNECTED){


       if( xTaskGetTickCount()-sleepTimeOut>TIMEOUT_10_S && xTaskGetTickCount()-sleepTimeOut<TIMEOUT_12_S ){
            wssReset();
       }
       
    }
    if(CPUBgflag==0){
        vTaskDelay(500 / portTICK_PERIOD_MS); //Delay before retry
        print_memory_status();
        ESP_LOGE("HEAP", "Free      : %dkb\n\n", heap_caps_get_free_size(MALLOC_CAP_8BIT)/1024);
    }

}

static void cloudeHandlerTask(void *arg)
{
	// const TickType_t xDelay = pdMS_TO_TICKS(500); // Run every 1 seconds

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


                save_face_data(image->id, image->Name, image->width, image->height, image->buf,FACE_DIRECTORY);

                ESP_LOGE(TAG, "image saved ");
                vTaskDelay(100);

                // Free the image buffer if it was dynamically allocated
                heap_caps_free(image->buf);

                image->buf = NULL;
                CPUBgflag=0;
            }
            // ESP_LOGW(TAG, "cloudeHandlerTask is being deleted");
            // Delete the task
            detectionFaceProcesingTaskHandler = NULL;         // Clear the handle to avoid dangling references
            vTaskDelete(NULL);  // NULL means this task deletes itself
        }
    }
}
void facedataHandle(const QueueHandle_t input )
{
    xQueueCloudI = input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, &detectionFaceProcesingTaskHandler, 1);

}

