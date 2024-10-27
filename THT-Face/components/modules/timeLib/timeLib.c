#include "timeLib.h"
#include "globalScope.h"

static const char *TAG = "TimeLib";

time_library_time_t reference_time;
static uint32_t reference_tick_count;
// time_library_time_t current_time;




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




void gpioMode(bool output, gpio_num_t pinNo) {
    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .mode = output ? GPIO_MODE_OUTPUT : GPIO_MODE_INPUT,
        .pin_bit_mask = (1ULL << pinNo),
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .pull_up_en = GPIO_PULLUP_DISABLE
    };
    gpio_config(&io_conf);
}

// GPIO Macros for RTC Control
#define RTC_DATA_WRITE_MODE gpioMode(1, DS1302_IO_PIN)
#define RTC_DATA_READ_MODE gpioMode(0, DS1302_IO_PIN)

#define RTC_CLK_HIGH gpio_set_level(DS1302_SCLK_PIN, 1)
#define RTC_CLK_LOW gpio_set_level(DS1302_SCLK_PIN, 0)
#define RTC_CS_HIGH gpio_set_level(DS1302_CE_PIN, 1)
#define RTC_CS_LOW gpio_set_level(DS1302_CE_PIN, 0)
#define RTC_DATA_HIGH gpio_set_level(DS1302_IO_PIN, 1)
#define RTC_DATA_LOW gpio_set_level(DS1302_IO_PIN, 0)
#define RTC_DATA_READ gpio_get_level(DS1302_IO_PIN)

