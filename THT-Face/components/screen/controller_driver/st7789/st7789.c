// Copyright 2020 Espressif Systems (Shanghai) Co. Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "screen_driver.h"
#include "screen_utility.h"
#include "st7789.h"

static const char *TAG = "lcd st7789";

#define LCD_CHECK(a, str, ret)  if(!(a)) {                               \
        ESP_LOGE(TAG,"%s:%d (%s):%s", __FILE__, __LINE__, __FUNCTION__, str);   \
        return (ret);                                                           \
    }

#define LCD_NAME "ST7789"
#define LCD_BPP  16

/** commands of ST7789 */
#define LCD_SWRESET 0x01  // Software Reset
#define LCD_RDDID   0x04  // Read Display ID
#define LCD_INVOFF  0x20  // Display Inversion Off
#define LCD_INVON   0x21  // Display Inversion On
#define LCD_CASET   0x2A  // Column Address Set
#define LCD_PASET   0x2B  // Row Address Set
#define LCD_RAMWR   0x2C  // Memory Writ
#define LCD_RAMRD   0x2E  // Memory Read
#define LCD_MADCTL  0x36  // Memory Data Access Control

/* MADCTL Defines */
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x08
#define MADCTL_MH  0x04

#define ST7789_RESOLUTION_HOR 240
#define ST7789_RESOLUTION_VER 320

static scr_handle_t g_lcd_handle;

/**
 * This header file is only used to redefine the function to facilitate the call.
 * It can only be placed in this position, not in the head of the file.
 */
#include "interface_drv_def.h"






esp_err_t lcd_st7789_cmd(uint8_t cmd){

    LCD_WRITE_DATA(cmd);
    return ESP_OK;

}


scr_driver_t lcd_st7789_default_driver = {
    .init = lcd_st7789_init,
    .deinit = lcd_st7789_deinit,
    .set_direction = lcd_st7789_set_rotation,
    .set_window = lcd_st7789_set_window,
    .write_ram_data = lcd_st7789_write_ram_data,
    .draw_pixel = lcd_st7789_draw_pixel,
    .draw_bitmap = lcd_st7789_draw_bitmap,
    .get_info = lcd_st7789_get_info,
};

static void lcd_st7789_init_reg(void)
{

#if !defined GC9306

    LCD_WRITE_CMD(0x3A);
    LCD_WRITE_DATA(0x05);
    LCD_WRITE_CMD(0xB2);
    LCD_WRITE_DATA(0x0C);
    LCD_WRITE_DATA(0x0C);
    LCD_WRITE_DATA(0x00);
    LCD_WRITE_DATA(0x33);
    LCD_WRITE_DATA(0x33);

    LCD_WRITE_CMD(0xB7);  //Gate Control
    LCD_WRITE_DATA(0x35);

    LCD_WRITE_CMD(0xBB);  //VCOM Setting
    LCD_WRITE_DATA(0x19);

    LCD_WRITE_CMD(0xC0); //LCM Control     
    LCD_WRITE_DATA(0x2C);

    LCD_WRITE_CMD(0xC2);  //VDV and VRH Command Enable
    LCD_WRITE_DATA(0x01);
    LCD_WRITE_CMD(0xC3);  //VRH Set
    LCD_WRITE_DATA(0x12);
    LCD_WRITE_CMD(0xC4);  //VDV Set
    LCD_WRITE_DATA(0x20);

    LCD_WRITE_CMD(0xC6);  //Frame Rate Control in Normal Mode
    LCD_WRITE_DATA(0x0F);
    
    LCD_WRITE_CMD(0xD0);  // Power Control 1
    LCD_WRITE_DATA(0xA4);
    LCD_WRITE_DATA(0xA1);

    LCD_WRITE_CMD(0xE0);  //Positive Voltage Gamma Control
    LCD_WRITE_DATA(0xD0);
    LCD_WRITE_DATA(0x04);
    LCD_WRITE_DATA(0x0D);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x13);
    LCD_WRITE_DATA(0x2B);
    LCD_WRITE_DATA(0x3F);
    LCD_WRITE_DATA(0x54);
    LCD_WRITE_DATA(0x4C);
    LCD_WRITE_DATA(0x18);
    LCD_WRITE_DATA(0x0D);
    LCD_WRITE_DATA(0x0B);
    LCD_WRITE_DATA(0x1F);
    LCD_WRITE_DATA(0x23);

    LCD_WRITE_CMD(0xE1);  //Negative Voltage Gamma Control
    LCD_WRITE_DATA(0xD0);
    LCD_WRITE_DATA(0x04);
    LCD_WRITE_DATA(0x0C);
    LCD_WRITE_DATA(0x11);
    LCD_WRITE_DATA(0x13);
    LCD_WRITE_DATA(0x2C);
    LCD_WRITE_DATA(0x3F);
    LCD_WRITE_DATA(0x44);
    LCD_WRITE_DATA(0x51);
    LCD_WRITE_DATA(0x2F);
    LCD_WRITE_DATA(0x1F);
    LCD_WRITE_DATA(0x1F);
    LCD_WRITE_DATA(0x20);
    LCD_WRITE_DATA(0x23);

    LCD_WRITE_CMD(0x21);  //Display Inversion On

    LCD_WRITE_CMD(0x11);  //Sleep Out

    LCD_WRITE_CMD(0x29);  //Display On

