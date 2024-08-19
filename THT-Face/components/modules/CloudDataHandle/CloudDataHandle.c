#include "CloudDataHandle.h"
#include "esp_system.h"

extern bool stompSend(char * buff, char* topic);
static const char *TAG = "CLOUD";

static QueueHandle_t xQueueCloudI = NULL;


bool imagesend( uint8_t* buff){

// stompSend((char *) buff, "/app/cloud");

return 1;
}




static void cloudeHandlerTask(void *arg)
{

    while (true)
    {
        imageData_t image;
        if(xQueueReceive(xQueueCloudI, &image, portMAX_DELAY)){

            printf("image len:%d w: %d", image.len, image.width);
        }

    }
}
void cloudHandel(const QueueHandle_t input ){

    xQueueCloudI=input;
    xTaskCreatePinnedToCore(cloudeHandlerTask, TAG, 4 * 1024, NULL, 5, NULL, 0);
    
}
