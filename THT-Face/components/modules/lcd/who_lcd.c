#include "who_lcd.h"
#include "esp_camera.h"
#include <string.h>
#include "logo_en_240x240_lcd.h"

// #include "handleQrCode.h"

//------------------------------

#define glog(x) __LPM(&g0log[x])
#define gexp(x) __LPM(&g0exp[x])
#define QRBIT(x,y) ( ( qrframe[((x)>>3) + (y) * WDB] >> (7-((x) & 7 ))) & 1 )
#define SETQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] |= 0x80 >> ((x) & 7)
#define TOGQRBIT(x,y) qrframe[((x)>>3) + (y) * WDB] ^= 0x80 >> ((x) & 7)
#define __LPM(x) *x

int8_t offsetsX_x = 42;
int8_t offsetsY = 10;
int16_t screenwidth = 128;
int8_t screenheight = 64;



//------------------------------


static const char *TAG = "who_lcd";

static scr_driver_t g_lcd;
static scr_info_t g_lcd_info;

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static bool gReturnFB = true;


uint8_t wifiStatus=false;

static void task_process_handler(void *arg)
{
    camera_fb_t *frame = NULL;

    while (true)
    {
        if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
        {


            // if(!wifiStatus)app_lcd_draw_wallpaper_try();

            // memset(frame->buf, 0, frame->len);



             // Erase a 50x50 section of the frame buffer from 340*240
            int start_x = 202;  // Starting x coordinate
            int start_y = 22;  // Starting y coordinate
            int erase_width = 110;
            int erase_height = 110;

            for (int y = start_y; y < start_y + erase_height; y++)
            {
                for (int x = start_x; x < start_x + erase_width; x++)
                {
                    int index = (y * frame->width + x) * 2; // Assuming 2 bytes per pixel

                    frame->buf[index] = 0xff;
                    frame->buf[index + 1] = 0xff;

                }
            }

            create("facebook.com");

            // g_lcd.draw_bitmap(0, 0, frame->width, frame->height, (uint16_t *)frame->buf);



            if (xQueueFrameO)
            {
                xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
            }
            else if (gReturnFB)
            {
                esp_camera_fb_return(frame);
            }
            else
            {
                free(frame);
            }
        }
    }
}

esp_err_t register_lcd(const QueueHandle_t frame_i, const QueueHandle_t frame_o, const bool return_fb)
{
    spi_config_t bus_conf = {
        .miso_io_num = BOARD_LCD_MISO,
        .mosi_io_num = BOARD_LCD_MOSI,
        .sclk_io_num = BOARD_LCD_SCK,
        .max_transfer_sz = 2 * 320 * 240 + 10,//WANG
    };
    spi_bus_handle_t spi_bus = spi_bus_create(SPI2_HOST, &bus_conf);

    scr_interface_spi_config_t spi_lcd_cfg = {
        .spi_bus = spi_bus,
        .pin_num_cs = BOARD_LCD_CS,
        .pin_num_dc = BOARD_LCD_DC,
        .clk_freq = 40 * 1000000,
        .swap_data = 0,
    };

    scr_interface_driver_t *iface_drv;
    scr_interface_create(SCREEN_IFACE_SPI, &spi_lcd_cfg, &iface_drv);
    esp_err_t ret = scr_find_driver(SCREEN_CONTROLLER_ST7789, &g_lcd);
    if (ESP_OK != ret)
    {
        return ret;
        ESP_LOGE(TAG, "screen find failed");
    }

    scr_controller_config_t lcd_cfg = {
        .interface_drv = iface_drv,
        .pin_num_rst = BOARD_LCD_RST,
        .pin_num_bckl = BOARD_LCD_BL,
        .rst_active_level = 0,
        .bckl_active_level = 0,
        .offset_hor = 0,
        .offset_ver = 0,
        .width = 240,
        .height = 320,
        //.rotate = 4,            SCR_DIR_TBLR, /**< From top to bottom then from left to right */ 
        .rotate = SCR_DIR_BTLR,  //  h miror lcd .
       // .rotate = SCR_DIR_TBRL,/**< From top to bottom then from right to left change by sozib due to flip the display*/

    };
    ret = g_lcd.init(&lcd_cfg);
    if (ESP_OK != ret)
    {
        return ESP_FAIL;
        ESP_LOGE(TAG, "screen initialize failed");
    }

    g_lcd.get_info(&g_lcd_info);
    ESP_LOGI(TAG, "Screen name:%s | width:%d | height:%d", g_lcd_info.name, g_lcd_info.width, g_lcd_info.height);

    app_lcd_set_color(0x000000);
    //vTaskDelay(pdMS_TO_TICKS(200));
    //app_lcd_draw_wallpaper();
    //vTaskDelay(pdMS_TO_TICKS(200));

    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    gReturnFB = return_fb;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 0);

    return ESP_OK;
}

