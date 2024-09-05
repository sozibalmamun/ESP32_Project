#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "esp_camera.h"
#include "CloudDataHandle.h"


typedef enum
{
    IDLE = 0,
    DETECT,
    ENROLL,
    ENROLING,
    RECOGNIZE,
    DELETE,
    SYNCING,

} recognizer_state_t;


extern uint8_t CPUBgflag;

bool copy_rectangle(const camera_fb_t *src,imageData_t** dst, int x_start, int x_end, int y_start, int y_end);

void editImage(imageData_t *buff );

void register_human_face_recognition(const QueueHandle_t frame_i,
                                     const QueueHandle_t event,
                                     const QueueHandle_t result,
                                     const QueueHandle_t frame_o,
                                     
                                     const QueueHandle_t cloud,

                                     const bool camera_fb_return);
