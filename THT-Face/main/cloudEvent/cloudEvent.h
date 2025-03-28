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
#include "CloudDataHandle.h"
#include "timeLib.h"
// #include "NVS.h"




const DATA_FLASH uint16_t crc16_table[256] = 
{
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7,
    0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6,
    0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485,
    0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4,
    0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc69c,
    0x48a4, 0x58b5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a22,
    0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
    0x6ca6, 0x7c87, 0x4404, 0x5405, 0x3403, 0x2402, 0x1401, 0x0400,
    0xd95a, 0xc95b, 0xfca1, 0xeca2, 0x8cdd, 0x9cd9, 0xbcc3, 0xacc2,
    0x5c8f, 0x4c9e, 0x7ea7, 0x6ea6, 0x2a12, 0x3a13, 0x0a50, 0x1a51,
    0xf8bf, 0xe8ae, 0x9c9d, 0x8cad, 0xbab5, 0xaab4, 0x6e8f, 0x7e9e,
    0x4f4c, 0x5f5d, 0x2d23, 0x3d32, 0x1b11, 0x0b10, 0x86d3, 0x96e2,
    0xe6df, 0xf6cf, 0x4ed
  };

extern  uint8_t sleepEnable;
extern volatile TickType_t sleepTimeOut; 
extern bool dspTimeFormet;
extern uint8_t CPUBgflag;
extern uint8_t config;

#ifdef __cplusplus
extern "C" {
#endif


extern bool delete_face_data(uint16_t person_id , const char * directory);
extern void eventFeedback(void);
extern bool sendToWss(uint8_t *buff, size_t buffLen);
extern void format_fatfs(void);
extern void create_directories(void);


extern void save_time_format(bool is_12_hour);
extern void saveSubscription(bool enable);

extern void welcomeMusic(bool enable);
extern bool checkMusicEnable();
// ble
extern void blufi_security_deinit(void);
extern void blufiAddStart(void);
extern esp_err_t blufi_security_init(void);









uint16_t crc16(const char *buf, size_t len);
void process_command(const char* buffer);
bool process_and_send_faces(uint16_t id);





#ifdef __cplusplus
}
#endif



#endif
