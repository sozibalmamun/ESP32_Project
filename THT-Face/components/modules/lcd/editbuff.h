#pragma once

#include <string.h>
#include <stdint.h>
#include "stdbool.h"
#include "handleQrCode.h"


extern qrcode_t qrInfo;

#ifdef __cplusplus
extern "C"
{
#endif


void editDisplayBuff(camera_fb_t **buff);
uint64_t generate_unique_id(void);
void iconPrint(int x_offset, int y_offset, uint8_t w, uint8_t h,char* logobuff, camera_fb_t *buff);
uint8_t get_wifi_signal_strength();
// void wrightChar(int x_offset, int y_offset, const uint8_t *char_data, camera_fb_t *buff);
void wrightChar(int x_offset, int y_offset, char c, camera_fb_t *buff);

#ifdef __cplusplus
}
#endif