#include "CloudDataHandle.h"
#include "esp_system.h"
#include "esp_log.h"

extern bool stompSend(char *buff, char *topic);

static const char *TAG = "CLOUD";

static QueueHandle_t xQueueCloudI = NULL;

// bool imagesend(uint8_t *buff, size_t len, const char *topic)
// {
//     // Ensure the buffer is not NULL
//     if (buff == NULL)
//     {
//         ESP_LOGE(TAG, "Buffer is NULL, cannot send image.");
//         return false;
//     }

//     // Attempt to send the image buffer using stompSend
//     if (stompSend((char *)buff, (char *)topic))
//     {
//         ESP_LOGI(TAG, "Image successfully sent to topic: %s", topic);
//         return true;
//     }
//     else
//     {
//         ESP_LOGE(TAG, "Failed to send image to topic: %s", topic);
//         return false;
//     }
// }

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

                for(int i=0 ; i<128;i++){

                    printf("%x",image->buf[i] );

                }
                // Send the image data to the cloud
                if(!imagesent(image->buf,image->len, "/app/cloud")){

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

void cloudHandel(const QueueHandle_t input)
{
    xQueueCloudI = input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, NULL, 0);
}
