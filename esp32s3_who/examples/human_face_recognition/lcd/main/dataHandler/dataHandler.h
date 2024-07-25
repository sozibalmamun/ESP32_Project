#ifndef DATAHANDLE_H
#define DATAHANDLE_H

#include <stdio.h>
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/event_groups.h"

#include "esp_log.h"

#include <string.h>          // For handling strings
#include "stdbool.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "lwip/err.h"        // Light weight IP packets error handling
#include "lwip/sys.h"        // System applications for lightweight IP apps
#include "lwip/netdb.h"



#ifdef __cplusplus
extern "C" {
#endif


const uint32_t crc16_table[16] = {
    0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
    0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
    0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
    0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c
  };

char tcpBuffer[2024]; // Adjust MAX_TRANSACTION_SIZE as needed



// parsing
void dataHandele(const char *rx_buffer);

uint16_t crc16(const char *buf, size_t len);
uint16_t getCRC16(uint16_t value);

uint16_t hex_to_uint16(const char* hex_str);


void u16tochar (uint16_t data, char* buff);
void u32tochar (uint32_t data, char* buff);
uint16_t chartou16 (char* data);
uint32_t chartou32 (char* data);
void toArray(uint16_t slotL, uint8_t *data_buffer);


void resizeBuffer(void);
void extractMessage(const char *buffer, char *output);







#ifdef __cplusplus
}
#endif



#endif
