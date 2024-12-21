#pragma once

#include <stdint.h>
#include "esp_log.h"
#include "screen_driver.h"
#include "esp_camera.h"

#define BOARD_LCD_MOSI 47
#define BOARD_LCD_MISO -1
#define BOARD_LCD_SCK 21
#define BOARD_LCD_CS 42
#define BOARD_LCD_DC 41
#define BOARD_LCD_RST -1
#define BOARD_LCD_BL 2
#define BOARD_LCD_PIXEL_CLOCK_HZ (40 * 1000 * 1000)

#define BOARD_LCD_BK_LIGHT_ON_LEVEL 0
#define BOARD_LCD_BK_LIGHT_OFF_LEVEL !BOARD_LCD_BK_LIGHT_ON_LEVEL
#define BOARD_LCD_H_RES 320
#define BOARD_LCD_V_RES 240
#define BOARD_LCD_CMD_BITS 8
#define BOARD_LCD_PARAM_BITS 8
#define LCD_HOST SPI2_HOST

extern  uint8_t sleepEnable;



#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t register_lcd(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const bool return_fb);

    void app_lcd_draw_wallpaper();
    void app_lcd_set_color(int color);
    void app_lcd_draw_wallpaper_try();
    void scaleImageTo320x240(camera_fb_t *src, camera_fb_t *dst);
    void scaleImageTo320x240_on_the_fly(camera_fb_t *src, camera_fb_t *dst);



#ifdef __cplusplus
}
#endif