#else

//----------------------------------------end Reset Sequence---------------------------------------//
//----------------------------------display control setting----------------------------------------//
LCD_WRITE_CMD(0xfe);
LCD_WRITE_CMD(0xef);
LCD_WRITE_CMD(0x36);
LCD_WRITE_DATA(0x48);
LCD_WRITE_CMD(0x3a);
LCD_WRITE_DATA(0x05);
LCD_WRITE_CMD(0xad);
LCD_WRITE_DATA(0x33);
LCD_WRITE_CMD(0xaf);
LCD_WRITE_DATA(0x55);
LCD_WRITE_CMD(0xae);
LCD_WRITE_DATA(0x2b);

//----------------------------------end display control setting--------------------------------//
//----------------------------------Power Control Registers Initial--------------------------------//
LCD_WRITE_CMD(0xa4);
LCD_WRITE_DATA(0x44);
LCD_WRITE_DATA(0x44);
LCD_WRITE_CMD(0xa5);
LCD_WRITE_DATA(0x42);
LCD_WRITE_DATA(0x42);
LCD_WRITE_CMD(0xaa);
LCD_WRITE_DATA(0x88);
LCD_WRITE_DATA(0x88);
LCD_WRITE_CMD(0xae);
LCD_WRITE_DATA(0x2b);
LCD_WRITE_CMD(0xe8);
LCD_WRITE_DATA(0x11);
LCD_WRITE_DATA(0x0b);
LCD_WRITE_CMD(0xe3);
LCD_WRITE_DATA(0x01);
LCD_WRITE_DATA(0x10);
LCD_WRITE_CMD(0xff);
LCD_WRITE_DATA(0x61);
LCD_WRITE_CMD(0xAC);
LCD_WRITE_DATA(0x00);
LCD_WRITE_CMD(0xAf);
LCD_WRITE_DATA(0x67);
LCD_WRITE_CMD(0xa6);
LCD_WRITE_DATA(0x2a);
LCD_WRITE_DATA(0x2a);
LCD_WRITE_CMD(0xa7);
LCD_WRITE_DATA(0x2b);
LCD_WRITE_DATA(0x2b);
LCD_WRITE_CMD(0xa8);
LCD_WRITE_DATA(0x18);
LCD_WRITE_DATA(0x18);
LCD_WRITE_CMD(0xa9);
LCD_WRITE_DATA(0x2a);
LCD_WRITE_DATA(0x2a);
//---------------------------------display window 240X320-------------------------------------//
LCD_WRITE_CMD(0x2a);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0xef);
LCD_WRITE_CMD(0x2b);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0x01);
LCD_WRITE_DATA(0x3f);
LCD_WRITE_CMD(0x2c);
//------------------------------------end display window ------------------------------------------//
//----------------------------------------gamma setting----------------------------------------------//

// LCD_WRITE_CMD(0xf0);
// LCD_WRITE_DATA(0x02);
// LCD_WRITE_DATA(0x00);
// LCD_WRITE_DATA(0x00);

// LCD_WRITE_DATA(0x1b);

