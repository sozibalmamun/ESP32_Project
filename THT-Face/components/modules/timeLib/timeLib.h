#pragma once



#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"

#include "nvs_flash.h"
#include "nvs.h"

#ifdef __cplusplus
extern "C" {
#endif

/***   Sozib al Mamun                                   Ram Map
----------------------------------------------------------------------------------------------------------------------------
| READ | WRITE |  BIT 7    |  BIT 6    |  BIT 5    |  BIT 4    |  BIT 3    |  BIT 2    |  BIT 1    |  BIT 0    | RANGE     |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 81h  | 80h   |  CH       |             10's Seconds          |                    Seconds        |           | 00?59     |
|------|-------|           |-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 83h  | 82h   |   0       |             10's  Minutes         |                    Minutes        |           | 00?59     |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 85h  | 84h   | 1:12/0:24 |   0       | 0AM1PM/20+| 10+  Hour |                     Hour                      | 1?12/0?23 |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 87h  | 86h   |   0       |   0       |        10's Date      |                     Day                       | 1?31      |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 89h  | 88h   |   0       |   0       |   0       |10's Month |                     Month                     | 1?12      |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 8Bh  | 8Ah   |   0       |   0       |   0       |   0       |   0       |                 Week              | 1?7       |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 8Dh  | 8Ch   |             10 Year                           |                     Year                      | 00?99     |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 8Fh  | 8Eh   |  WP       |   0       |   0       |   0       |   0       |   0       |   0       |   0       |  ?        |
|------|-------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|-----------|
| 91h  | 90h   |  TCS      |  TCS      |  TCS      |  TCS      |  DS       |  DS       |  RS       |  RS       |  ?        |
----------------------------------------------------------------------------------------------------------------------------
***/

// Define delays
#define testTime 5
#define DelayRST    testTime
#define DelayIO     testTime
#define CLK_DELAY   testTime
// Define RTC pin connections
#define DS1302_SCLK_PIN 39
#define DS1302_IO_PIN 40
#define DS1302_CE_PIN 38

#define  RTC_WR_SEC_ADDR        0x80
#define  RTC_RD_SEC_ADDR        0x81

#define  RTC_WR_MIN_ADDR        0x82
#define  RTC_RD_MIN_ADDR        0x83

#define  RTC_WR_HOUR_ADDR       0x84
#define  RTC_RD_HOUR_ADDR       0x85

#define  RTC_WR_DAY_ADDR        0x86    // e.g., bit0 ~ bit3: Day Ones. e.g,  1 - 9, bit4 ~ bit 5: Day tens. e.g, (3)0, (2)2, bit6 ~ bit7: 0
#define  RTC_RD_DAY_ADDR        0x87

#define  RTC_WR_MONTH_ADDR      0x88    // e.g., bit0 ~ bit3: 1 - 9, bit4: Month tens. e.g, (1)0, (1)2, bit5 ~ bit7: 0
#define  RTC_RD_MONTH_ADDR      0x89

#define  RTC_WR_WEEKDAY_ADDR    0x8A    // e.g., bit0 ~ bit2: 1~7, bit3 ~ bit7: 0
#define  RTC_RD_WEEKDAY_ADDR    0x8B

#define  RTC_WR_YEAR_ADDR       0x8C
#define  RTC_RD_YEAR_ADDR       0x8D

#define  RTC_WR_WP_ADDR         0x8E	// Write Protection Register
#define  RTC_RD_WP_ADDR         0x8F

#define  RTC_WR_TCS_ADDR        0x90    // Trickle Charge Control Register 0xaf:0.45mA3.6V 0xa0:0mA	or Hardware 3mA
#define  RTC_RD_TCS_ADDR        0x91
#define  RTC_TCS_0_DNRN		0xa0 // no	// 0xa0~0xa3 or 0xac~0xaf
#define  RTC_TCS_1_D2R8K	0xab // 450	uA
#define  RTC_TCS_2_D1R8K	0xa7 // 525	uA
#define  RTC_TCS_3_D2R4K	0xaa // 900	uA
#define  RTC_TCS_4_D1R4K	0xa6 // 1050uA
#define  RTC_TCS_5_D2R2K	0xa9 // 1800uA
#define  RTC_TCS_6_D1R2K	0xa5 // 2100uA

#define  RTC_WR_CLK_BURST_ADDR  0xBE    // Clock Burst Mode 31 bytes
#define  RTC_RD_CLK_BURST_ADDR  0xBF



#define RTC_WR_RAM_ADDR         0xC0
#define RTC_RD_RAM_ADDR         0xC1
// ......total 31 bytes ram
#define RTC_WR_RAM_ADDR         0xFC
#define RTC_RD_RAM_ADDR         0xFD

#define RTC_WR_RAM_BURST_ADDR   0xFE
#define RTC_RD_RAM_BURST_ADDR   0xFF

// GPIO control macros
#define RTC_DATA_WRITE_MODE {gpio_set_direction(DS1302_IO_PIN, GPIO_MODE_OUTPUT); }
#define RTC_DATA_READ_MODE  {gpio_set_direction(DS1302_IO_PIN, GPIO_MODE_INPUT); }
#define RTC_CLK_HIGH        {gpio_set_level(DS1302_SCLK_PIN, 1); }
#define RTC_CLK_LOW         {gpio_set_level(DS1302_SCLK_PIN, 0); }
#define RTC_CS_HIGH         {gpio_set_level(DS1302_CE_PIN, 1); }
#define RTC_CS_LOW          {gpio_set_level(DS1302_CE_PIN, 0); }
#define RTC_DATA_HIGH       {gpio_set_level(DS1302_IO_PIN, 1);}
#define RTC_DATA_LOW        {gpio_set_level(DS1302_IO_PIN, 0);}
#define RTC_DATA_READ       gpio_get_level(DS1302_IO_PIN)

//rtc 
void RtcInit(void);
void RtcSetDate(uint8_t day, uint8_t month, uint8_t weekday, uint8_t year);
void RtcSetTime(uint8_t sec, uint8_t min, uint8_t hour);
void RtcDataRead(uint8_t eRtcDataType);




static const char *NVS_NAMESPACE = "Config";
static const char *TIME_FORMAT_KEY = "Format";
// Time structure
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t weekday;  // 0 = Sunday, ..., 6 = Saturday
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} time_library_time_t;



//get time 12/24 type
uint8_t get_time(time_library_time_t *time, bool is_12);
// Function to calculate the day of the week
uint8_t calculate_day_of_week(uint16_t year, uint8_t month, uint8_t day);
// Function to calculate the elapsed time in milliseconds
uint32_t time_library_elapsed_time_ms(uint32_t start_time);
// Function to get the current time in milliseconds
uint32_t time_library_get_time_ms(void);

// Function to initialize the time library
void time_library_init(time_library_time_t *initial_time ,bool rtcUpdate);
// Function to set the current time manually
void  time_library_set_time(time_library_time_t *time ,bool rtcUpdate);
// Function to get the current time
void time_library_get_time(time_library_time_t *time);
void save_time_format(bool is_12_hour);

#ifdef __cplusplus
}
#endif
