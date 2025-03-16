#pragma once

#include <string.h>
#include <stdint.h>
#include "stdbool.h"
#include "handleQrCode.h"
#include "timeLib.h"
#include "who_button.h"
#include "gpioControl.h"


extern qrcode_t qrInfo;
extern bool ble_is_connected;
extern uint8_t networkStatus;
extern int8_t percentage;
extern key_state_t key_state;
extern uint8_t CPUBgflag;
extern bool dataAvailable;
extern bool dspTimeFormet;
extern uint8_t DataUpDoun;
extern uint8_t config;




typedef struct {
    uint32_t bitmap_index; // Index of the glyph bitmap in the bitmap array
    uint16_t adv_w;        // Advance width
    uint8_t box_w;         // Glyph width in pixels
    uint8_t box_h;         // Glyph height in pixels
    int8_t ofs_x;          // Horizontal offset from cursor position
    int8_t ofs_y;          // Vertical offset from baseline
} lv_font_fmt_txt_glyph_dsc_t;








#ifdef __cplusplus
extern "C"
{
#endif


uint64_t generate_unique_id(void);
uint8_t calculate_battery_level(uint32_t voltage);
uint16_t pixleLen(uint8_t letSize, char *str);
uint8_t get_wifi_signal_strength();

void editDisplayBuff(camera_fb_t **buff);
bool display_faces( camera_fb_t *buff);
void drawImage_u8(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t *image, camera_fb_t *buff);
void drawImage_u16(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint16_t *image, camera_fb_t *buff);
void drawFilledRoundedRectangle(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint8_t radius, uint16_t color, camera_fb_t *buff);
void scaleAndDisplayImageInFrame(uint8_t *src_image, uint8_t src_width, uint8_t src_height, camera_fb_t *dst_buff, uint8_t pos_x, uint8_t pos_y);
void drawRoundedRectangleBorder(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint8_t corner_radius, uint16_t color, camera_fb_t *buff);
void iconPrint(uint16_t x_offset, uint8_t y_offset, uint8_t w, uint8_t h,char* logobuff,uint16_t color ,camera_fb_t *buff);
void icnPrint(uint16_t x_offset, uint8_t y_offset, uint8_t w, uint8_t h,uint16_t* logobuff,uint16_t color ,camera_fb_t *buff);
void WriteString(uint8_t letterSize, uint16_t x_offset, uint8_t y_offset, const char *str,uint16_t color, camera_fb_t *buff);
void wrightChar(uint8_t letterSize, uint16_t x_offset, uint8_t y_offset, char c,uint16_t color, camera_fb_t *buff);
void writeSn(camera_fb_t *buff ,uint64_t id);
void writedateTime(camera_fb_t *buff,time_library_time_t current_time,uint8_t clockType);
void sleepTimeDate(camera_fb_t *buff, time_library_time_t current_time);
void wrighSingle7segment(uint16_t x_offset, uint8_t y_offset, char c, camera_fb_t *buff);
void WriteMulti7segment(uint16_t x_offset, uint8_t y_offset, const char *str, camera_fb_t *buff);
void timeDisplay(uint8_t x, uint8_t y, uint8_t value,camera_fb_t *buff);
void renderText(camera_fb_t *fb, int x, int y, const char *text, const lv_font_fmt_txt_glyph_dsc_t *glyph_dsc, const uint8_t *bitmap, uint16_t color) ;



#ifdef __cplusplus
}
#endif