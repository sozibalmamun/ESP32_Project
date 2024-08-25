#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"

// extern bool stompSend(char *buff, char *topic);

static const char *TAG = "CLOUD";
uint8_t CPUBgflag;
extern  volatile uint8_t CmdEnroll;

static QueueHandle_t xQueueCloudI = NULL;


static void cloudeHandlerTask(void *arg)
{
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
        }
    }
}

static void attendanceHandlerTask(void *arg)
{
    const TickType_t xDelay = pdMS_TO_TICKS(500); // Run every 1 seconds

    while (true)
    {
        // Process attendance files
        if(wifiStatus==2 && CPUBgflag==0){
            process_attendance_files();
            process_and_send_faces("/app/cloud"); 
            if(CmdEnroll!=IDLEENROL)eventFeedback();


        }
        vTaskDelay(xDelay);

        // Delay to allow periodic checking
    }
}


void cloudHandel(const QueueHandle_t input )
{

    xTaskCreatePinnedToCore(attendanceHandlerTask, "AttendanceTask", 4 * 1024, NULL,1, NULL, 0);
    xQueueCloudI = input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, NULL, 0);



}
// void logHandle(void){

//     xTaskCreatePinnedToCore(attendanceHandlerTask, "AttendanceTask", 4 * 1024, NULL, 5, NULL, 0);

// }
