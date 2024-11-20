/*******************************************************************************
 * Size: 20 px
 * Bpp: 1
 * Opts: --bpp 1 --size 20 --no-compress --font RebellionSquad-ZpprZ.ttf --symbols 0123456789 --format lvgl -o watch1.c
 ******************************************************************************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifndef WATCH1
#define WATCH1 1
#endif

#if WATCH1

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0030 "0" */
    0xf, 0x83, 0xfc, 0x3c, 0xe7, 0xce, 0xfc, 0xff,
    0xc7, 0xfc, 0x7f, 0xc7, 0xfc, 0x7f, 0xc7, 0xfc,
    0x7f, 0xe7, 0x7e, 0x67, 0xee, 0x3f, 0xc0, 0xf8,

    /* U+0031 "1" */
    0x7, 0x87, 0xe3, 0xf9, 0xff, 0xff, 0xfd, 0xf6,
    0x7c, 0x1f, 0x7, 0xc1, 0xf0, 0xfc, 0x3f, 0xf,
    0xc3, 0xf0, 0xf8, 0xe,

    /* U+0032 "2" */
    0xf, 0x81, 0xfe, 0x3f, 0xe7, 0xff, 0x7f, 0xff,
    0xdf, 0xfd, 0xff, 0xbe, 0x27, 0xe0, 0xfc, 0x1f,
    0x83, 0xf0, 0x3e, 0xe7, 0xff, 0x7f, 0xf3, 0xff,

    /* U+0033 "3" */
    0x1f, 0x87, 0xfe, 0xff, 0xf7, 0xff, 0x7f, 0xf3,
    0x8f, 0x0, 0xe0, 0xfc, 0xf, 0xc0, 0x1e, 0x30,
    0xf7, 0xc7, 0x7f, 0xf7, 0xff, 0x7f, 0xe0, 0xfc,

    /* U+0034 "4" */
    0xf, 0xc0, 0x7f, 0x81, 0xfe, 0xf, 0xf8, 0x3b,
    0xe1, 0xef, 0xc7, 0xbf, 0x1e, 0xfe, 0xff, 0xfb,
    0xff, 0xef, 0xff, 0xb0, 0xfc, 0x3, 0xf0, 0xf,
    0xc0, 0x3f, 0x0, 0x78,

    /* U+0035 "5" */
    0xff, 0xcf, 0xfc, 0xff, 0xcf, 0xfc, 0x78, 0x7,
    0x80, 0x78, 0x3, 0xf8, 0x3f, 0xe0, 0xe, 0x20,
    0xf7, 0xff, 0x7f, 0xff, 0xfe, 0x7f, 0xe3, 0xf8,

    /* U+0036 "6" */
    0x3c, 0x3, 0xe0, 0x7e, 0x7, 0xe0, 0x7c, 0xf,
    0xc0, 0xfc, 0xf, 0xfc, 0xff, 0xef, 0x9f, 0xfb,
    0xff, 0xff, 0xff, 0xe7, 0xfe, 0x7f, 0x81, 0xf0,

    /* U+0037 "7" */
    0xff, 0xe7, 0xff, 0xbf, 0xfc, 0xfb, 0xe0, 0x3f,
    0x1, 0xf0, 0xf, 0x80, 0xfc, 0x7, 0xe0, 0x3e,
    0x3, 0xf0, 0x1f, 0x80, 0xfc, 0x7, 0xe0, 0x3f,
    0x0, 0xf0,

    /* U+0038 "8" */
    0x1f, 0xf, 0xf9, 0xff, 0x7f, 0xff, 0xf, 0xe1,
    0xdf, 0xf1, 0xfe, 0x3f, 0x8f, 0xfb, 0xc3, 0xf8,
    0x7f, 0xff, 0xff, 0xdf, 0xf0, 0xf8,

    /* U+0039 "9" */
    0x7, 0x81, 0xfc, 0x3f, 0xe7, 0xff, 0xff, 0xff,
    0xff, 0xfd, 0xff, 0xbf, 0x7f, 0xf3, 0xff, 0x3,
    0xf0, 0x3e, 0x7, 0xe0, 0x7e, 0x7, 0xc0, 0x38
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 186, .box_w = 12, .box_h = 16, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 24, .adv_w = 147, .box_w = 10, .box_h = 16, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 44, .adv_w = 184, .box_w = 12, .box_h = 16, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 68, .adv_w = 180, .box_w = 12, .box_h = 16, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 92, .adv_w = 196, .box_w = 14, .box_h = 16, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 120, .adv_w = 169, .box_w = 12, .box_h = 16, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 144, .adv_w = 178, .box_w = 12, .box_h = 16, .ofs_x = 1, .ofs_y = -1},
    {.bitmap_index = 168, .adv_w = 165, .box_w = 13, .box_h = 16, .ofs_x = -1, .ofs_y = -1},
    {.bitmap_index = 194, .adv_w = 170, .box_w = 11, .box_h = 16, .ofs_x = 0, .ofs_y = -1},
    {.bitmap_index = 216, .adv_w = 183, .box_w = 12, .box_h = 16, .ofs_x = 0, .ofs_y = -1}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/



/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 48, .range_length = 10, .glyph_id_start = 1,
        .unicode_list = NULL, .glyph_id_ofs_list = NULL, .list_length = 0, .type = LV_FONT_FMT_TXT_CMAP_FORMAT0_TINY
    }
};



/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = NULL,
    .kern_scale = 0,
    .cmap_num = 1,
    .bpp = 1,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif
};

extern const lv_font_t watch1;


/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t watch1 = {
#else
lv_font_t watch1 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 16,          /*The maximum line height required by the font*/
    .base_line = 1,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -2,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = &watch1,
#endif
    .user_data = NULL,
};



#endif /*#if WATCH1*/

