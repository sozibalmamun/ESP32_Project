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
    RECOGNIZE,
    DELETE,
} recognizer_state_t;

// CMD handle from server
#define IDLEENROL       0x00
#define ENROLING        0x01
#define ENROLED         0x02
#define DUPLICATE       0x03

#define DELETE_CMD      0X04
#define DELETED         0X05
#define ID_INVALID      0X06


// bool copy_rectangle(const camera_fb_t *src, int x_start, int x_end, int y_start, int y_end);

bool copy_rectangle(const camera_fb_t *src,imageData_t* dst, int x_start, int x_end, int y_start, int y_end);


void editImage(imageData_t *buff );


void register_human_face_recognition(const QueueHandle_t frame_i,
                                     const QueueHandle_t event,
                                     const QueueHandle_t result,
                                     const QueueHandle_t frame_o,
                                     
                                     const QueueHandle_t cloud,

                                     const bool camera_fb_return);
