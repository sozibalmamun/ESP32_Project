
#include "mylvgl.h"


SemaphoreHandle_t xGuiSemaphore;

void lv_tick_task(void *arg) // LVGL tick task
{
   (void)arg;
   lv_tick_inc(10);
}





static void updateIcon(lv_obj_t *labelWIFI, lv_obj_t *labelWBLE){


    if(wifiStatus==1){
        
        lv_obj_set_style_text_color(labelWIFI, networkAvaiable?lv_color_white():lv_color_red(), 0); // White text
        lv_obj_set_style_text_color(labelWBLE, lv_color_black(), 0); // White text
        
    }else{

        lv_obj_set_style_text_color(labelWIFI, lv_color_black(), 0); // White text
        lv_obj_set_style_text_color(labelWBLE, lv_color_white(), 0); // White text
    }



    
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

   // Create a label for sec
   lv_obj_t * sec_label = lv_label_create(lv_scr_act());
   lv_obj_set_style_text_color(sec_label, lv_color_sec(), 0); // White text
   lv_label_set_text(sec_label, "12");
   lv_obj_align(sec_label, LV_ALIGN_CENTER, 77, 25); // Align the time at the center

   lv_style_t secStyle ;
   lv_style_init(&secStyle);
   lv_style_set_text_font(&secStyle,&sec); 
   lv_obj_add_style(sec_label, &secStyle, 0);

   // Create a label for hour
   lv_obj_t * hour_label = lv_label_create(lv_scr_act());
   lv_obj_set_style_text_color(hour_label, lv_color_hour(), 0); // White text
   lv_label_set_text(hour_label, "12");
   lv_obj_align(hour_label, LV_ALIGN_CENTER, 0, -85); // Align the time at the center

   lv_style_t hourStyle;
   lv_style_init(&hourStyle);
   lv_style_set_text_font(&hourStyle,&watch); 
   lv_obj_add_style(hour_label, &hourStyle, 0);



   // Create a label for min
   lv_obj_t * min_label = lv_label_create(lv_scr_act());
   lv_obj_set_style_text_color(min_label, lv_color_min(), 0); 
   lv_label_set_text(min_label, "12");
   lv_obj_align(min_label, LV_ALIGN_CENTER, 0, 0); // Align the time at the center

   lv_style_t minStyle;
   lv_style_init(&minStyle);
   lv_style_set_text_font(&minStyle,&watch);
   lv_obj_add_style(min_label, &minStyle, 0);



    // // Create a label for the date
    lv_obj_t * date_label = lv_label_create(lv_scr_act());
    lv_obj_set_style_text_color(date_label, lv_color_sec(), 0); // White text
    lv_label_set_text(date_label, "24-09-2024");
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 80); // Align date below the time

   lv_style_t dateStyle;
   lv_style_init(&dateStyle);
   lv_style_set_text_font(&dateStyle,&date); 
   lv_obj_add_style(date_label, &dateStyle, 0);

//wifi icon
   lv_obj_t * labelWIFI = lv_label_create(lv_scr_act());
   lv_obj_set_style_text_color(labelWIFI, lv_color_black(), 0); // White text
   lv_label_set_text(labelWIFI , LV_SYMBOL_WIFI);
   lv_obj_align(labelWIFI, LV_ALIGN_CENTER, 107, -152); // Align date below the time


// ble icon
   lv_obj_t * labelWBLE = lv_label_create(lv_scr_act());
   lv_obj_set_style_text_color(labelWBLE, lv_color_black(), 0); // White text
   lv_label_set_text(labelWBLE , LV_SYMBOL_BLUETOOTH );
   lv_obj_align(labelWBLE, LV_ALIGN_CENTER, 90, -152); // Align date below the time



// // up icon 
//    lv_obj_t * labelUp = lv_label_create(lv_scr_act());
//    lv_obj_set_style_text_color(labelUp, lv_color_sec(), 0); // White text
//    lv_obj_set_style_text_font(labelUp, &lv_font_montserrat_12, 0); 
//    lv_label_set_text(labelUp , LV_SYMBOL_UP );
//    lv_obj_align(labelUp, LV_ALIGN_CENTER, 109, -157); // Align date below the time


// // doun icon
//    lv_obj_t * labelDown = lv_label_create(lv_scr_act());
//    lv_obj_set_style_text_color(labelDown, lv_color_sec(), 0); // White text

//    lv_obj_set_style_text_font(labelDown, &lv_font_montserrat_12, 0); // Set large font for time  lv_font_unscii_8

//    lv_label_set_text(labelDown , LV_SYMBOL_DOWN );
//    lv_obj_align(labelDown, LV_ALIGN_CENTER, 109, -151); // Align date below the time













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

         update_time(hour_label, min_label , sec_label);
         update_date(date_label);      // Update the date label
         if(wifiStatus==1)obtain_and_update_local_time();

         updateIcon(labelWIFI,labelWBLE);
         lv_timer_handler();           // Handle LVGL tasks
         xSemaphoreGive(xGuiSemaphore);// Release semaphore

      }
   }
}

