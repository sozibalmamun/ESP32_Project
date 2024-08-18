#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "esp_camera.h"


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

typedef struct {
    uint8_t *buf;             /*!< Pointer to the copied pixel data */
    size_t len;               /*!< Length of the buffer in bytes */
    size_t width;             /*!< Width of the buffer in pixels */
    size_t height;            /*!< Height of the buffer in pixels */
} imageData_t;

// bool copy_rectangle(const camera_fb_t *src, int x_start, int x_end, int y_start, int y_end);

bool copy_rectangle(const camera_fb_t *src,imageData_t* dst, int x_start, int x_end, int y_start, int y_end);


void editImage(imageData_t *buff );


void register_human_face_recognition(QueueHandle_t frame_i,
                                     QueueHandle_t event,
                                     QueueHandle_t result,
                                     QueueHandle_t frame_o = NULL,
                                     const bool camera_fb_return = false);