// BCD to Decimal Conversion
uint8_t BCD2Dec(uint8_t bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// Decimal to BCD Conversion
uint8_t Dec2BCD(uint8_t dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// Initialize communication with the RTC
void RTCStart(void) {
    RTC_DATA_LOW;
    ets_delay_us(DelayRST);
    RTC_CLK_LOW;
    ets_delay_us(DelayRST);
    RTC_CS_HIGH;
    ets_delay_us(DelayRST);
}

// End communication with the RTC
void RTCStop(void) {
    RTC_CS_LOW;
    ets_delay_us(DelayRST);
    RTC_CLK_LOW;
    ets_delay_us(DelayRST);
    RTC_DATA_LOW;
    ets_delay_us(DelayRST);
}

// Write a command or data byte to the RTC
void WriteCmd(uint8_t command) {
    RTC_DATA_WRITE_MODE;
    ets_delay_us(MODE_CHANGE_DELAY);

    for (int i = 0; i < 8; i++) {
        if (command & 0x01) {
            RTC_DATA_HIGH;
        } else {
            RTC_DATA_LOW;
        }
        command >>= 1;

        RTC_CLK_HIGH;
        ets_delay_us(CLK_DELAY);
        RTC_CLK_LOW;
        ets_delay_us(CLK_DELAY);
    }
}

// Read a byte from the RTC
uint8_t readbyte() {
    uint8_t data = 0;
    RTC_DATA_READ_MODE;
    ets_delay_us(MODE_CHANGE_DELAY);

    for (int i = 0; i < 8; i++) {
        RTC_CLK_HIGH;
        ets_delay_us(CLK_DELAY);

        data >>= 1;
        if (RTC_DATA_READ) {
            data |= 0x80;
        }

        RTC_CLK_LOW;
        ets_delay_us(CLK_DELAY);
    }
    return data;
}

// RTC Initialization with RAM Check
void RtcInit(void) {
    // Set initial state of the GPIOs
    RTC_CLK_LOW;
    RTC_DATA_LOW;
    RTC_CS_LOW;

    // Set GPIO pins as output
    gpioMode(1, DS1302_CE_PIN);
    gpioMode(1, DS1302_IO_PIN);
    gpioMode(1, DS1302_SCLK_PIN);

    // Disable write protection
    RTCStart();
    WriteCmd(RTC_WR_WP_ADDR);
    WriteCmd(0x00);  // Disable write protection
    RTCStop();

    // Delay for stability
    vTaskDelay(10);

    // Check if RTC is already configured
    RTCStart();
    WriteCmd(RTC_RD_RAM_BURST_ADDR);  // Start reading from RAM in burst mode
    uint8_t c8c1 = readbyte();         // Read first byte from RAM
    uint8_t c8c2 = readbyte();         // Read second byte from RAM
    RTCStop();

    printf("RAM Data Read: 0x%02X, 0x%02X\n", c8c1, c8c2);

    // If RAM not configured, proceed with initialization
    if ((c8c1 != 'O') || (c8c2 != 'K')) {
        printf("RTC not configured. Initializing now...\n");

        RTCStart();
        WriteCmd(RTC_WR_TCS_ADDR);
        WriteCmd(RTC_TCS_0_DNRN);  // Enable trickle charge
        RTCStop();

        // Write 'O' and 'K' to RAM as initialization check
        RTCStart();
        WriteCmd(RTC_WR_RAM_BURST_ADDR);
        WriteCmd('O');
        WriteCmd('K');
        RTCStop();

        printf("RTC configured successfully.\n");
    } else {
        printf("RTC is already configured.\n");
    }
}

// Get the current time and date in burst mode
RTC_TimeDate GetTimeAndDate(void) {
    RTC_TimeDate timeDate;

    RTCStart();
    WriteCmd(RTC_RD_CLK_BURST_ADDR);  // Start burst read command

    timeDate.seconds = BCD2Dec(readbyte());  // Read seconds
    timeDate.minutes = BCD2Dec(readbyte());  // Read minutes
    timeDate.hours   = BCD2Dec(readbyte());  // Read hours
    timeDate.day     = BCD2Dec(readbyte());  // Read day
    timeDate.month   = BCD2Dec(readbyte());  // Read month
    readbyte();                             // Read weekday (skip if unused)
    timeDate.year    = BCD2Dec(readbyte());  // Read year

    RTCStop();
    return timeDate;
}

// Print the current date and time
void PrintTimeAndDate() {
    RTC_TimeDate currentTime = GetTimeAndDate();

    printf("Current Date and Time: %02d-%02d-20%02d %02d:%02d:%02d\n",
           currentTime.day,
           currentTime.month,
           currentTime.year,
           currentTime.hours,
           currentTime.minutes,
           currentTime.seconds);
}













// uint8_t RTCReadByte(void)
// {
// 	uint8_t c8c1, c8c2, ByteData = 0;
// 	printf("\nread mode ");

	

//     RTC_DATA_READ_MODE for(c8c1=DelayIO; c8c1; c8c1--){}

// 	// for(c8c2 = 0x01; c8c2; c8c2<<=1)
// 	// {

//     //     vTaskDelay(10);
// 	// 	if(RTC_DATA_READ){
//     //         ByteData |= c8c2;
//     //         printf("1");
//     //     }else printf("0");
		
// 	// 	for(c8c1=DelayClk; c8c1; c8c1--) RTC_CLK_HIGH
// 	// 	for(c8c1=DelayClk; c8c1; c8c1--) RTC_CLK_LOW

// 	// }
// 	// return ByteData;

//     for (int i = 0; i < 8; i++) {
//         ByteData >>= 1;
//         vTaskDelay(1);
//         if (gpio_get_level(DS1302_IO_PIN)) {
//             ByteData |= 0x80;  // Read bit from IO pin and shift it to the correct position
//         }
//         gpio_set_level(DS1302_SCLK_PIN, 1); ets_delay_us(300);                 // Wait for clock high period (recommended 200-300us)  // Set clock high
//         gpio_set_level(DS1302_SCLK_PIN, 0); vTaskDelay(1); // Set clock low
//     }

// 	return ByteData;



// }

// void RTCWriteByte(uint8_t ByteData)
// {
// 	uint8_t c8c1, c8c2;
// 	printf("\nwrite mode cmd ");
    
//     RTC_DATA_WRITE_MODE for(c8c1=DelayIO; c8c1; c8c1--){}

// 	for(c8c2 = 0x01; c8c2; c8c2<<=1)
// 	{

//         printf("%d", (ByteData&c8c2)?1:0);

//         if(ByteData&c8c2){
//             for(c8c1=DelayIO; c8c1; c8c1--) RTC_DATA_HIGH
//         }	
//         else{
//             for(c8c1=DelayIO; c8c1; c8c1--) RTC_DATA_LOW
//         }				
	
// 		for(c8c1=DelayClk; c8c1; c8c1--) RTC_CLK_HIGH
// 		for(c8c1=DelayClk; c8c1; c8c1--) RTC_CLK_LOW	
// 	}




// //  for (int i = 0; i < 8; i++) {
// //         gpio_set_level(DS1302_IO_PIN, ByteData & 0x01);  // Set IO pin based on the LSB of data
// //         printf("%d",ByteData & 0x01);

// //         ByteData >>= 1;  // Shift data to right to send the next bit
// //         for(c8c1=DelayClk; c8c1; c8c1--) RTC_CLK_HIGH
// // 		for(c8c1=DelayClk; c8c1; c8c1--) RTC_CLK_LOW	
// //     }



// }


// void RtcDataRead(uint8_t eRtcDataType)
// {
// 	uint8_t c8c1;

// 	switch(eRtcDataType)
// 	{
// 	default:
// 	case eRtcDataSec:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_SEC_ADDR);

// 		c8c1 =BCD2Dec(RTCReadByte()); // sec

// 		RTCStop();

//         printf(" sec %d\n",c8c1);
// 		break;
		
// 	case eRtcDataMin:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_MIN_ADDR);
// 		c8c1 = BCD2Dec(RTCReadByte());	// min
// 		RTCStop();
//         printf(" min %d",c8c1);

// 		break;
		
// 	case eRtcDataHour:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_HOUR_ADDR);
// 		c8c1 = BCD2Dec(RTCReadByte());	// hour
// 		RTCStop();
//         printf(" hourse %d",c8c1);

// 		break;
		
// 	case eRtcDataDay:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_DAY_ADDR);
// 		c8c1 = BCD2Dec(RTCReadByte());	// day
// 		RTCStop();
//         printf(" day %d ",c8c1);

// 		break;
		
// 	case eRtcDataWeek:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_WEEKDAY_ADDR);
// 		RTCReadWeekDay = BCD2Dec(RTCReadByte());	// weekday
// 		RTCStop();

//         printf(" day %d",RTCReadWeekDay );


// 		break;
		
// 	case eRtcDataMonth:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_MONTH_ADDR);
// 		RTCReadMonth = BCD2Dec(RTCReadByte());		// month
// 		RTCStop();+
//         printf(" month %d",RTCReadMonth );

// 		break;
		
// 	case eRtcDataYear:
// 		RTCStart();
// 		RTCWriteByte(RTC_RD_YEAR_ADDR);
// 		RTCReadYear = BCD2Dec(RTCReadByte());		// year
// 		RTCStop();
//         printf("\nyear %d",RTCReadYear );
// 		break;
// 	}

// }
// void RtcDataWrite(uint8_t eRtcDataType)
// {
// 	uint8_t c8c1;

// 	switch(eRtcDataType)
// 	{
// 	default:
// 	case eRtcDataSec:
// 		c8c1 = Dec2BCD(RtcTimeSec);
// 		c8c1 &= ~0x80;
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_SEC_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
		
// 	case eRtcDataMin:
// 		c8c1 = Dec2BCD(RtcTimeMin);
// 		c8c1 &= ~0x80;
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_MIN_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
		
// 	case eRtcDataHour:
// 		c8c1 = Dec2BCD(RtcTimeHour);
// 		c8c1 &= ~0x80;
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_HOUR_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
		
// 	case eRtcDataDay:
// 		c8c1 = Dec2BCD(RtcTimeDay);
// 		c8c1 &= ~0xc0;
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_DAY_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
		
// 	case eRtcDataWeek:
// 		c8c1 = Dec2BCD(RTCReadWeekDay);
// 		c8c1 &= ~0xf8;
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_WEEKDAY_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
		
// 	case eRtcDataMonth:
// 		c8c1 = Dec2BCD(RTCReadMonth);
// 		c8c1 &= ~0xe0;
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_MONTH_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
		
// 	case eRtcDataYear:
// 		c8c1 = Dec2BCD(RTCReadYear);
// 		RTCStart();
// 		RTCWriteByte(RTC_WR_YEAR_ADDR);
// 		RTCWriteByte(c8c1);
// 		RTCStop();
// 		break;
// 	}
// }


// uint8_t RtcReadBuffer(void) // 800us
// {
// 	uint8_t Result = 0; // 0: normal !0:Err
// //	static uint8_t sc8c1;
// 	uint8_t c8c1, c8c2, c8c3, c8c4, c8c5, c8c6, c8c7;

// //	if(sc8c1<50)	{sc8c1++;return Result;}
// //	else sc8c1 = 0;
	
// 	// RTCStart();
// 	// RTCWriteByte(RTC_RD_CLK_BURST_ADDR);
// 	// c8c1 = BCD2Dec(RTCReadByte());	// sec
// 	// c8c2 = BCD2Dec(RTCReadByte());	// min
// 	// c8c3 = BCD2Dec(RTCReadByte());	// hour
// 	// c8c4 = BCD2Dec(RTCReadByte());	// day
// 	// c8c5 = BCD2Dec(RTCReadByte());	// month
// 	// c8c6 = BCD2Dec(RTCReadByte());	// week
// 	// c8c7 = BCD2Dec(RTCReadByte());	// year
// 	// RTCStop();

//     // printf("year %d Month %d week %d day %d hour %d min %d sec %d\n", c8c7,c8c5, c8c6 ,c8c4 ,c8c3 ,c8c2 ,c8c1);



// 	// RtcDataRead(eRtcDataYear);
// 	// RtcDataRead(eRtcDataMonth);
// 	// RtcDataRead(eRtcDataWeek);
// 	// RtcDataRead(eRtcDataDay);
// 	// RtcDataRead(eRtcDataHour);
// 	// RtcDataRead(eRtcDataMin);
// 	RtcDataRead(eRtcDataSec);


// 	return Result;
// }

// void RtcSetTime(uint8_t iSec, uint8_t iMin, uint8_t iHour) // iHour 24H
// {
// 	iSec	=	Dec2BCD(iSec);
// 	iMin	=	Dec2BCD(iMin);
// 	iHour	=	Dec2BCD(iHour);

// 	iSec &= 0x7f;    // Sec
// 	iMin &= 0x7f;    // Min
// 	iHour &= 0x7f;    // Hour // 24H

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_SEC_ADDR);
// 	RTCWriteByte(iSec);
// 	RTCStop();

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_MIN_ADDR);
// 	RTCWriteByte(iMin);
// 	RTCStop();

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_HOUR_ADDR);
// 	RTCWriteByte(iHour);
// 	RTCStop();
// }

