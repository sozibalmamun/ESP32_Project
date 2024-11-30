// 禾木科技——LVGL Window 窗口控件
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

static void event_handler(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    LV_UNUSED(obj);
    ESP_LOGI(TAG,"Button %d clicked", (int)lv_obj_get_index(obj));
}

void lv_example_win_1(void)
{
    lv_obj_t * win = lv_win_create(lv_scr_act(), 40);// 在主屏幕上创建一个窗口
    lv_obj_t * btn;
    btn = lv_win_add_btn(win, LV_SYMBOL_LEFT, 40);
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);

    lv_win_add_title(win, "A title");// 设置窗口标题

    btn = lv_win_add_btn(win, LV_SYMBOL_RIGHT, 40);// 添加一个设置按钮
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);// 设置按钮事件调回函数

    btn = lv_win_add_btn(win, LV_SYMBOL_CLOSE, 60);// 添加一个关闭并使用win内置的关闭回调操作
    lv_obj_add_event_cb(btn, event_handler, LV_EVENT_CLICKED, NULL);// 设置按钮事件调回函数 lv_win_close_event_cb是内部回调不用定义

    lv_obj_t * cont = lv_win_get_content(win);  /*Content can be added here*/
    lv_obj_t * label = lv_label_create(cont);// 在窗口控件中创建一个标签
    lv_label_set_text(label, "This is\n"// 设置标签内容
                      "a pretty\n"
                      "long text\n"
                      "to see how\n"
                      "the window\n"
                      "becomes\n"
                      "scrollable.\n"
                      "\n"
                      "\n"
                      "Some more\n"
                      "text to be\n"
                      "sure it\n"
                      "overflows. :)");


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

  lv_example_win_1();
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
