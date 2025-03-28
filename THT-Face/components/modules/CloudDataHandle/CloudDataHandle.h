#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "timeLib.h"
#include "globalScope.h"
#include "esp_camera.h"






#ifdef __cplusplus
extern "C"
{
#endif

extern uint8_t networkStatus;
extern bool lisence;

extern void wssReset(void);


typedef struct {
    uint8_t *buf;             /*!< Pointer to the copied pixel data */
    size_t len;               /*!< Length of the buffer in bytes */
    size_t width;             /*!< Width of the buffer in pixels */
    size_t height;            /*!< Height of the buffer in pixels */
    uint16_t id;
    char* Name;
} imageData_t;


bool imagesent(uint8_t *buff, uint16_t buffLen, uint8_t h, uint8_t w ,char* name,uint16_t id);
void ensureLogDelivery();
void facedataHandle(const QueueHandle_t input);


//---------------fs system-------------------------------------------------------------------------------------------------------

void  save_face_data(uint16_t person_id, const char* name, uint8_t image_width, uint8_t image_height, const uint8_t* image_data, const char* directory);
void write_log_attendance(uint16_t person_id,  uint8_t* timestamp);
void print_memory_status(void);
void format_fatfs(void);
void process_attendance_files(void);
bool delete_face_data(uint16_t person_id , const char * directory);
bool syncFace(const camera_fb_t *src, imageData_t **person);
bool pendingData();

//----------------event feedback--------------------------------------------------
void eventFeedback(void);



#ifdef __cplusplus
}
#endif