void app_lcd_draw_wallpaper()
{
    scr_info_t lcd_info;
    g_lcd.get_info(&lcd_info);

    uint16_t *pixels = (uint16_t *)heap_caps_malloc((logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (NULL == pixels)
    {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
        return;
    }
    memcpy(pixels, logo_en_240x240_lcd, (logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t));
    g_lcd.draw_bitmap(0, 0, logo_en_240x240_lcd_width, logo_en_240x240_lcd_height, (uint16_t *)pixels);
    heap_caps_free(pixels);
}

void app_lcd_set_color(int color)
{
    scr_info_t lcd_info;
    g_lcd.get_info(&lcd_info);
    uint16_t *buffer = (uint16_t *)malloc(lcd_info.width * sizeof(uint16_t));
    if (NULL == buffer)
    {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
    }
    else
    {
        for (size_t i = 0; i < lcd_info.width; i++)
        {
            buffer[i] = color;
        }

        for (int y = 0; y < lcd_info.height; y++)
        {
            g_lcd.draw_bitmap(0, y, lcd_info.width, 1, buffer);
        }

        free(buffer);
    }
}


void app_lcd_draw_wallpaper_try()
{
    scr_info_t lcd_info;
    g_lcd.get_info(&lcd_info);

    uint16_t *pixels = (uint16_t *)heap_caps_malloc((logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
    if (NULL == pixels)
    {
        ESP_LOGE(TAG, "Memory for bitmap is not enough");
        return;
    }
    memcpy(pixels, logo_en_240x240_lcd, (logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t));
    g_lcd.draw_bitmap(0, 0, logo_en_240x240_lcd_width, logo_en_240x240_lcd_height, (uint16_t *)pixels);
    heap_caps_free(pixels);
}
/////////////////////////////////////////////////qrcpde//////////////////////////////////////////////////////////////////////////

void render(int x, int y, int color){
  
#define multiply 2

  x=(x*multiply)+offsetsX_x;
  y=(y*multiply)+offsetsY;

  if(color==1) {
// drow black
    g_lcd.draw_pixel(x,y,0x0000);

    // tft->drawPixel(x,y,ST77XX_BLACK);
    // if (multiply>1) {
    //   tft->drawPixel(x+1,y,ST77XX_BLACK);
    //   tft->drawPixel(x+1,y+1,ST77XX_BLACK);
    //   tft->drawPixel(x,y+1,ST77XX_BLACK);
    // }

  }
  else {

// drow white
    g_lcd.draw_pixel(x,y,0xffff);

    // tft->drawPixel(x,y,ST77XX_WHITE);
    // if (multiply>1) {
    //   tft->drawPixel(x+1,y,ST77XX_WHITE);
    //   tft->drawPixel(x+1,y+1,ST77XX_WHITE);
    //   tft->drawPixel(x,y+1,ST77XX_WHITE);
    // }

  }
}

void create(char *message) {

  // create QR code

    size_t len = strlen(message);
    if (len >= sizeof(strinbuf)) {
        len = sizeof(strinbuf) - 1;
    }

    // Copy the message to the buffer and ensure it is null-terminated
    strncpy((char *)strinbuf, message, 270);
    strinbuf[len] = '\0'; // Explicitly add null terminator

    qrencode();

  
  // print QR Code
  for (uint8_t x = 0; x < WD; x+=2) {
    for (uint8_t y = 0; y < WD; y++) {
      if ( QRBIT(x,y) &&  QRBIT((x+1),y)) {
        // black square on top of black square
        render(x, y, 1);
        render((x+1), y, 1);
      }
      if (!QRBIT(x,y) &&  QRBIT((x+1),y)) {
        // white square on top of black square
        render(x, y, 0);
        render((x+1), y, 1);
      }
      if ( QRBIT(x,y) && !QRBIT((x+1),y)) {
        // black square on top of white square
        render(x, y, 1);
        render((x+1), y, 0);
      }
      if (!QRBIT(x,y) && !QRBIT((x+1),y)) {
        // white square on top of white square
        render(x, y, 0);
        render((x+1), y, 0);
      }
    }
  }
}



//========================================================================
// Reed Solomon error correction
unsigned modnn(unsigned x)
{
    while (x >= 255) {
        x -= 255;
        x = (x >> 8) + (x & 255);
    }
    return x;
}

void initrspoly(unsigned char eclen, unsigned char *genpoly)
{
    unsigned char i, j;

    genpoly[0] = 1;
    for (i = 0; i < eclen; i++) {
        genpoly[i + 1] = 1;
        for (j = i; j > 0; j--)
            genpoly[j] = genpoly[j]
                ? genpoly[j - 1] ^ gexp(modnn(glog(genpoly[j]) + i)) : genpoly[j - 1];
        genpoly[0] = gexp(modnn(glog(genpoly[0]) + i));
    }
    for (i = 0; i <= eclen; i++)
        genpoly[i] = glog(genpoly[i]);     // use logs for genpoly[]
}

void appendrs(unsigned char *data, unsigned char dlen, 
              unsigned char *ecbuf, unsigned char eclen, unsigned char *genpoly)
{
    unsigned char i, j, fb;

    memset(ecbuf, 0, eclen);
    for (i = 0; i < dlen; i++) {
        fb = glog(data[i] ^ ecbuf[0]);
        if (fb != 255)          /* fb term is non-zero */
            for (j = 1; j < eclen; j++)
                ecbuf[j-1] = ecbuf[j] ^ gexp(modnn(fb + genpoly[eclen - j]));
        else
            memmove(ecbuf, ecbuf + 1, eclen - 1);
        ecbuf[eclen - 1] = fb == 255 ? 0 : gexp(modnn(fb + genpoly[0]));
    }
}

//========================================================================
// 8 bit data to QR-coded 8 bit data
void stringtoqr(void)
{
    unsigned i;
    unsigned size, max;
    size = strlen((char *) strinbuf);

    max = datablkw * (neccblk1 + neccblk2) + neccblk2;
    if (size >= max - 2) {
        size = max - 2;
        if (VERSION > 9)
            size--;
    }

    i = size;
    if (VERSION > 9) {
        strinbuf[i + 2] = 0;
        while (i--) {
            strinbuf[i + 3] |= strinbuf[i] << 4;
            strinbuf[i + 2] = strinbuf[i] >> 4;
        }
        strinbuf[2] |= size << 4;
        strinbuf[1] = size >> 4;
        strinbuf[0] = 0x40 | (size >> 12);
    } else {
        strinbuf[i + 1] = 0;
        while (i--) {
            strinbuf[i + 2] |= strinbuf[i] << 4;
            strinbuf[i + 1] = strinbuf[i] >> 4;
        }
        strinbuf[1] |= size << 4;
        strinbuf[0] = 0x40 | (size >> 4);
    }
    i = size + 3 - (VERSION < 10);
    while (i < max) {
        strinbuf[i++] = 0xec;
        // buffer has room        if (i == max)            break;
        strinbuf[i++] = 0x11;
    }

    // calculate and append ECC
    unsigned char *ecc = &strinbuf[max];
    unsigned char *dat = strinbuf;
    initrspoly(eccblkwid,qrframe);

    for (i = 0; i < neccblk1; i++) {
        appendrs(dat, datablkw, ecc, eccblkwid, qrframe);
        dat += datablkw;
        ecc += eccblkwid;
    }
    for (i = 0; i < neccblk2; i++) {
        appendrs(dat, datablkw + 1, ecc, eccblkwid, qrframe);
        dat += datablkw + 1;
        ecc += eccblkwid;
    }
    unsigned j;
    dat = qrframe;
    for (i = 0; i < datablkw; i++) {
        for (j = 0; j < neccblk1; j++)
            *dat++ = strinbuf[i + j * datablkw];
        for (j = 0; j < neccblk2; j++)
            *dat++ = strinbuf[(neccblk1 * datablkw) + i + (j * (datablkw + 1))];
    }
    for (j = 0; j < neccblk2; j++)
        *dat++ = strinbuf[(neccblk1 * datablkw) + i + (j * (datablkw + 1))];
    for (i = 0; i < eccblkwid; i++)
        for (j = 0; j < neccblk1 + neccblk2; j++)
            *dat++ = strinbuf[max + i + j * eccblkwid];
    memcpy(strinbuf, qrframe, max + eccblkwid * (neccblk1 + neccblk2));

}

//========================================================================
// Frame data insert following the path rules
unsigned char ismasked(unsigned char x, unsigned char y)
{
    unsigned bt;
    if (x > y) {
        bt = x;
        x = y;
        y = bt;
    }
    bt = y;
    bt += y * y;
#if 0
    // bt += y*y;
    unsigned s = 1;
    while (y--) {
        bt += s;
        s += 2;
    }
#endif
    bt >>= 1;
    bt += x;
    return (__LPM(&framask[bt >> 3]) >> (7 - (bt & 7))) & 1;
}

void fillframe(void)
{
    unsigned i;
    unsigned char d, j;
    unsigned char x, y, ffdecy, ffgohv;

    memcpy(qrframe, framebase, WDB * WD);
    x = y = WD - 1;
    ffdecy = 1;                 // up, minus
    ffgohv = 1;

    /* inteleaved data and ecc codes */
    for (i = 0; i < ((datablkw + eccblkwid) * (neccblk1 + neccblk2) + neccblk2); i++) {
        d = strinbuf[i];
        for (j = 0; j < 8; j++, d <<= 1) {
            if (0x80 & d)
                SETQRBIT(x, y);
            do {                // find next fill position
                if (ffgohv)
                    x--;
                else {
                    x++;
                    if (ffdecy) {
                        if (y != 0)
                            y--;
                        else {
                            x -= 2;
                            ffdecy = !ffdecy;
                            if (x == 6) {
                                x--;
                                y = 9;
                            }
                        }
                    } else {
                        if (y != WD - 1)
                            y++;
                        else {
                            x -= 2;
                            ffdecy = !ffdecy;
                            if (x == 6) {
                                x--;
                                y -= 8;
                            }
                        }
                    }
                }
                ffgohv = !ffgohv;
            } while (ismasked(x, y));
        }
    }

}

//========================================================================
// Masking 
void applymask(unsigned char m)
{
    unsigned char x, y, r3x, r3y;

    switch (m) {
    case 0:
        for (y = 0; y < WD; y++)
            for (x = 0; x < WD; x++)
                if (!((x + y) & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
        break;
    case 1:
        for (y = 0; y < WD; y++)
            for (x = 0; x < WD; x++)
                if (!(y & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
        break;
    case 2:
        for (y = 0; y < WD; y++)
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!r3x && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        break;
    case 3:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = r3y, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!r3x && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    case 4:
        for (y = 0; y < WD; y++)
            for (r3x = 0, r3y = ((y >> 1) & 1), x = 0; x < WD; x++, r3x++) {
                if (r3x == 3) {
                    r3x = 0;
                    r3y = !r3y;
                }
                if (!r3y && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        break;
    case 5:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!((x & y & 1) + !(!r3x | !r3y)) && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    case 6:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!(((x & y & 1) + (r3x && (r3x == r3y))) & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    case 7:
        for (r3y = 0, y = 0; y < WD; y++, r3y++) {
            if (r3y == 3)
                r3y = 0;
            for (r3x = 0, x = 0; x < WD; x++, r3x++) {
                if (r3x == 3)
                    r3x = 0;
                if (!(((r3x && (r3x == r3y)) + ((x + y) & 1)) & 1) && !ismasked(x, y))
                    TOGQRBIT(x, y);
            }
        }
        break;
    }
    return;
}

unsigned badruns(unsigned char length)
{
    unsigned char i;
    unsigned runsbad = 0;
    for (i = 0; i <= length; i++)
        if (rlens[i] >= 5)
            runsbad += N1 + rlens[i] - 5;
    // BwBBBwB
    for (i = 3; i < length - 1; i += 2)
        if (rlens[i - 2] == rlens[i + 2]
          && rlens[i + 2] == rlens[i - 1]
          && rlens[i - 1] == rlens[i + 1]
          && rlens[i - 1] * 3 == rlens[i]
          // white around the black pattern?  Not part of spec
          && (rlens[i - 3] == 0 // beginning
            || i + 3 > length   // end
            || rlens[i - 3] * 3 >= rlens[i] * 4 || rlens[i + 3] * 3 >= rlens[i] * 4)
          )
            runsbad += N3;
    return runsbad;
}

int badcheck()
{
    unsigned char x, y, h, b, b1;
    unsigned thisbad = 0;
    int bw = 0;

    // blocks of same color.
    for (y = 0; y < WD - 1; y++)
        for (x = 0; x < WD - 1; x++)
            if ((QRBIT(x, y) && QRBIT(x + 1, y) && QRBIT(x, y + 1) && QRBIT(x + 1, y + 1))      // all black
              || !(QRBIT(x, y) || QRBIT(x + 1, y) || QRBIT(x, y + 1) || QRBIT(x + 1, y + 1)))   // all white
                thisbad += N2;

    // X runs
    for (y = 0; y < WD; y++) {
        rlens[0] = 0;
        for (h = b = x = 0; x < WD; x++) {
            if ((b1 = QRBIT(x, y)) == b)
                rlens[h]++;
            else
                rlens[++h] = 1;
            b = b1;
            bw += b ? 1 : -1;
        }
        thisbad += badruns(h);
    }

    // black/white imbalance
    if (bw < 0)
        bw = -bw;

    unsigned long big = bw;
    unsigned count = 0;
    big += big << 2;
    big <<= 1;
    while (big > WD * WD)
        big -= WD * WD, count++;
    thisbad += count * N4;

    // Y runs
    for (x = 0; x < WD; x++) {
        rlens[0] = 0;
        for (h = b = y = 0; y < WD; y++) {
            if ((b1 = QRBIT(x, y)) == b)
                rlens[h]++;
            else
                rlens[++h] = 1;
            b = b1;
        }
        thisbad += badruns(h);
    }
    return thisbad;
}



void addfmt(unsigned char masknum)
{
    unsigned fmtbits;
    unsigned char i, lvl = ECCLEVEL - 1;

    fmtbits = fmtword[masknum + (lvl << 3)];
    // low byte
    for (i = 0; i < 8; i++, fmtbits >>= 1)
        if (fmtbits & 1) {
            SETQRBIT(WD - 1 - i, 8);
            if (i < 6)
                SETQRBIT(8, i);
            else
                SETQRBIT(8, i + 1);
        }
    // high byte
    for (i = 0; i < 7; i++, fmtbits >>= 1)
        if (fmtbits & 1) {
            SETQRBIT(8, WD - 7 + i);
            if (i)
                SETQRBIT(6 - i, 8);
            else
                SETQRBIT(7, 8);
        }
}

void qrencode()
{
    unsigned mindem = 30000;
    unsigned char best = 0;
    unsigned char i;
    unsigned badness;

    stringtoqr();
    fillframe();                // Inisde loop to avoid having separate mask buffer
    memcpy(strinbuf, qrframe, WD * WDB);
    for (i = 0; i < 8; i++) {
        applymask(i);           // returns black-white imbalance
        badness = badcheck();
#if 0                           //ndef PUREBAD
        if (badness < WD * WD * 5 / 4) {        // good enough - masks grow in compute complexity
            best = i;
            break;
        }
#endif
        if (badness < mindem) {
            mindem = badness;
            best = i;
        }
        if (best == 7)
            break;              // don't increment i to avoid redoing mask
        memcpy(qrframe, strinbuf, WD * WDB);    // reset filled frame
    }
    if (best != i)              // redo best mask - none good enough, last wasn't best
        applymask(best);
    addfmt(best);               // add in final format bytes
}

