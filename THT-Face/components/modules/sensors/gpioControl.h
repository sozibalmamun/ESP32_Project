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
//-------------------------------

#include "globalScope.h"

extern  uint8_t sleepEnable;
extern TaskHandle_t cameraTaskHandler;
extern TaskHandle_t eventTaskHandler;
extern TaskHandle_t recognitionTaskHandler;
extern TaskHandle_t recognitioneventTaskHandler;
extern TaskHandle_t lcdTaskHandler;
extern TaskHandle_t cloudeTaskHandler;


#ifdef __cplusplus
extern "C"
{
#endif

    #define ADC_CHANNEL ADC2_CHANNEL_8   // GPIO19 for ADC2
    #define DEFAULT_VREF 1100           // Default reference voltage in mV (you may need to adjust this)
    #define NO_OF_SAMPLES 64            // Multisampling to improve accuracy

    ledc_channel_config_t ledc_channel;
    // ADC calibration characteristics
    esp_adc_cal_characteristics_t *adc_chars;
    void PwmInt( ledc_channel_config_t *ledc_channel ,gpio_num_t pinNo );
    void interruptInt(void);
    void gpioInt(void);
    void reduce_cpu_frequency();
    void restore_cpu_frequency();
    void configure_dynamic_frequency();
    void list_all_tasks(void);
    void enter_light_sleep(void);
    void init_adc();
    void readBatteryVoltage();

#ifdef __cplusplus
}
#endif