// LCD_WRITE_DATA(0x1f);
// LCD_WRITE_DATA(0xb);
// LCD_WRITE_CMD(0xf1);
// LCD_WRITE_DATA(0x01);
// LCD_WRITE_DATA(0x3);
// LCD_WRITE_DATA(0x00);

// LCD_WRITE_DATA(0x28);

// LCD_WRITE_DATA(0x2b);
// LCD_WRITE_DATA(0xe);
// LCD_WRITE_CMD(0xf2);
// LCD_WRITE_DATA(0xb);
// LCD_WRITE_DATA(0x08);
// LCD_WRITE_DATA(0x3b);
// LCD_WRITE_DATA(0x04);
// LCD_WRITE_DATA(0x03);
// LCD_WRITE_DATA(0x4c);
// LCD_WRITE_CMD(0xf3);
// LCD_WRITE_DATA(0xe);
// LCD_WRITE_DATA(0x7);
// LCD_WRITE_DATA(0x46);
// LCD_WRITE_DATA(0x4);
// LCD_WRITE_DATA(0x5);
// LCD_WRITE_DATA(0x51);
// LCD_WRITE_CMD(0xf4);
// LCD_WRITE_DATA(0x08);
// LCD_WRITE_DATA(0x15);
// LCD_WRITE_DATA(0x15);
// LCD_WRITE_DATA(0x1f);
// LCD_WRITE_DATA(0x22);
// LCD_WRITE_DATA(0x0F);
// LCD_WRITE_CMD(0xf5);
// LCD_WRITE_DATA(0xb);
// LCD_WRITE_DATA(0x13);
// LCD_WRITE_DATA(0x11);
// LCD_WRITE_DATA(0x1f);
// LCD_WRITE_DATA(0x21);
// LCD_WRITE_DATA(0x0F);



// Gamma correction (adjust white balance)
LCD_WRITE_CMD(0xF0);
LCD_WRITE_DATA(0x02);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(20); // Adjust for white balance 20
LCD_WRITE_DATA(0x1C);
LCD_WRITE_DATA(0x08);

LCD_WRITE_CMD(0xF1);
LCD_WRITE_DATA(0x01);
LCD_WRITE_DATA(0x02);
LCD_WRITE_DATA(0x00);
LCD_WRITE_DATA(0x00); // Adjust for white balance 20
LCD_WRITE_DATA(0x22);
LCD_WRITE_DATA(0x0C);

// Brightness control
LCD_WRITE_CMD(0x51);
LCD_WRITE_DATA(0x00); // Set brightness to medium (0x00 - 0xFF) 80 

// Contrast control
LCD_WRITE_CMD(0xA6);
LCD_WRITE_DATA(0x00); // Adjust contrast level 30 
LCD_WRITE_DATA(0x30);




//------------------------------------end gamma setting------------------------------------------//
LCD_WRITE_CMD(0x11);
// vTaskDelay(120 / portTICK_RATE_MS);//120
LCD_WRITE_CMD(0x29);
LCD_WRITE_CMD(0x2c);

// sleep exit 

LCD_WRITE_CMD(0xfe);
LCD_WRITE_CMD(0xef);
LCD_WRITE_CMD(0x11);
// vTaskDelay(120 / portTICK_RATE_MS);//120
LCD_WRITE_CMD(0x29);

#endif



}

