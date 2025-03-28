#ifndef ENROL_H
#define ENROL_H


#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include <stdio.h>
#include "esp_wifi.h"
#include "esp_log.h"





#define     PUBLISH_TOPIC       "/app/cloud"

extern const uint32_t crc16_table;
extern char tcpBuffer; // Adjust MAX_TRANSACTION_SIZE as needed


#ifdef __cplusplus
extern "C" {
#endif

void process_command(const char* buffer);
void enrolOngoing(void);
void idDeletingOngoing(void);

extern uint16_t crc16(const char *buf, size_t len);
extern uint16_t getCRC16(uint16_t value);


extern uint16_t hex_to_uint16(const char* hex_str);

extern bool stompSend(char * buff, char* topic);

extern void u16tochar (uint16_t data, char* buff);
extern void u32tochar (uint32_t data, char* buff);
extern uint16_t chartou16 (char* data);
extern uint32_t chartou32 (char* data);
extern void toArray(uint16_t slotL, uint8_t *data_buffer);


#ifdef __cplusplus
}
#endif



#endif
