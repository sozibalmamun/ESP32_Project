#include "timeLib.h"
#include "globalScope.h"

static const char *TAG = "TimeLib";

time_library_time_t reference_time;
static uint32_t reference_tick_count;
bool dspTimeFormet;

static const uint8_t days_in_month[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
const DATA_FLASH char* day_names[] = {
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
void time_library_set_time(time_library_time_t *initial_time, bool rtcUpdate) {

    reference_time = *initial_time;
    reference_tick_count = xTaskGetTickCount();
    ESP_LOGI(TAG, "initialized with reference time: %d-%d-%d %d %d:%d:%d",
             reference_time.year, reference_time.month, reference_time.day, reference_time.weekday,
             reference_time.hour, reference_time.minute, reference_time.second);

    if(rtcUpdate){
        RtcSetTime(reference_time.second, reference_time.minute, reference_time.hour); // Set time: 12:45:30
        RtcSetDate(reference_time.day, reference_time.month, reference_time.weekday, reference_time.year); // Set date: 23rd Oct, 2024, Weekday = 4 (Wednesday)
    }
	
}

// Set the current time manually
// void time_library_set_time(time_library_time_t *time ,bool rtcUpdate) {
//     time_library_init(time ,rtcUpdate);
// }
// Get the current time
// void time_library_get_time(time_library_time_t *current_time) {
//     uint32_t elapsed_ticks = xTaskGetTickCount() - reference_tick_count;
//     uint32_t elapsed_seconds = (elapsed_ticks * portTICK_PERIOD_MS) / 1000;
//     *current_time = reference_time;
//     add_seconds_to_time(current_time, elapsed_seconds);
// }


void time_library_get_time(time_library_time_t *current_time) {
    // Get the number of ticks that have elapsed since reference
    uint32_t elapsed_ticks = xTaskGetTickCount() - reference_tick_count;
    uint32_t elapsed_seconds = (elapsed_ticks * portTICK_PERIOD_MS) / 1000;

    // Copy reference time as the starting point
    *current_time = reference_time;
    
    // Add elapsed seconds to update the time
    add_seconds_to_time(current_time, elapsed_seconds);

    // Calculate the elapsed days
    uint32_t elapsed_days = elapsed_seconds / 86400;

    // Update the weekday based on elapsed days
    current_time->weekday = (reference_time.weekday + elapsed_days) % 7;
}

// Calculate the elapsed time in milliseconds
uint32_t time_library_elapsed_time_ms(uint32_t start_time) {
    uint32_t current_time_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;
    return current_time_ms - start_time;
}

// Get the current time in 12-hour format
uint8_t  get_time(time_library_time_t *time, bool is_12) {

    uint8_t PM=0;
    time_library_get_time(time);

    if(!is_12)return PM;// 24 hour type clock
    else{

        if (time->hour == 0) {
            time->hour = 12; // Midnight
        } else if (time->hour > 12) {
            PM=1; //PM
            time->hour -= 12; // Convert to 12-hour format
        }else if(time->hour < 12){
            PM=2;// am
        }
        return PM;

    }
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

// BCD to decimal and vice versa conversions
uint8_t BCD2Dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

uint8_t Dec2BCD(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// RTC Control Functions
void RTCStart(void) {
	RTC_CLK_LOW
	RTC_DATA_LOW
	RTC_CS_HIGH ets_delay_us(DelayRST);
}

void RTCStop(void) {
	RTC_CS_LOW;  
    ets_delay_us(DelayRST);
	RTC_CLK_LOW
	RTC_DATA_LOW
}

// Reading and writing bytes to the DS1302
uint8_t RTCReadByte(void) {
	uint8_t c8c2 ,ByteData = 0;

    RTC_DATA_READ_MODE;ets_delay_us(DelayIO);

	for(c8c2 = 0x01; c8c2; c8c2<<=1)
	{
		if(RTC_DATA_READ)ByteData |= c8c2;	
        
        RTC_CLK_HIGH    ets_delay_us(CLK_DELAY);
        RTC_CLK_LOW     ets_delay_us(CLK_DELAY);
	}

	return ByteData;
}

void RTCWriteByte(uint8_t ByteData) {
	uint8_t c8c2;
    RTC_DATA_WRITE_MODE;ets_delay_us(DelayIO);

	for(c8c2 = 0x01; c8c2; c8c2<<=1)
	{
		if(ByteData&c8c2) RTC_DATA_HIGH 
		else RTC_DATA_LOW
			
        ets_delay_us(DelayIO);
		RTC_CLK_HIGH ets_delay_us(CLK_DELAY);
		RTC_CLK_LOW  ets_delay_us(CLK_DELAY);
	}

}

// Reading specific data from RTC
void RtcDataRead(uint8_t eRtcDataType) {
    uint8_t data;

    RTCStart();
    RTCWriteByte(eRtcDataType);
    data = BCD2Dec(RTCReadByte());
    RTCStop();

    switch (eRtcDataType) {
        case RTC_RD_SEC_ADDR: printf("Seconds: %d", data); break;
        case RTC_RD_MIN_ADDR: printf(" Minutes: %d", data); break;
        case RTC_RD_HOUR_ADDR: printf(" Hours: %d", data); break;
        case RTC_RD_DAY_ADDR: printf(" Day: %d", data); break;
        case RTC_RD_MONTH_ADDR: printf(" Month: %d", data); break;
        case RTC_RD_WEEKDAY_ADDR: printf( " Weekday: %d", data); break;
        case RTC_RD_YEAR_ADDR: printf(" Year: %d\n", data + 2000); break;
        default: printf("Unknown data type\n"); break;
    }
}

// Writing specific data to RTC
void RtcDataWrite(uint8_t eRtcDataType, uint8_t data) {
    // uint8_t bcdData = Dec2BCD(data);
    // RTCStart();
    // RTCWriteByte(eRtcDataType);
    // RTCWriteByte(bcdData);
    // RTCStop();

    RTCStart();
    RTCWriteByte(eRtcDataType);
    RTCWriteByte(data);
    RTCStop();


}

// Set time in the RTC
void RtcSetTime(uint8_t sec, uint8_t min, uint8_t hour) {


    uint8_t bcdData = Dec2BCD(sec);
    RtcDataWrite(RTC_WR_SEC_ADDR, (bcdData &= ~0x80));
    ets_delay_us(DelayIO);

    bcdData = Dec2BCD(min);
    RtcDataWrite(RTC_WR_MIN_ADDR, (bcdData &= ~0x80));
    ets_delay_us(DelayIO);

    bcdData = Dec2BCD(hour);
    RtcDataWrite(RTC_WR_HOUR_ADDR, (bcdData &= ~0x80));
    ets_delay_us(DelayIO);

}

// Set date in the RTC
void RtcSetDate(uint8_t day, uint8_t month, uint8_t weekday, uint8_t year) {

    uint8_t bcdData = Dec2BCD(day);
    RtcDataWrite(RTC_WR_DAY_ADDR, (bcdData &= ~0xc0));
    ets_delay_us(DelayIO);

    bcdData = Dec2BCD(month);
    RtcDataWrite(RTC_WR_MONTH_ADDR, (bcdData &= ~0xe0));
    ets_delay_us(DelayIO);

    bcdData = Dec2BCD(weekday);
    RtcDataWrite(RTC_WR_WEEKDAY_ADDR, (bcdData &= ~0xf8));
    ets_delay_us(DelayIO);

    bcdData = Dec2BCD(year);
    RtcDataWrite(RTC_WR_YEAR_ADDR, bcdData);
    ets_delay_us(DelayIO);
}


void RtcReadBuffer(void) // 800us
{
	uint8_t Result = 0; // 0: normal !0:Err
	uint8_t sec, min, hour, day, month, week, year;

	RTCStart();
	RTCWriteByte(RTC_RD_CLK_BURST_ADDR);
	sec = BCD2Dec(RTCReadByte());	// sec
	min = BCD2Dec(RTCReadByte());	// min
	hour = BCD2Dec(RTCReadByte());	// hour
	day = BCD2Dec(RTCReadByte());	// day
	month = BCD2Dec(RTCReadByte());	// month
	week = BCD2Dec(RTCReadByte());	// week
	year = BCD2Dec(RTCReadByte());	// year

	RTCStop();

/*

RtcSetTime(0, 0, 12); // Set time: 12:45:30
		RtcSetDate(1, 1, 1, 20); // Set date: 23rd Oct, 2024, Weekday = 4 (Wednesday)

*/



	time_library_time_t initial_time = {year+2000, month, day, week, hour, min, sec};//     year, month, day, hour, minute, second;
    time_library_set_time(&initial_time, 0);

	return Result;
}

void gpioMode(bool output , gpio_num_t pinNo){

 gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = output?GPIO_MODE_OUTPUT:GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << pinNo),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE 
    };
    gpio_config(&io_conf);
}

static void intTimeFormet(void){

// dspTimeFormet;
    nvs_handle_t nvs_handle;
    esp_err_t  ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        int32_t saved_format = 0;
        ret = nvs_get_i32(nvs_handle, TIME_FORMAT_KEY, &saved_format);
        if (ret == ESP_OK) {
            dspTimeFormet = saved_format;
            // ESP_LOGI(TAG, "Retrieved time format: %s-hour", dspTimeFormet ? "12" : "24");
        } else {
            // ESP_LOGI(TAG, "Time format not found, defaulting to 12-hour.");
            dspTimeFormet = true; // Default to 12-hour format if not set
        }
        nvs_close(nvs_handle);
    }
}

// RTC Initialization
void RtcInit(void) {

	gpioMode(1, DS1302_CE_PIN);   // CE (chip select) pin as output
    gpioMode(1, DS1302_IO_PIN);   // IO (data) pin as output (initially)
    gpioMode(1, DS1302_SCLK_PIN); // SCLK (clock) pin as output
    vTaskDelay(100); // Wait for 1 second

    // Disable write protect
    RTCStart();
    RTCWriteByte(RTC_WR_WP_ADDR);
    RTCWriteByte(0x00);
    RTCStop();

    // Read from RAM to check if RTC is configured
    RTCStart();
    RTCWriteByte(RTC_RD_RAM_BURST_ADDR);
    uint8_t ramData1 = RTCReadByte();
    uint8_t ramData2 = RTCReadByte();
    RTCStop();

    printf("ramData1 %x  ramData2 %x \n", ramData1,ramData2);

    if (ramData1 != 'O' || ramData2 != 'K') {

        printf("RTC not configured.\n");

		RTCStart();
		RTCWriteByte(RTC_WR_TCS_ADDR);
		RTCWriteByte(RTC_TCS_0_DNRN);//RTCWriteByte(RTC_TCS_0_DNRN); // hard ware 3.3V
		RTCStop();

		RTCStart();
		RTCWriteByte(RTC_WR_RAM_BURST_ADDR);
		RTCWriteByte('O');
		RTCWriteByte('K');
		RTCStop();

		RtcSetTime(0, 0, 12); // Set time: 12:45:30
		RtcSetDate(1, 1, 1, 20); // Set date: 23rd Oct, 2024, Weekday = 4 (Wednesday)

    } 
    intTimeFormet();
    RtcReadBuffer();

}
void save_time_format(bool is_12_hour) {
    nvs_handle_t nvs_handle;
    esp_err_t ret = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &nvs_handle);
    if (ret == ESP_OK) {
        ret = nvs_set_i32(nvs_handle, TIME_FORMAT_KEY, is_12_hour);
        if (ret == ESP_OK) {
            nvs_commit(nvs_handle);
            // ESP_LOGI(TAG, "Time format saved as %s-hour", is_12_hour ? "12" : "24");
        }
        nvs_close(nvs_handle);
    }
}
