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

#ifdef __cplusplus
}
#endif