// void RtcSetDate(uint8_t day, uint8_t month, uint8_t weekday, uint8_t year)
// {
// 	day		=	Dec2BCD(day);
// 	month	=	Dec2BCD(month);
// 	weekday	=	Dec2BCD(weekday);
// 	year	=	Dec2BCD(year);

// 	day		&=	0x3f;    // Day
// 	month	&=	0x1f;    // Month
// 	weekday	&=	0x07;    // Week Day
// 	year	&=	0xff;    // Year

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_DAY_ADDR);
// 	RTCWriteByte(day);
// 	RTCStop();

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_MONTH_ADDR);
// 	RTCWriteByte(month);
// 	RTCStop();

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_WEEKDAY_ADDR);
// 	RTCWriteByte(weekday);
// 	RTCStop();

// 	RTCStart();
// 	RTCWriteByte(RTC_WR_YEAR_ADDR);
// 	RTCWriteByte(year);
// 	RTCStop();
// }


// bool RTCAdjustTimeFormat(unsigned short *hour) {

//     bool isPM = false; // false for AM, true for PM

//     // If hour is greater than or equal to 24, reset to 0
//     if (*hour >= 24) {
//         *hour = *hour % 12;
//     }

//     // If hour is 0, it's 12 AM
//     if (*hour == 0) {
//         *hour = 12;
//     }
//     // If hour is 12, it's 12 PM
//     else if (*hour == 12) {
//         isPM = true;
//     }
//     // If hour is greater than 12, subtract 12 to convert to 12-hour format
//     else if (*hour > 12) {
//         *hour -= 12;
//         isPM = true;
//     }

