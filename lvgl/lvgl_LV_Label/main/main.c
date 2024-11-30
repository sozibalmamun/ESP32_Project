 //禾木科技——Label 标签控件例程
//https://hemukeji.taobao.com/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "lvgl/demos/lv_demos.h"
#include "lvgl_helpers.h"
#include "esp_freertos_hooks.h"

SemaphoreHandle_t xGuiSemaphore;// 创建一个GUI信号量
static void lv_tick_task(void *arg)// LVGL 时钟任务
{
   (void)arg;
   lv_tick_inc(10);
}



static void lvgl_task(void *arg)// GUI任务
{
   xGuiSemaphore = xSemaphoreCreateMutex();// 创建GUI信号量
   lv_init();          //lvgl初始化
   lvgl_driver_init(); // 初始化液晶驱动

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
   
	/* 创建一个10ms定时器*/// 定期处理GUI回调
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &lv_tick_task,
		.name = "periodic_gui"};
	esp_timer_handle_t periodic_timer;
	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000));

   //TEST
   lv_obj_t * label = lv_label_create(lv_scr_act());     // Create a tab on the home screen
   //lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);		      // Align to center

   lv_label_set_recolor(label, true);							// 使能字符命令重新对字符上色
	lv_obj_set_align(label, LV_FLEX_ALIGN_CENTER);			// 内容居中对齐
   lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);		// 标签长内容框，保持控件宽度，内容过长就换行

   lv_label_set_text(label, "#0000ff S##ff00ff O##ff0000 Z##ff00ED I#B #ff0000 LVGL#");
	lv_obj_align(label, LV_ALIGN_CENTER, 0, -60);		// 对齐到中心偏上
           
   lv_obj_t * label2 = lv_label_create(lv_scr_act());	//Create a tab on the home screen
	// Label long content box, maintain the width of the control, if the content is too long, it will scroll in a loop
   lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);	
	lv_obj_set_width(label2, 150);								// 设置标签宽度
	lv_label_set_text(label2, "THT SPACE ELECTRICAL COMPANY LTD ");// 设置显示文本
	lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);			// 对齐到中心偏下



/*sozib
   static lv_obj_t *default_src;
   default_src = lv_scr_act();		//获取默认屏幕
   lv_obj_t * label = lv_label_create(default_src);          /// 在主屏幕创建一个标签
   lv_label_set_recolor(label, true);							// 使能字符命令重新对字符上色
	lv_obj_set_align(label, LV_FLEX_ALIGN_CENTER);			// 内容居中对齐
   lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);		// 标签长内容框，保持控件宽度，内容过长就换行
   	// 设置显示文本（其中含颜色命令 #颜色上色内容# ）
	lv_label_set_text(label, "#0000ff lvgl# #ff00ff LV_Label# #ff0000 Label# label "
							"lvgl_LV_Label");
	lv_obj_set_width(label, 150);								// 设置标签宽度
	lv_obj_align(label, LV_ALIGN_CENTER, 0, -60);		// 对齐到中心偏上
	///////////////Label1 演示长文本动态滚动显示/////////
	lv_obj_t * label2 = lv_label_create(lv_scr_act());	// 在主屏幕创建一个标签
	lv_label_set_long_mode(label2, LV_LABEL_LONG_SCROLL_CIRCULAR);	// 标签长内容框，保持控件宽度，内容过长循环滚动
	lv_obj_set_width(label2, 150);								// 设置标签宽度
	lv_label_set_text(label2, "lvgl_LV_Labellvgl_LV_Labellvgl_LV_Labellvgl_LV_Label ");// 设置显示文本
	lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);			// 对齐到中心偏下
*/
   while (1)
   {
      /* Delay 1 tick (assumes FreeRTOS tick is 10ms */
      vTaskDelay(pdMS_TO_TICKS(10));

      /* Try to take the semaphore, call lvgl related function on success */
      if (pdTRUE == xSemaphoreTake(xGuiSemaphore, portMAX_DELAY))
      {
         lv_timer_handler();// 处理LVGL任务
         xSemaphoreGive(xGuiSemaphore);// 释放信号量
      }
   }
}
void app_main(void)
{
//任务创建
   xTaskCreatePinnedToCore(lvgl_task, "gui task", 1024 * 4, NULL, 1, NULL, 0);
}
