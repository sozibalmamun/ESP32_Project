#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"


#include "esp_camera.h"
#include "CloudDataHandle.h"
#include "gpioControl.h"


#define   bytes_per_pixel 2

typedef enum
{
    IDLE = 0,
    DETECT,
    ENROLL,
    ENROLING,
    RECOGNIZE,
    DELETE,
    SYNCING,
    SYNC,
    DUMP,

} recognizer_state_t;


extern uint8_t CPUBgflag;
extern union shiftResistorBitfild shiftOutData;
extern SemaphoreHandle_t musicShiftSemaphore;





bool copy_rectangle(const camera_fb_t *src, imageData_t **dst, int16_t x_start, int16_t x_end, int16_t y_start, int16_t y_end);


void register_human_face_recognition(const QueueHandle_t frame_i,
                                     const QueueHandle_t event,
                                     const QueueHandle_t result,
                                     const QueueHandle_t frame_o,
                                     
                                     const QueueHandle_t cloud,

                                     const bool camera_fb_return);