//     return isPM;
// }



// void RtcInit(void)
// {



//     // gpio_set_direction(DS1302_CE_PIN, GPIO_MODE_OUTPUT);
//     // gpio_pad_select_gpio(DS1302_CE_PIN);

//     gpioMode(1,DS1302_CE_PIN);
//     gpioMode(1,DS1302_IO_PIN);
//     gpioMode(1,DS1302_SCLK_PIN);

//     // gpio_set_direction(DS1302_IO_PIN, GPIO_MODE_OUTPUT);
//     // gpio_pad_select_gpio(DS1302_IO_PIN);


//     // gpio_set_direction(DS1302_SCLK_PIN, GPIO_MODE_OUTPUT);
//     // gpio_pad_select_gpio(DS1302_SCLK_PIN);


// 	uint8_t c8c1, c8c2, c8c3, c8c4, c8c5, c8c6;

// 	// Rtc write enable
// 	RTCStart();
// 	RTCWriteByte(RTC_WR_WP_ADDR);
// 	RTCWriteByte(0x00);
// 	RTCStop();

// 	// read ram from rtc to check rtc config or not
// 	RTCStart();
// 	RTCWriteByte(RTC_RD_RAM_BURST_ADDR);
// 	c8c1 = (RTCReadByte());
// 	c8c2 = (RTCReadByte());
// 	RTCStop();




