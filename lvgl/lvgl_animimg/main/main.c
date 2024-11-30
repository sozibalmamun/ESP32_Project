// 禾木科技——LVGL animimg动画控件
// lvgl v9
// https://hemukeji.taobao.com/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"
#include "esp_log.h"
#define TAG " https://hemukeji.taobao.com"

SemaphoreHandle_t xGuiSemaphore;    // 创建一个GUI信号量
static void lv_tick_task(void *arg) // LVGL 时钟任务
{
   (void)arg;
   lv_tick_inc(10);
}
LV_IMG_DECLARE(animimg001)
LV_IMG_DECLARE(animimg002)
LV_IMG_DECLARE(animimg003)

static const lv_img_dsc_t *anim_imgs[3] = {
    &animimg001,
    &animimg002,
    &animimg003,
};

void lv_example_animimg_1(void)
{
   lv_obj_t *animimg0 = lv_animimg_create(lv_scr_act());//Create animation object
   lv_obj_center(animimg0);//Center the object on the screen
   lv_animimg_set_src(animimg0, (lv_img_dsc_t **)anim_imgs, 3);//Loading animation resources
   lv_animimg_set_duration(animimg0, 1000);//Create animation time
   lv_animimg_set_repeat_count(animimg0, LV_ANIM_REPEAT_INFINITE); //Set the repeat time
   lv_animimg_start(animimg0);//Open
}

static void lvgl_task(void *arg) // GUI任务
{
   xGuiSemaphore = xSemaphoreCreateMutex(); // Create GUI signal
   lv_init();                               // Initialize lvgl
   lvgl_driver_init();                      // Initialize the LCD driver

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

/* Create a 10ms timer*/ // Timer handle GUI回调
   const esp_timer_create_args_t periodic_timer_args = {
       .callback = &lv_tick_task,
       .name = "periodic_gui"};
   esp_timer_handle_t periodic_timer;
   ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
   ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));
   lv_example_animimg_1();

       while (1)
   {
      /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
      vTaskDelay(pdMS_TO_TICKS(10));

      /* Try to take the semaphore, call lvgl related function on success */
      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {
         lv_timer_handler();            // Handle LVGL tasks
         xSemaphoreGive(xGuiSemaphore); // Release signal
      }
   }
}
void app_main(void)
{
   // Task creation
   xTaskCreatePinnedToCore(lvgl_task, "gui task", 1024 * 4, NULL, 1, NULL, 0);
}
