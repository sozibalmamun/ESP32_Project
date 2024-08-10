#include "timeLib.h"


static const char *TAG = "TimeLib";

time_library_time_t reference_time;
static uint32_t reference_tick_count;
// time_library_time_t current_time;




static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const char* day_names[] = {
    "Sun", "Mon", "Tue", "Wed", "Thu", "Fri" ,"Sat"
};
// Function to check if a year is a leap year
static bool is_leap_year(uint16_t year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

// Function to add seconds to a time structure
static void add_seconds_to_time(time_library_time_t *time, uint32_t seconds) {
    time->second += seconds % 60;
    time->minute += (seconds / 60) % 60;
    time->hour += (seconds / 3600) % 24;
    time->day += seconds / 86400;

    if (time->second >= 60) {
        time->second -= 60;
        time->minute++;
    }
    if (time->minute >= 60) {
        time->minute -= 60;
        time->hour++;
    }
    if (time->hour >= 24) {
        time->hour -= 24;
        time->day++;
    }

    while (true) {
        uint8_t days_in_current_month = days_in_month[time->month - 1];
        if (time->month == 2 && is_leap_year(time->year)) {
            days_in_current_month = 29;
        }
        if (time->day <= days_in_current_month) {
            break;
        }
        time->day -= days_in_current_month;
        time->month++;
        if (time->month > 12) {
            time->month = 1;
            time->year++;
        }
    }
}

// Initialize the time library with a known time
void time_library_init(time_library_time_t *initial_time) {
    reference_time = *initial_time;
    reference_tick_count = xTaskGetTickCount();
    ESP_LOGI(TAG, "Time library initialized with reference time: %d-%d-%d %d:%d:%d",
             reference_time.year, reference_time.month, reference_time.day,
             reference_time.hour, reference_time.minute, reference_time.second);
}

// Set the current time manually
void time_library_set_time(time_library_time_t *time) {
    time_library_init(time);
}

// Get the current time
void time_library_get_time(time_library_time_t *current_time) {
    uint32_t elapsed_ticks = xTaskGetTickCount() - reference_tick_count;
    uint32_t elapsed_seconds = (elapsed_ticks * portTICK_PERIOD_MS) / 1000;
    *current_time = reference_time;
    add_seconds_to_time(current_time, elapsed_seconds);
}

// Calculate the elapsed time in milliseconds
uint32_t time_library_elapsed_time_ms(uint32_t start_time) {
    uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return current_time_ms - start_time;
}

// Get the current time in 12-hour format
uint8_t  get_time(time_library_time_t *time, bool is_12) {

    uint8_t PM=0;
    if(!is_12){
        time_library_get_time(time);
        return PM;// 24 hour type clock

    }
    time_library_get_time(time);

    if (time->hour == 0) {
        time->hour = 12; // Midnight
    } else if (time->hour > 12) {
        PM=1; //AM
        time->hour -= 12; // Convert to 12-hour format
    }else if(time->hour < 12){
        PM=2;// PM
    }
    return PM;

}

// Function to calculate the day of the week using Zeller's Congruence
uint8_t calculate_day_of_week(uint16_t year, uint8_t month, uint8_t day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    uint8_t q = day;
    uint8_t m = month;
    uint16_t k = year % 100;
    uint16_t j = year / 100;
    uint8_t day_of_week = (q + 13 * (m + 1) / 5 + k + k / 4 + j / 4 - 2 * j) % 7;
    return (day_of_week + 6) % 7;
}