esp_err_t lcd_st7789_init(const scr_controller_config_t *lcd_conf)
{
    LCD_CHECK(lcd_conf->width <= ST7789_RESOLUTION_HOR, "Width greater than maximum", ESP_ERR_INVALID_ARG);
    LCD_CHECK(lcd_conf->height <= ST7789_RESOLUTION_VER, "Height greater than maximum", ESP_ERR_INVALID_ARG);
    LCD_CHECK(NULL != lcd_conf, "config pointer invalid", ESP_ERR_INVALID_ARG);
    LCD_CHECK((NULL != lcd_conf->interface_drv->write_cmd && \
               NULL != lcd_conf->interface_drv->write_data && \
               NULL != lcd_conf->interface_drv->write && \
               NULL != lcd_conf->interface_drv->read && \
               NULL != lcd_conf->interface_drv->bus_acquire && \
               NULL != lcd_conf->interface_drv->bus_release),
              "Interface driver invalid", ESP_ERR_INVALID_ARG);
    esp_err_t ret;
    // Reset the display
    if (lcd_conf->pin_num_rst >= 0) {
        gpio_pad_select_gpio(lcd_conf->pin_num_rst);
        gpio_set_direction(lcd_conf->pin_num_rst, GPIO_MODE_OUTPUT);
        gpio_set_level(lcd_conf->pin_num_rst, (lcd_conf->rst_active_level) & 0x1);
        // vTaskDelay(100 / portTICK_RATE_MS);//100
        gpio_set_level(lcd_conf->pin_num_rst, (~(lcd_conf->rst_active_level)) & 0x1);
        // vTaskDelay(100 / portTICK_RATE_MS);//100
    }

    g_lcd_handle.interface_drv = lcd_conf->interface_drv;
    g_lcd_handle.original_width = lcd_conf->width;
    g_lcd_handle.original_height = lcd_conf->height;
    g_lcd_handle.offset_hor = lcd_conf->offset_hor;
    g_lcd_handle.offset_ver = lcd_conf->offset_ver;

    lcd_st7789_init_reg();

    // Enable backlight
    if (lcd_conf->pin_num_bckl >= 0) {
        gpio_pad_select_gpio(lcd_conf->pin_num_bckl);
        gpio_set_direction(lcd_conf->pin_num_bckl, GPIO_MODE_OUTPUT);
        gpio_set_level(lcd_conf->pin_num_bckl, (lcd_conf->bckl_active_level) & 0x1);
    }
    ret = lcd_st7789_set_rotation(lcd_conf->rotate);
    LCD_CHECK(ESP_OK == ret, "Set rotate failed", ESP_FAIL);
    ret = lcd_st7789_set_invert(0);  /**< ST7789 setting the reverse color is the normal color */
    LCD_CHECK(ESP_OK == ret, "Set color invert failed", ESP_FAIL);
    return ESP_OK;
}

esp_err_t lcd_st7789_deinit(void)
{
    memset(&g_lcd_handle, 0, sizeof(scr_handle_t));
    return ESP_OK;
}

esp_err_t lcd_st7789_set_rotation(scr_dir_t dir)
{
    esp_err_t ret;
    uint8_t reg_data = 0;

// to configure our new display
#if !defined GC9306
    reg_data &= ~MADCTL_RGB;// st7789
#else
    reg_data |= MADCTL_RGB; // GC9306
#endif

    if (SCR_DIR_MAX < dir) {
        dir >>= 5;
    }
    LCD_CHECK(dir < 8, "Unsupport rotate direction", ESP_ERR_INVALID_ARG);
    switch (dir) {
    case SCR_DIR_LRTB:
        g_lcd_handle.width = g_lcd_handle.original_width;
        g_lcd_handle.height = g_lcd_handle.original_height;
        break;
    case SCR_DIR_LRBT:
        reg_data |= MADCTL_MY;
        g_lcd_handle.width = g_lcd_handle.original_width;
        g_lcd_handle.height = g_lcd_handle.original_height;
        break;
    case SCR_DIR_RLTB:
        reg_data |= MADCTL_MX;
        g_lcd_handle.width = g_lcd_handle.original_width;
        g_lcd_handle.height = g_lcd_handle.original_height;
        break;
    case SCR_DIR_RLBT:
        reg_data |= MADCTL_MX | MADCTL_MY;
        g_lcd_handle.width = g_lcd_handle.original_width;
        g_lcd_handle.height = g_lcd_handle.original_height;
        break;

    case SCR_DIR_TBLR:
        reg_data |= MADCTL_MV;
        g_lcd_handle.width = g_lcd_handle.original_height;
        g_lcd_handle.height = g_lcd_handle.original_width;
        break;
    case SCR_DIR_BTLR:
        reg_data |= MADCTL_MY | MADCTL_MV;
        g_lcd_handle.width = g_lcd_handle.original_height;
        g_lcd_handle.height = g_lcd_handle.original_width;
        break;
    case SCR_DIR_TBRL:
        reg_data |= MADCTL_MX | MADCTL_MV;
        g_lcd_handle.width = g_lcd_handle.original_height;
        g_lcd_handle.height = g_lcd_handle.original_width;
        break;
    case SCR_DIR_BTRL:
        reg_data |= MADCTL_MX | MADCTL_MY | MADCTL_MV;
        g_lcd_handle.width = g_lcd_handle.original_height;
        g_lcd_handle.height = g_lcd_handle.original_width;
        break;
    default: break;
    }
    ESP_LOGI(TAG, "MADCTL=%x", reg_data);
    ret = LCD_WRITE_REG(LCD_MADCTL, reg_data);
    LCD_CHECK(ESP_OK == ret, "Set screen rotate failed", ESP_FAIL);
    g_lcd_handle.dir = dir;
    return ESP_OK;
}

