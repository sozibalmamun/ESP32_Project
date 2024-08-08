#ifndef TIME_H
#define TIME_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"



#ifdef __cplusplus
extern "C" {
#endif


// Time structure
typedef struct {
    uint16_t year;
    uint8_t month;
    uint8_t day;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
} time_library_time_t;


// Function to initialize the time library
void time_library_init(time_library_time_t *initial_time);

// Function to set the current time manually
void time_library_set_time(time_library_time_t *time);

// Function to get the current time
void time_library_get_time(time_library_time_t *time);

// Function to get the current time in milliseconds
uint32_t time_library_get_time_ms(void);

// Function to get the current time in seconds
uint32_t time_library_get_time_s(void);

// Function to get the current time in minutes
uint32_t time_library_get_time_min(void);

// Function to get the current time in hours
uint32_t time_library_get_time_hr(void);

// Function to get the current time in days
uint32_t time_library_get_time_day(void);

// Function to get the current time in months
uint32_t time_library_get_time_month(void);

// Function to get the current time in years
uint32_t time_library_get_time_year(void);

// Function to calculate the elapsed time in milliseconds
uint32_t time_library_elapsed_time_ms(uint32_t start_time);

// extern dsp_time_t timeNow;
// extern dsp_time_t dispTime();


#ifdef __cplusplus
}
#endif

#endif // TIME_LIBRARY_H
