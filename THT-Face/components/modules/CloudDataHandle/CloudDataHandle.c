#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"

// extern bool stompSend(char *buff, char *topic);

static const char *TAG = "CLOUD";

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
                ESP_LOGI(TAG, "Received image len: %d w: %d h: %d", image->len, image->width, image->height);

                for(int i=0 ; i<256;i++){

                    printf("%x",image->buf[i] );

                }


                
                // Send the image data to the cloud
                if(!imagesent(image->buf,image->len,image->height,image->width,"/app/cloud")){

                    ESP_LOGE(TAG, "Fail Sending image data.");

                }
                    ESP_LOGE(TAG, "\ntest image data ");


                // Free the image buffer if it was dynamically allocated
                heap_caps_free(image->buf);
                image->buf = NULL;
            }
            else
            {
                ESP_LOGE(TAG, "Received NULL image data.");
            }
        }
    }
}

// static void attendanceHandlerTask(void *arg)
// {
//     const TickType_t xDelay = pdMS_TO_TICKS(10000); // Run every 10 seconds

//     while (true)
//     {
//         // Process attendance files
//         process_attendance_files();

//         // Delay to allow periodic checking
//         vTaskDelay(xDelay);
//     }
// }


void cloudHandel(const QueueHandle_t input)
{
    xQueueCloudI = input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, NULL, 0);

}
// void logHandle(void){

//     xTaskCreatePinnedToCore(attendanceHandlerTask, "AttendanceTask", 4 * 1024, NULL, 5, NULL, 0);

// }
