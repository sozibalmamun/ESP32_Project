#pragma once

#include "stdbool.h"
#include "stdint.h"
#include "stdio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#ifdef __cplusplus
extern "C"
{
#endif
typedef struct {
    uint8_t *buf;             /*!< Pointer to the copied pixel data */
    size_t len;               /*!< Length of the buffer in bytes */
    size_t width;             /*!< Width of the buffer in pixels */
    size_t height;            /*!< Height of the buffer in pixels */
} imageData_t;


bool imagesend( uint8_t* buff);
// bool stompSend(char * buff, char* topic);
bool imagesent(uint8_t *buff, uint16_t buffLen, char* topic);


void cloudHandel(const QueueHandle_t input );

#ifdef __cplusplus
}
#endif