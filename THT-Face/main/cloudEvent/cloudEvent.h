#ifndef ENROL_H
#define ENROL_H


#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "globalScope.h"


#include "timeLib.h"


// #define     PUBLISH_TOPIC       "/app/cloud"
extern uint8_t CPUBgflag;

extern const uint32_t crc16_table;
extern char tcpBuffer; // Adjust MAX_TRANSACTION_SIZE as needed
extern  uint8_t sleepEnable;
extern TickType_t sleepTimeOut; 
extern bool dspTimeFormet;

#ifdef __cplusplus
extern "C" {
#endif

void process_command(const char* buffer);
void enrolOngoing(void);
extern void eventFeedback(void);

extern uint16_t crc16(const char *buf, size_t len);
extern uint16_t getCRC16(uint16_t value);


extern uint16_t hex_to_uint16(const char* hex_str);

extern bool sendToWss(uint8_t *buff, size_t buffLen);
extern void u16tochar (uint16_t data, char* buff);
extern void u32tochar (uint32_t data, char* buff);
extern uint16_t chartou16 (char* data);
extern uint32_t chartou32 (char* data);
extern uint16_t chartoDeci(char* data);

extern void toArray(uint16_t slotL, uint8_t *data_buffer);

extern void format_fatfs(void);
extern void create_directories(void);

#ifdef __cplusplus
}
#endif



#endif