// 	if((c8c1 != 'O')||(c8c2 != 'K'))
// 	{

//         printf("in ok %x %x \n",c8c1,c8c1 );


// 	// 	// batter charge anable
// 	// 	RTCStart();
// 	// 	RTCWriteByte(RTC_WR_TCS_ADDR);
// 	// 	RTCWriteByte(RTC_TCS_0_DNRN);//RTCWriteByte(RTC_TCS_0_DNRN); // hard ware 3.3V
// 	// 	RTCStop();

// 	// 	RTCStart();
// 	// 	RTCWriteByte(RTC_WR_RAM_BURST_ADDR);
// 	// 	RTCWriteByte('O');
// 	// 	RTCWriteByte('K');
// 	// 	RTCStop();

// 	// 	c8c1 = (TABLESoftWare[27]-'0')*10+(TABLESoftWare[28]-'0');
// 	// 	c8c2 = (TABLESoftWare[25]-'0')*10+(TABLESoftWare[26]-'0');
// 	// 	c8c3 = (TABLESoftWare[23]-'0')*10+(TABLESoftWare[24]-'0');
// 	// 	c8c4 = (TABLESoftWare[21]-'0')*10+(TABLESoftWare[22]-'0');
// 	// 	c8c5 = (TABLESoftWare[19]-'0')*10+(TABLESoftWare[20]-'0');
// 	// 	c8c6 = (TABLESoftWare[17]-'0')*10+(TABLESoftWare[18]-'0');
// 	// 	RTCReadYear = c8c6;
// 	// 	RTCReadMonth = c8c5;
// 	// 	RtcTimeCount10mS = c8c4*24*60*60*100;
// 	// 	RtcTimeCount10mS += c8c3*60*60*100;
// 	// 	RtcTimeCount10mS += c8c2*60*100;
// 	// 	RtcTimeCount10mS += c8c1*100;
// 	// 	RtcDataWrite(eRtcDataYear);
// 	// 	RtcDataWrite(eRtcDataMonth);
// 	// 	RtcDataWrite(eRtcDataDay);
// 	// 	RtcDataWrite(eRtcDataHour);
// 	// 	RtcDataWrite(eRtcDataMin);
// 	// 	RtcDataWrite(eRtcDataSec);

// 	// 	// �Լ��������ڣ�������IC�ṩ��������Ϣ
// 	// 	RTCReadWeekDay = WeekDayCaculate(RTCReadYear, RTCReadMonth, RtcTimeDay); // ����������ģʽ//RtcDataRead(eRtcDataWeek); // read the week day

// 	// 	RtcState = 0xaa;
// 	}else{
//         printf("not ok");
//     }



// 	// RtcDataRead(eRtcDataYear);
// 	// RtcDataRead(eRtcDataMonth);
// 	// RtcDataRead(eRtcDataWeek);
// 	// RtcDataRead(eRtcDataDay);
// 	// RtcDataRead(eRtcDataHour);
// 	// RtcDataRead(eRtcDataMin);
// 	// RtcDataRead(eRtcDataSec);
// }