esp_err_t lcd_st7789_get_info(scr_info_t *info)
{
    LCD_CHECK(NULL != info, "info pointer invalid", ESP_ERR_INVALID_ARG);
    info->width = g_lcd_handle.width;
    info->height = g_lcd_handle.height;
    info->dir = g_lcd_handle.dir;
    info->name = LCD_NAME;
    info->color_type = SCR_COLOR_TYPE_RGB565;
    info->bpp = LCD_BPP;
    return ESP_OK;
}

esp_err_t lcd_st7789_set_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1)
{
    LCD_CHECK((x1 < g_lcd_handle.width) && (y1 < g_lcd_handle.height), "The set coordinates exceed the screen size", ESP_ERR_INVALID_ARG);
    LCD_CHECK((x0 <= x1) && (y0 <= y1), "Window coordinates invalid", ESP_ERR_INVALID_ARG);
    esp_err_t ret = ESP_OK;
    scr_utility_apply_offset(&g_lcd_handle, ST7789_RESOLUTION_HOR, ST7789_RESOLUTION_VER, &x0, &y0, &x1, &y1);

    ret |= LCD_WRITE_CMD(LCD_CASET);
    ret |= LCD_WRITE_DATA(x0 >> 8);
    ret |= LCD_WRITE_DATA(x0 & 0xff);
    ret |= LCD_WRITE_DATA(x1 >> 8);
    ret |= LCD_WRITE_DATA(x1 & 0xff);
    ret |= LCD_WRITE_CMD(LCD_PASET);
    ret |= LCD_WRITE_DATA(y0 >> 8);
    ret |= LCD_WRITE_DATA(y0 & 0xff);
    ret |= LCD_WRITE_DATA(y1 >> 8);
    ret |= LCD_WRITE_DATA(y1 & 0xff);

    ret |= LCD_WRITE_CMD(LCD_RAMWR);
    LCD_CHECK(ESP_OK == ret, "Set window failed", ESP_FAIL);
    return ESP_OK;
}

esp_err_t lcd_st7789_write_ram_data(uint16_t color)
{
    static uint8_t data[2];
    data[0] = (uint8_t)(color & 0xff);
    data[1] = (uint8_t)(color >> 8);
    return LCD_WRITE(data, 2);
}

esp_err_t lcd_st7789_set_invert(bool is_invert)
{
    return LCD_WRITE_CMD(is_invert ? LCD_INVON : LCD_INVOFF);
}

esp_err_t lcd_st7789_draw_pixel(uint16_t x, uint16_t y, uint16_t color)
{
    esp_err_t ret;
    ret = lcd_st7789_set_window(x, y, x, y);
    if (ESP_OK != ret) {
        return ESP_FAIL;
    }
    return lcd_st7789_write_ram_data(color);
}

esp_err_t lcd_st7789_draw_bitmap(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t *bitmap)
{
    esp_err_t ret;
    LCD_CHECK(NULL != bitmap, "bitmap pointer invalid", ESP_ERR_INVALID_ARG);

    LCD_IFACE_ACQUIRE();
    ret = lcd_st7789_set_window(x, y, x + w - 1, y + h - 1);
    if (ESP_OK != ret) {
        return ESP_FAIL;
    }

    uint32_t len = w * h;
    ret = LCD_WRITE((uint8_t *)bitmap, 2 * len);
    LCD_IFACE_RELEASE();
    LCD_CHECK(ESP_OK == ret, "lcd write ram data failed", ESP_FAIL);
    return ESP_OK;
}
