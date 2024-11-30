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
extern volatile TickType_t sleepTimeOut; 

union shiftResistorBitfild {
    struct PACKED_STRUCT bit {
        uint8_t CAMEN   : 1;
        uint8_t MSDA    : 1;
        uint8_t MSCL    : 1;
        uint8_t PEREN   : 1;
        uint8_t LED     : 1;
        uint8_t LCDEN   : 1;
        uint8_t CAMPDWN : 1;
        uint8_t IRLED   : 1;
    } bitset;
    
    // Alternative way to access the same 8-bit memory space
    uint8_t read;
    uint8_t write;

};

union shiftResistorBitfild shiftOutData;




extern TaskHandle_t cameraTaskHandler;
extern TaskHandle_t eventTaskHandler;
extern TaskHandle_t recognitionTaskHandler;
extern TaskHandle_t recognitioneventTaskHandler;
extern TaskHandle_t lcdTaskHandler;
extern TaskHandle_t cloudeTaskHandler;


#define ADC_CHANNEL ADC2_CHANNEL_8   // GPIO19 for ADC2
#define DEFAULT_VREF 1100           // Default reference voltage in mV (you may need to adjust this)
#define NO_OF_SAMPLES 64            // Multisampling to improve accuracy

#define CHARGE_IDLE         0   
#define CHARGER_PLUGED      1  
#define CHARGEING           2  
#define UN_PLUGING          3  
#define CHARGING_UN_PLUGED  4    



//Sleep config---------------------------------------------------
#define MAX_FREQ   240    // Maximum frequency
#define MIN_FREQ   40
#define GPIO_WAKEUP_BUTTON GPIO_NUM_0
#define BATTERY_CHARGE_STATE GPIO_NUM_20




#define CAMP_DWN GPIO_NUM_3 
#define LCE_BL GPIO_NUM_14
// shift registor
#define SER_SDI GPIO_NUM_3
#define SER_CLK GPIO_NUM_45
#define SER_LAT GPIO_NUM_14



#define ESP_INTR_FLAG_DEFAULT 0
#define SLEEP_LCD 5
#define WAKE_LCD 70
#define MIN_BRIGHTNESS (8191)
#define BRIGHTNESS(x)  MIN_BRIGHTNESS-(((MIN_BRIGHTNESS/100)*x))

#define WAKE_STATE (gpio_get_level(GPIO_WAKEUP_BUTTON)==0)
#define CHARGING_STATE (gpio_get_level(BATTERY_CHARGE_STATE)==0)
//-------------sleep config file end------------------------------

#ifdef __cplusplus
extern "C"
{
#endif


    // ADC calibration characteristics
    esp_adc_cal_characteristics_t *adc_chars;
    // void PwmInt( ledc_channel_config_t *ledc_channel ,gpio_num_t pinNo );
    void PwmInt(gpio_num_t pinNo );

    void brightness(bool sleep);
    void interruptInt(void);
    void gpioInt(void);
    void reduce_cpu_frequency();
    void restore_cpu_frequency();
    void configure_dynamic_frequency();
    // void list_all_tasks(void);
    void enter_light_sleep(void);
    void init_adc();
    void readBatteryVoltage();
    void plugIn(bool plugin);

#ifdef __cplusplus
}
#endif