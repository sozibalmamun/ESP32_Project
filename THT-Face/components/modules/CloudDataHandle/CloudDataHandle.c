#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_heap_caps.h"


static const char *TAG = "CLOUD";
uint8_t CPUBgflag;
bool dataAvailable = false;

extern  volatile uint8_t CmdEvent;

static QueueHandle_t xQueueCloudI = NULL;
TaskHandle_t detectionFaceProcesingTaskHandler=NULL;

TaskHandle_t cloudeTaskHandler = NULL;

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

        // if (detectionFaceProcesingTaskHandler != NULL) {
        //     vTaskDelete(detectionFaceProcesingTaskHandler);   // Delete the task
        //     detectionFaceProcesingTaskHandler = NULL;         // Clear the handle to avoid dangling references
        //     ESP_LOGW("TAGSTOMP", "detectionFaceProcesingTaskHandler deleted");
        // }

        }
    }
}

static void attendanceHandlerTask(void *arg)
{
    const TickType_t xDelay = pdMS_TO_TICKS(500); // Run every 1 seconds

    while (true)
    {
        
        // Process attendance files
        // if(networkStatus==STOMP_CONNECTED){//WSS_CONNECTED
        if(networkStatus==WSS_CONNECTED){

            if(CPUBgflag==0){
            CPUBgflag=1;
            process_attendance_files();
            CPUBgflag=0;
            }
        }
        vTaskDelay(xDelay);
        // cloudeTaskHandler = NULL; 
        // vTaskDelete(NULL);  // NULL means this task deletes itself
    }
}


void reconnect(){


    // if(networkStatus==STOMP_CONNECTED){
    if(networkStatus==WSS_CONNECTED){


        if(CPUBgflag==0){

            if(CmdEvent!=IDLE_EVENT)eventFeedback();

            if(!pendingData()){

                dataAvailable = false;
                // printf("\nno pending data");
                // if (cloudeTaskHandler != NULL) {
                // vTaskDelete(cloudeTaskHandler);   // Delete the task
                // cloudeTaskHandler = NULL;         // Clear the handle to avoid dangling references
                // ESP_LOGW("TAGSTOMP", "AttendanceTask deleted");

                // }

            }else {

                vTaskDelay(1000 / portTICK_PERIOD_MS); // 
                printf("pending data\n");
                if (cloudeTaskHandler == NULL) cloudHandel();

            } 

        }
    }else if(networkStatus<STOMP_CONNECTED && networkStatus>WIFI_CONNECTED){

        // stomp_client_connect();

    }else if(networkStatus==WIFI_DISS){

        CPUBgflag=0;
        if(pendingData())dataAvailable = true;
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS); //Delay before retry
    if(CPUBgflag==0)print_memory_status();
    // ESP_LOGW("HEAP", "Free      : %dkb\n\n", heap_caps_get_free_size(MALLOC_CAP_8BIT)/1024);



}

void cloudHandel()
{

    // ESP_LOGI("cloudHandel", "AttendanceTask creat");

    xTaskCreatePinnedToCore(attendanceHandlerTask, "AttendanceTask", 4 * 1024, NULL,15, &cloudeTaskHandler, 1);

}


void facedataHandle(const QueueHandle_t input )
{
    // ESP_LOGW("cloudHandel", "detection FaceProcesing creat");
    xQueueCloudI = input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, detectionFaceProcesingTaskHandler, 1);


}

