#pragma once

#include <string.h>
#include <stdint.h>
#include "stdbool.h"
#include "esp_camera.h"


typedef struct {
uint8_t xOfset;
uint8_t yOfset;
uint8_t width ;
uint8_t height;
} qrcode_t;




#ifdef __cplusplus
extern "C"
{
#endif

void qrencode(void);
// void createQrcode(char* message);
// void render(int x, int y, int color);

void createQrcode(char *message ,camera_fb_t *buff);
void render(int x, int y, int color ,camera_fb_t *buff);


unsigned modnn(unsigned x);
void initrspoly(unsigned char eclen, unsigned char *genpoly);
void appendrs(unsigned char *data, unsigned char dlen, 
              unsigned char *ecbuf, unsigned char eclen, unsigned char *genpoly);

void stringtoqr(void);
unsigned char ismasked(unsigned char x, unsigned char y);
void fillframe(void);
void applymask(unsigned char m);
unsigned badruns(unsigned char length);
int badcheck();
void addfmt(unsigned char masknum);
void qrencode();



#ifdef __cplusplus
}
#endif
