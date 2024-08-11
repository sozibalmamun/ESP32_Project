#pragma once



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
//get time 12/24 type
uint8_t get_time(time_library_time_t *time, bool is_12);
// Function to calculate the day of the week
uint8_t calculate_day_of_week(uint16_t year, uint8_t month, uint8_t day);
// Function to calculate the elapsed time in milliseconds
uint32_t time_library_elapsed_time_ms(uint32_t start_time);
// Function to get the current time in milliseconds
uint32_t time_library_get_time_ms(void);

#ifdef __cplusplus
}
#endif
