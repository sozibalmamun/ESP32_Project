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

// union shiftResistorBitfild {
//     struct PACKED_STRUCT bit {
//         uint8_t CAMEN   : 1;//q0 
//         uint8_t MSDA    : 1;
//         uint8_t MSCL    : 1;
//         uint8_t PEREN   : 1;
//         uint8_t LED     : 1;
//         uint8_t LCDEN   : 1;
//         uint8_t CAMPDWN : 1;
//         uint8_t IRLED   : 1;//q7
//     } bitset;
    
//     // Alternative way to access the same 8-bit memory space
//     uint8_t read;
//     uint8_t write;

// };

union shiftResistorBitfild {
    struct PACKED_STRUCT bit {
        uint8_t CAMPDWN     : 1;//Q0 
        uint8_t UVOFF       : 1;//Q1
        uint8_t LED         : 1;//Q2
        uint8_t IRLED       : 1;//Q3
        uint8_t CAMEN       : 1;//Q4
        uint8_t LCDEN       : 1;//Q5
        uint8_t ADC_EN      : 1;//Q6
        uint8_t PEREN       : 1;//Q7
    } bitset;
    
    // Alternative way to access the same 8-bit memory space
    uint8_t read;
    uint8_t write;

};




// union shiftResistorBitfild shiftOutData;


extern TaskHandle_t cameraTaskHandler;
extern TaskHandle_t eventTaskHandler;
extern TaskHandle_t recognitionTaskHandler;
extern TaskHandle_t recognitioneventTaskHandler;
extern TaskHandle_t lcdTaskHandler;
extern TaskHandle_t cloudeTaskHandler;

  

// battery 
#define BATTERY_ADC_CHANNEL ADC2_CHANNEL_8   // GPIO19 for ADC2
#define DEFAULT_VREF 1100           // Default reference voltage in mV (you may need to adjust this)
#define NO_OF_SAMPLES 64            // Multisampling to improve accuracy
// pir
#define PIR_ADC_CHANNEL ADC1_CHANNEL_0       // GPIO1 for ADC1 



#define CHARGE_IDLE         0   
#define CHARGER_PLUGED      1  
#define CHARGEING           2  
#define UN_PLUGING          3  
#define CHARGING_UN_PLUGED  4    



//Sleep config---------------------------------------------------
#define MAX_FREQ   240    // Maximum frequency
#define MIN_FREQ   40
#define BATTERY_CHARGE_STATE GPIO_NUM_20



#define MUSICPIN GPIO_NUM_46
#define MUSIC_BUSY GPIO_NUM_0

#define PIR GPIO_NUM_1
#define LCE_BL GPIO_NUM_2//
// shift registor
// #define SER_SDI GPIO_NUM_3
// #define SER_CLK GPIO_NUM_45
// #define SER_LAT GPIO_NUM_14
#define SER_SDI GPIO_NUM_45
#define SER_CLK GPIO_NUM_3
#define SER_LAT GPIO_NUM_14



#define ESP_INTR_FLAG_DEFAULT 0
#define SLEEP_LCD 5
#define WAKE_LCD 100
#define MIN_BRIGHTNESS (8191)
#define BRIGHTNESS(x)  MIN_BRIGHTNESS-(((MIN_BRIGHTNESS/100)*x))

#define MUSINC_PLAYING (gpio_get_level(MUSIC_BUSY))
#define CHARGING_STATE (gpio_get_level(BATTERY_CHARGE_STATE)==1)
#define PIR_STATE (gpio_get_level(PIR))



//-------------sleep config file end------------------------------

// esp_adc_cal_characteristics_t *adc_chars;

#ifdef __cplusplus
extern "C"
{
#endif


    // ADC calibration characteristics
    void PwmInt(gpio_num_t pinNo );

    void brightness(bool sleep);

    void dispON(bool dspOn);
    void interruptInt(void);
    void gpioInt(void);
    void reduce_cpu_frequency();
    void restore_cpu_frequency();
    void configure_dynamic_frequency();
    // void list_all_tasks(void);
    void enter_light_sleep(void);
    void init_adc();
    void init_pir();
    void readBatteryVoltage();
    void plugIn(bool plugin);
    void musicPlay(uint8_t musicNo);
    void musicArrayPlay(uint8_t *music ,uint8_t len);
    void sensorHandel();

#ifdef __cplusplus
}
#endif