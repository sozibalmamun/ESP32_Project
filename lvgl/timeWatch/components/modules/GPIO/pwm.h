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

#define LCE_BL GPIO_NUM_14


#define BUZZER_GPIO GPIO_NUM_8   // GPIO 8 for passive buzzer
#define BUZZER_CHANNEL LEDC_CHANNEL_0
#define BUZZER_TIMER LEDC_TIMER_0

// Define the GPIO and LEDC settings
#define LEDC_TIMER LEDC_TIMER_0
#define LEDC_MODE LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT // 13-bit resolution
#define LEDC_FREQUENCY 2000 // Frequency in Hz (2 kHz)




#define SLEEP_LCD 5
#define WAKE_LCD 50
#define MIN_BRIGHTNESS (8191)
#define BRIGHTNESS(x)  MIN_BRIGHTNESS-(((MIN_BRIGHTNESS/100)*x))

#define WAKE_STATE (gpio_get_level(GPIO_WAKEUP_BUTTON)==0)
#define CHARGING_STATE (gpio_get_level(BATTERY_CHARGE_STATE)==0)


#ifdef __cplusplus
extern "C"
{
#endif

    void PwmInt(gpio_num_t pinNo);
    void brightness(bool sleep);
    void buzzer_init(void);
    void buzzer_play(uint16_t freq);
    void play_music(uint16_t *sequence, size_t length);



    


#ifdef __cplusplus
}
#endif


