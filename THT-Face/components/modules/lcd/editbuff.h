#pragma once

#include <string.h>
#include <stdint.h>
#include "stdbool.h"
#include "handleQrCode.h"
#include "timeLib.h"


extern qrcode_t qrInfo;
extern bool ble_is_connected;
extern uint8_t networkStatus;
extern uint8_t percentage;





#ifdef __cplusplus
extern "C"
{
#endif



void editDisplayBuff(camera_fb_t **buff);
uint64_t generate_unique_id(void);

bool display_faces( camera_fb_t *buff);
// void drawImage(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t *image, camera_fb_t *buff);
void drawFilledRoundedRectangle(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint8_t radius, uint16_t color, camera_fb_t *buff);
void scaleAndDisplayImageInFrame(uint8_t *src_image, uint8_t src_width, uint8_t src_height, camera_fb_t *dst_buff, uint8_t pos_x, uint8_t pos_y);
// void setPixel(camera_fb_t *buff, int x, int y, uint16_t color);
// void drawRoundedRectangle(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t corner_radius, uint16_t color, camera_fb_t *buff);
void drawRoundedRectangleBorder(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint8_t corner_radius, uint16_t color, camera_fb_t *buff);


void iconPrint(uint16_t x_offset, uint8_t y_offset, uint8_t w, uint8_t h,char* logobuff,uint16_t color ,camera_fb_t *buff);
uint8_t get_wifi_signal_strength();

// void WriteString(int x_offset, int y_offset, const char *str, camera_fb_t *buff);
void WriteString(uint8_t letterSize, uint16_t x_offset, uint8_t y_offset, const char *str, camera_fb_t *buff);
// void wrightChar(int x_offset, int y_offset, char c, camera_fb_t *buff);
void wrightChar(uint8_t letterSize, uint16_t x_offset, uint8_t y_offset, char c, camera_fb_t *buff);
// void writeSn(camera_fb_t *buff);
void writeSn(camera_fb_t *buff ,uint64_t id);
void writedateTime(camera_fb_t *buff,time_library_time_t current_time,uint8_t clockType);
uint16_t pixleLen(uint8_t letSize, char *str);

void sleepTimeDate(camera_fb_t *buff, time_library_time_t current_time);
void wrighSingle7segment(uint16_t x_offset, uint8_t y_offset, char c, camera_fb_t *buff);
void WriteMulti7segment(uint16_t x_offset, uint8_t y_offset, const char *str, camera_fb_t *buff);
void timeDisplay(uint8_t x, uint8_t y, uint8_t value,camera_fb_t *buff);


#ifdef __cplusplus
}
#endif