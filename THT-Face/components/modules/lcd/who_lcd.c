#include "who_lcd.h"
// #include "esp_camera.h"
#include <string.h>
#include "logo_en_240x240_lcd.h"
#include "editbuff.h"



static const char *TAG = "who_lcd";

static scr_driver_t g_lcd;
static scr_info_t g_lcd_info;

static QueueHandle_t xQueueFrameI = NULL;
static QueueHandle_t xQueueFrameO = NULL;
static bool gReturnFB = true;
extern TaskHandle_t lcdTaskHandler;


// static void task_process_handler(void *arg)
// {
//     camera_fb_t *frame = NULL;

//     while (true)
//     {


//         if (xQueueReceive(xQueueFrameI, &frame, portMAX_DELAY))
//         {
//             printf("cam on\n");


//             editDisplayBuff(&frame);
//             g_lcd.draw_bitmap(0, 0, frame->width, frame->height, (uint16_t *)frame->buf);

//             if (xQueueFrameO)
//             {
//                 xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
//             }
//             else if (gReturnFB)
//             {
//                 esp_camera_fb_return(frame);
//             }
//             else
//             {
//                 free(frame);
//             }
//         }
//     }
// }



static void task_process_handler(void *arg)
{
    camera_fb_t *frame = NULL;
    const TickType_t xDelay = pdMS_TO_TICKS(50);  // Run every 100 ms
    const TickType_t queueTimeout = pdMS_TO_TICKS(100);  // Queue receive timeout 500 ms

    while (true)
    {
        // Try to receive from the queue, but with a timeout (e.g., 100 ms)
        if (xQueueReceive(xQueueFrameI, &frame, queueTimeout))
        {

            // sleepEnable=WAKEUP;// IN FUTURE ITS CONTROL BY ADC PIN


            editDisplayBuff(&frame);
            g_lcd.draw_bitmap(0, 0, frame->width, frame->height, (uint16_t *)frame->buf);

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
        else
        {
            // If no frame received, log and delay
            vTaskDelay(xDelay);  // Delay task execution if no frame was received
            
            if(sleepEnable){

                // printf("dsp sleep\n");
                time_library_time_t current_time;
                sleepTimeDate(frame,current_time);
                g_lcd.draw_bitmap(0, 0, frame->width, frame->height, (uint16_t *)frame->buf);

            }



        }

        
    }
}

// // Efficient nearest-neighbor scaling of a large image to 320x240
// void scaleImageTo320x240_on_the_fly(camera_fb_t *src, camera_fb_t *dst) {
//     int target_width = 320;
//     int target_height = 240;
//     int bytes_per_pixel = 2; // RGB565 format, 2 bytes per pixel

//     float x_ratio = (float)src->width / target_width;
//     float y_ratio = (float)src->height / target_height;

//     // Iterate over each pixel in the destination (320x240)
//     for (int y = 0; y < target_height; y++) {
//         for (int x = 0; x < target_width; x++) {
//             int nearest_x = (int)(x * x_ratio);
//             int nearest_y = (int)(y * y_ratio);

//             // Calculate source pixel index in the original 1024x768 image
//             int src_index = (nearest_y * src->width + nearest_x) * bytes_per_pixel;

//             // Calculate destination pixel index in the 320x240 image
//             int dst_index = (y * target_width + x) * bytes_per_pixel;

//             // Copy the pixel data from source to destination
//             dst->buf[dst_index] = src->buf[src_index];         // High byte
//             dst->buf[dst_index + 1] = src->buf[src_index + 1]; // Low byte
//         }
//     }

//     // Update the destination frame properties
//     dst->width = target_width;
//     dst->height = target_height;
//     dst->len = target_width * target_height * bytes_per_pixel;










// }

// void task_process_handler(void *arg)
// {
//     camera_fb_t *frame = NULL;
//     const TickType_t xDelay = pdMS_TO_TICKS(50);  // Delay for retries
//     const TickType_t queueTimeout = pdMS_TO_TICKS(100);  // Queue timeout

//     while (true) {
//         // Try to receive a frame from the queue
//         if (xQueueReceive(xQueueFrameI, &frame, queueTimeout)) {
//             // Ensure enough memory is available for resized frame
//             size_t free_heap_size = heap_caps_get_free_size(MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//             if (free_heap_size < (320 * 240 * 2 + sizeof(camera_fb_t))) {
//                 printf("Insufficient memory to process frame\n");
//                 vTaskDelay(xDelay);  // Wait and retry
//                 continue;
//             }

//             // Allocate memory for resized frame
//             camera_fb_t *resized_frame = (camera_fb_t *)heap_caps_malloc(sizeof(camera_fb_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//             if (!resized_frame) {
//                 printf("Failed to allocate memory for resized frame\n");
//                 vTaskDelay(xDelay);
//                 continue;
//             }

//             // Allocate buffer for resized frame (320x240, RGB565)
//             resized_frame->buf = (uint8_t *)heap_caps_malloc(320 * 240 * 2, MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//             if (!resized_frame->buf) {
//                 printf("Failed to allocate memory for resized frame buffer\n");
//                 heap_caps_free(resized_frame);
//                 vTaskDelay(xDelay);
//                 continue;
//             }

//             // Scale the original frame to 320x240
//             scaleImageTo320x240_on_the_fly(frame, resized_frame);

//             // Process and display the resized frame
//             // editDisplayBuff(resized_frame);

//             // printf("w %d h %d\n",resized_frame->width, resized_frame->height);

//             g_lcd.draw_bitmap(0, 0, 320, 240, (uint16_t *)resized_frame->buf);

//             // Free the resized frame's buffer and structure after use
//             heap_caps_free(resized_frame->buf);
//             heap_caps_free(resized_frame);

//             // Return or send the processed frame back to queue
//             if (xQueueFrameO) {
//                 xQueueSend(xQueueFrameO, &frame, portMAX_DELAY);
//             } else if (gReturnFB) {
//                 esp_camera_fb_return(frame);
//             } else {
//                 free(frame);
//             }
//         } else {
//             // No frame received, delay task and check sleep mode
//             vTaskDelay(xDelay);

//             if (sleepEnable) {
//                 printf("Display sleep mode\n");
//             }
//         }
//     }
// }



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
        .clk_freq = 80 * 1000000,//40//60
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
        .bckl_active_level = 1,
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

    // g_lcd.get_info(&g_lcd_info);
    // ESP_LOGI(TAG, "Screen name:%s | width:%d | height:%d", g_lcd_info.name, g_lcd_info.width, g_lcd_info.height);

    app_lcd_set_color(0x000000);
    //vTaskDelay(pdMS_TO_TICKS(200));
    //app_lcd_draw_wallpaper();
    //vTaskDelay(pdMS_TO_TICKS(200));

    xQueueFrameI = frame_i;
    xQueueFrameO = frame_o;
    gReturnFB = return_fb;
    xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, &lcdTaskHandler, 0);
        // xTaskCreatePinnedToCore(task_process_handler, TAG, 4 * 1024, NULL, 5, NULL, 1);


    return ESP_OK;
}

