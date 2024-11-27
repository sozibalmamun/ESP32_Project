#pragma once

#include <string.h>
#include <stdint.h>
#include "stdbool.h"
#include "esp_camera.h"
#include "globalScope.h"



typedef struct {
uint16_t xOfset;
uint8_t yOfset;
uint8_t erase_size;

} qrcode_t;

#ifdef __cplusplus
extern "C"
{
#endif



unsigned badruns(unsigned char length);
unsigned char ismasked(unsigned char x, unsigned char y);
unsigned modnn(unsigned x);
int badcheck();


void qrencode(void);
void createQrcode(char *message ,camera_fb_t *buff);
void render(int x, int y, int color ,camera_fb_t *buff);
void initrspoly(unsigned char eclen, unsigned char *genpoly);
void appendrs(unsigned char *data, unsigned char dlen,unsigned char *ecbuf, unsigned char eclen, unsigned char *genpoly);
void stringtoqr(void);
void fillframe(void);
void applymask(unsigned char m);
void addfmt(unsigned char masknum);
void qrencode();



#ifdef __cplusplus
}
#endif
