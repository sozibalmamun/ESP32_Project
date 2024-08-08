#pragma once

#include <string.h>
#include <stdint.h>
#include "stdbool.h"
#include "handleQrCode.h"
#include "timeLib.h"


extern qrcode_t qrInfo;

#ifdef __cplusplus
extern "C"
{
#endif


void editDisplayBuff(camera_fb_t **buff);
uint64_t generate_unique_id(void);
void iconPrint(int x_offset, int y_offset, uint8_t w, uint8_t h,char* logobuff,uint16_t color ,camera_fb_t *buff);
uint8_t get_wifi_signal_strength();

void WriteString(int x_offset, int y_offset, const char *str, camera_fb_t *buff);
void wrightChar(int x_offset, int y_offset, char c, camera_fb_t *buff);
void writeSn(camera_fb_t *buff);

void sleepTimeDate(camera_fb_t *buff);
void wrighSingle7segment(int x_offset, int y_offset, char c, camera_fb_t *buff);
void WriteMulti7segment(int x_offset, int y_offset, const char *str, camera_fb_t *buff);
void timeDisplay(uint8_t x, uint8_t y, uint8_t value,camera_fb_t *buff);


#ifdef __cplusplus
}
#endif