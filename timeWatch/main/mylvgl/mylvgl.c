

#include "mylvgl.h"


SemaphoreHandle_t xGuiSemaphore;

void lv_tick_task(void *arg) // LVGL tick task
{
   (void)arg;
   lv_tick_inc(10);
}


 void lvgl_task(void *arg) // GUI task
{
   xGuiSemaphore = xSemaphoreCreateMutex(); // Create GUI semaphore
   lv_init();                               // Initialize LVGL
   lvgl_driver_init();                      // Initialize display driver

   // Initialize display buffer
   static lv_disp_draw_buf_t draw_buf;
   lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_DMA);
   lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_DMA);
   lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * LV_VER_RES_MAX);

   static lv_disp_drv_t disp_drv;
   lv_disp_drv_init(&disp_drv);
   disp_drv.draw_buf = &draw_buf;
   disp_drv.flush_cb = disp_driver_flush;
   disp_drv.hor_res = 240; // Horizontal resolution
   disp_drv.ver_res = 320; // Vertical resolution
   lv_disp_drv_register(&disp_drv);

   // Create a periodic timer for LVGL ticks
   const esp_timer_create_args_t periodic_timer_args = {
       .callback = &lv_tick_task,
       .name = "periodic_gui"};
   esp_timer_handle_t periodic_timer;
   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
   ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));

    // Initialize the screen UI
   // Set black background color for the screen
   lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);

//===============================================


    lv_obj_t * hour_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(hour_label, lv_color_hour(), 0); // White text
    lv_label_set_text(hour_label, "12");
    lv_obj_align(hour_label, LV_ALIGN_CENTER, 0, -85); // Align the time at the center

   lv_style_t hourStyle;
   lv_style_init(&hourStyle);
   lv_style_set_text_font(&hourStyle,&watch); //引用字库
   lv_obj_add_style(hour_label, &hourStyle, 0);



    // Create a label for min
    lv_obj_t * min_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(min_label, lv_color_min(), 0); // White text
    lv_label_set_text(min_label, "12");
    lv_obj_align(min_label, LV_ALIGN_CENTER, 0, 0); // Align the time at the center

   lv_style_t minStyle;
   lv_style_init(&minStyle);
   lv_style_set_text_font(&minStyle,&watch); //引用字库
   lv_obj_add_style(min_label, &minStyle, 0);


    // // // Create a label for time
    // lv_obj_t * sec_label = lv_label_create(lv_scr_act());
    // lv_obj_set_style_text_color(sec_label, lv_color_white(), 0); // White text
    // lv_obj_set_style_text_font(sec_label, &lv_font_montserrat_16, 0); // Set large font for time  lv_font_unscii_8
    // lv_label_set_text(sec_label, "12");
    // lv_obj_align(sec_label, LV_ALIGN_CENTER, 79, -104); // Align the time at the center



    lv_obj_t * sec_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(sec_label, lv_color_sec(), 0); // White text
    lv_label_set_text(sec_label, "12");
    lv_obj_align(sec_label, LV_ALIGN_CENTER, 79, 0); // Align the time at the center

    lv_style_t secStyle ;
    lv_style_init(&secStyle);
    lv_style_set_text_font(&secStyle,&watch1); 
    lv_obj_add_style(sec_label, &secStyle, 0);



//    lv_style_t segment;
//    lv_style_init(&segment);
//    lv_style_set_text_font(&segment,&tft7segment);
//    lv_obj_add_style(time_label, &segment, 0);

//========================================================
    // Create a label for the AM/PM indicator
    lv_obj_t * am_pm_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(am_pm_label, lv_color_white(), 0); // White text
    lv_obj_set_style_text_font(am_pm_label, &lv_font_montserrat_12, 0); // Set large font for time  lv_font_unscii_8
    lv_label_set_text(am_pm_label, "PM");
    lv_obj_align(am_pm_label, LV_ALIGN_CENTER, 80, -126); // Align the AM/PM beside the time

    // // Create a label for the date
    // lv_obj_t * date_label = lv_label_create(lv_scr_act());
    // lv_obj_set_style_text_color(date_label, lv_color_white(), 0); // White text
    // lv_label_set_text(date_label, "24-09-2024");
    // lv_obj_align(date_label, LV_ALIGN_CENTER, 0, -60); // Align date below the time


    // Create a label for the weather (optional)
    // lv_obj_t * weather_label = lv_label_create(main_screen);
    // lv_obj_set_style_text_color(weather_label, lv_color_white(), 0); // White text

    // lv_label_set_text(weather_label, "25°C ☀️");
    // lv_obj_align(weather_label, LV_ALIGN_CENTER, 0, 70); // Align weather info below the date


   while (1)
   {
      vTaskDelay(pdMS_TO_TICKS(1000)); // Delay 1 second

      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {

        update_time(hour_label, min_label , sec_label  ,am_pm_label);
        // update_date(date_label);      // Update the date label

        obtain_and_update_local_time() ;
        lv_timer_handler();           // Handle LVGL tasks
        xSemaphoreGive(xGuiSemaphore);// Release semaphore
      }
   }
}
