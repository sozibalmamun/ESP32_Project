#pragma once




#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "esp_event_loop.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "driver/adc.h"
#include "esp_adc_cal.h"
//-------------------------------
#include "esp_sleep.h"
#include "esp_pm.h"
#include "esp_clk.h"
#include "driver/gpio.h"

//------------------------------
#include "driver/ledc.h"
#include "driver/mcpwm.h"
//------------------------

#define LCE_BL GPIO_NUM_13
#define SLEEP_LCD 5
#define WAKE_LCD 20
#define MIN_BRIGHTNESS (8191)
#define BRIGHTNESS(x)  MIN_BRIGHTNESS-(((MIN_BRIGHTNESS/100)*x))

#define WAKE_STATE (gpio_get_level(GPIO_WAKEUP_BUTTON)==0)
#define CHARGING_STATE (gpio_get_level(BATTERY_CHARGE_STATE)==0)


#ifdef __cplusplus
extern "C"
{
#endif

    void PwmInt(gpio_num_t pinNo );

    void brightness(bool sleep);



#ifdef __cplusplus
}
#endif


