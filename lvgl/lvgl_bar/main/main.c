// 禾木科技——bar进度指示控件使用例程
// https://hemukeji.taobao.com/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"

SemaphoreHandle_t xGuiSemaphore;    // 创建一个GUI信号量
static void lv_tick_task(void *arg) // LVGL 时钟任务
{
   (void)arg;
   lv_tick_inc(10);
}
void lv_example_bar_1(void)
{
   lv_obj_t *bar1 = lv_bar_create(lv_scr_act()); // 创建Bar对象
   lv_obj_set_size(bar1, 200, 20);               // 设置宽为200，高为20
   lv_obj_center(bar1);                          // 居中显示
   lv_bar_set_value(bar1, 70, LV_ANIM_OFF);      // 设置进度值为70,动画效果为OFF
}
static void lv_example_bar_2(void)
{
   static lv_style_t style_bg;    // 背景style
   static lv_style_t style_indic; // 指示器style

   lv_obj_t *bar = lv_bar_create(lv_scr_act()); // 创建Bar
   lv_obj_center(bar);                          // 居中显示
   lv_obj_set_size(bar, 200, 20);               // 设置大小
   //  lv_obj_remove_style_all(bar); /*To have a clean start*/
   lv_style_init(&style_bg);                                               // 初始化背景style
   lv_style_set_border_color(&style_bg, lv_palette_main(LV_PALETTE_BLUE)); // 设置背景色
   lv_style_set_border_width(&style_bg, 2);                                // 设置边框宽度
   lv_style_set_pad_all(&style_bg, 6);                                     // 设置样式内部padding填充值
   lv_obj_add_style(bar, &style_bg, 0);                                    // 添加背景style

   lv_style_set_radius(&style_bg, 6);                                     // 设置倒角
   lv_style_set_anim_time(&style_bg, 1000);                               // 设置动画时间
   lv_style_init(&style_indic);                                           // 初始指示器style
   lv_style_set_bg_opa(&style_indic, LV_OPA_COVER);                       // 设置背景透明度
   lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE)); // 设置背景色
   lv_style_set_radius(&style_indic, 3);                                  // 设置指示器倒角
   lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);                // 添加指示器style

   lv_bar_set_value(bar, 100, LV_ANIM_ON); // 设置进度值为100，开启动画特性
}
static void set_temp(void* bar, int32_t temp)
{
    lv_bar_set_value(bar, temp, LV_ANIM_ON); //设置Bar进度值
}

static void lv_example_bar_3(void)
{
    static lv_style_t style_indic; //指示器style
    lv_style_init(&style_indic);
    lv_style_set_bg_opa(&style_indic, LV_OPA_COVER); //透明度为LV_OPA_COVER
    lv_style_set_bg_color(&style_indic, lv_palette_main(LV_PALETTE_RED)); //指示器背景为LV_PALETTE_RED
    lv_style_set_bg_grad_color(&style_indic, lv_palette_main(LV_PALETTE_BLUE));//指示器渐变色为LV_PALETTE_BLUE
    lv_style_set_bg_grad_dir(&style_indic, LV_GRAD_DIR_VER);//渐变色方向为LV_GRAD_DIR_VER
    lv_obj_t* bar = lv_bar_create(lv_scr_act()); //创建Bar
    lv_obj_add_style(bar, &style_indic, LV_PART_INDICATOR);//添加style
    lv_obj_set_size(bar, 20, 200);//设置大小
    lv_obj_center(bar); //居中显示
    lv_bar_set_range(bar, -20, 40);//设置Bar最小值为-20，最大值为40
    lv_anim_t a;
    lv_anim_init(&a); 
    lv_anim_set_exec_cb(&a, set_temp);//设置动画执行回调set_temp
    lv_anim_set_time(&a, 3000); //动画时间为3000毫秒
    lv_anim_set_playback_time(&a, 3000);//回播时间为3000毫秒
    lv_anim_set_var(&a, bar); //设置动画执行对象为bar
    lv_anim_set_values(&a, -20, 40); //动画播放start为-20，end为40
    lv_anim_set_repeat_count(&a, LV_ANIM_REPEAT_INFINITE);//无限重复执行
    lv_anim_start(&a);//启动动画
}

static void lvgl_task(void *arg) // GUI任务
{
   xGuiSemaphore = xSemaphoreCreateMutex(); // 创建GUI信号量
   lv_init();                               // lvgl初始化
   lvgl_driver_init();                      // 初始化液晶驱动

   /* Example for 1) */
   static lv_disp_draw_buf_t draw_buf;
   // 初始化缓存
   lv_color_t *buf1 = heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_DMA);
   lv_color_t *buf2 = heap_caps_malloc(DISP_BUF_SIZE * 2, MALLOC_CAP_DMA);

   // 添加并注册触摸驱动
   lv_disp_draw_buf_init(&draw_buf, buf1, buf2, LV_HOR_RES_MAX * LV_VER_RES_MAX); /*Initialize the display buffer*/

   static lv_disp_drv_t disp_drv;         /*A variable to hold the drivers. Must be static or global.*/
   lv_disp_drv_init(&disp_drv);           /*Basic initialization*/
   disp_drv.draw_buf = &draw_buf;         /*Set an initialized buffer*/
   disp_drv.flush_cb = disp_driver_flush; /*Set a flush callback to draw to the display*/
   disp_drv.hor_res = 320;                /*Set the horizontal resolution in pixels*/
   disp_drv.ver_res = 240;                /*Set the vertical resolution in pixels*/
   lv_disp_drv_register(&disp_drv);       /*Register the driver and save the created display objects*/
                                          /*触摸屏输入接口配置*/
   lv_indev_drv_t indev_drv;
   lv_indev_drv_init(&indev_drv);
   indev_drv.read_cb = touch_driver_read;
   indev_drv.type = LV_INDEV_TYPE_POINTER;
   lv_indev_drv_register(&indev_drv);

   /* 创建一个10ms定时器*/ // 定期处理GUI回调
   const esp_timer_create_args_t periodic_timer_args = {
       .callback = &lv_tick_task,
       .name = "periodic_gui"};
   esp_timer_handle_t periodic_timer;
   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
   ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
   // lv_example_bar_1();
   // lv_example_bar_2();
   lv_example_bar_3();
   while (1)
   {
      /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
      vTaskDelay(pdMS_TO_TICKS(10));

      /* Try to take the semaphore, call lvgl related function on success */
      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {
         lv_timer_handler();            // 处理LVGL任务
         xSemaphoreGive(xGuiSemaphore); // 释放信号量
      }
   }
}
void app_main(void)
{
   // 任务创建
   xTaskCreatePinnedToCore(lvgl_task, "gui task", 1024 * 4, NULL, 1, NULL, 0);
}