// void app_lcd_draw_wallpaper()
// {
//     scr_info_t lcd_info;
//     g_lcd.get_info(&lcd_info);

//     uint16_t *pixels = (uint16_t *)heap_caps_malloc((logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//     if (NULL == pixels)
//     {
//         ESP_LOGE(TAG, "Memory for bitmap is not enough");
//         return;
//     }
//     memcpy(pixels, logo_en_240x240_lcd, (logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t));
//     g_lcd.draw_bitmap(0, 0, logo_en_240x240_lcd_width, logo_en_240x240_lcd_height, (uint16_t *)pixels);
//     heap_caps_free(pixels);
// }

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


// void app_lcd_draw_wallpaper_try()
// {
//     scr_info_t lcd_info;
//     g_lcd.get_info(&lcd_info);

//     uint16_t *pixels = (uint16_t *)heap_caps_malloc((logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
//     if (NULL == pixels)
//     {
//         ESP_LOGE(TAG, "Memory for bitmap is not enough");
//         return;
//     }
//     memcpy(pixels, logo_en_240x240_lcd, (logo_en_240x240_lcd_width * logo_en_240x240_lcd_height) * sizeof(uint16_t));
//     g_lcd.draw_bitmap(0, 0, logo_en_240x240_lcd_width, logo_en_240x240_lcd_height, (uint16_t *)pixels);
//     heap_caps_free(pixels);
// }


