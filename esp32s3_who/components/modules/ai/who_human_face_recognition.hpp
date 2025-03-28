#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

typedef enum
{
    IDLE = 0,
    DETECT,
    ENROLL,
    RECOGNIZE,
    DELETE,
} recognizer_state_t;

// CMD handle from server
#define ENROLING        0x01
#define ENROLED         0x02
#define DUPLICATE       0x03

#define DELETE_CMD      0X04
#define DELETED         0X05
#define ID_INVALID      0X06





void register_human_face_recognition(QueueHandle_t frame_i,
                                     QueueHandle_t event,
                                     QueueHandle_t result,
                                     QueueHandle_t frame_o = NULL,
                                     const bool camera_fb_return = false);
