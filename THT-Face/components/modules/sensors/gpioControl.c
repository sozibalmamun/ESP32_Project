#include "gpioControl.h"
#include <string.h>

#define MUSIC_PLAY_TIME TIMEOUT_500_MS

uint16_t batVoltage;
uint8_t chargeState=0;
uint8_t music=0;
// uint32_t musicPlayDuration = 0;
TaskHandle_t sensorsHandeler = NULL;

esp_adc_cal_characteristics_t *adc_chars_battery = NULL;
esp_adc_cal_characteristics_t *adc_chars_pir = NULL;

union shiftResistorBitfild shiftOutData;


void gpioInt(void){

    gpio_set_level((gpio_num_t)LCE_BL, 1);
    gpio_pad_select_gpio(LCE_BL);
    gpio_set_direction((gpio_num_t)LCE_BL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)LCE_BL, 1);


    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << MUSICPIN), // Pin mask
        .mode = GPIO_MODE_OUTPUT,                  // Set as input
        .pull_up_en = GPIO_PULLUP_DISABLE ,         // Enable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE     // Disable pull-down resistor
    };
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << BATTERY_CHARGE_STATE); // Pin mask
    io_conf.mode = GPIO_MODE_INPUT;                  // Set as input
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;         // Enable pull-up resistor
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;   // Disable pull-down resistor
    gpio_config(&io_conf);



    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << MUSIC_BUSY); // Pin mask
    io_conf.mode = GPIO_MODE_INPUT;                  // Set as input
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;         // Enable pull-up resistor
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;   // Disable pull-down resistor
    gpio_config(&io_conf);


    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << PIR); // Pin mask
    io_conf.mode = GPIO_MODE_INPUT;                  // Set as input
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;         // Enable pull-up resistor
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;   // Disable pull-down resistor
    gpio_config(&io_conf);


}

void PwmInt( gpio_num_t pinNo ) {

    ledc_timer_config_t ledc_timer = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,   // Low-speed mode
        .duty_resolution = LEDC_TIMER_13_BIT,      // 13-bit duty resolution
        .timer_num       = LEDC_TIMER_0,           // Timer 0
        .freq_hz         = 5000,                   // 5kHz PWM frequency
        .clk_cfg         = LEDC_AUTO_CLK           // Auto select clock
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel={

    // Configure LEDC channel for GPIO 14
        .gpio_num   = pinNo,                // GPIO pin for PWM output
        .speed_mode = LEDC_LOW_SPEED_MODE,        // Low-speed mode
        .channel    = LEDC_CHANNEL_0,             // Channel: 0
        .intr_type  = LEDC_INTR_DISABLE,          // Disable fade interrupt
        .timer_sel  = LEDC_TIMER_0,               // Select Timer 0
        .duty       = 0,                          // Initial duty cycle: 0
        .hpoint     = 0,                          // Hpoint: 0
        .flags.output_invert = 0                 // Disable output inversion
    };
    // Initialize the channel
    ledc_channel_config(&ledc_channel);

    // Set initial duty cycle to 50% (for 13-bit resolution, this is 4096 out of 8192)
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 8192);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Optional: Use fading
    ledc_fade_func_install(0);  // Install the fade function
    ledc_set_fade_time_and_start(ledc_channel.speed_mode, ledc_channel.channel, 8192, 1000, LEDC_FADE_NO_WAIT);
    for (int duty = 8192; duty >= 0; duty -= 1024) {
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 0);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

}


void brightness(bool sleep){
    
    ledc_channel_config_t ledc_channel={

        .speed_mode = LEDC_LOW_SPEED_MODE,        // Low-speed mode
        .channel    = LEDC_CHANNEL_0,             // Channel: 0

    };

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, sleep?BRIGHTNESS(SLEEP_LCD): BRIGHTNESS(WAKE_LCD));//8192
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

}

void dispON(bool dspOn){
    
    ledc_channel_config_t ledc_channel={

        .speed_mode = LEDC_LOW_SPEED_MODE,        // Low-speed mode
        .channel    = LEDC_CHANNEL_0,             // Channel: 0

    };
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, dspOn?0:8192 );//8192
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

}






void configure_dynamic_frequency() {
    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = MAX_FREQ,      // Maximum frequency
        .min_freq_mhz = MIN_FREQ,       // Minimum frequency
        .light_sleep_enable = false  // Keep in active mode
    };
    esp_err_t ret = esp_pm_configure(&pm_config);
    if (ret == ESP_OK) {
        // ESP_LOGI("Frequency", "Dynamic CPU frequency scaling configured.");
    } else {
        // ESP_LOGE("Frequency", "Failed to configure CPU frequency: %s", esp_err_to_name(ret));
    }
}
void reduce_cpu_frequency() {


    if (cameraTaskHandler) vTaskSuspend(cameraTaskHandler);
    if (eventTaskHandler) vTaskSuspend(eventTaskHandler);
    if (recognitionTaskHandler) vTaskSuspend(recognitionTaskHandler);
    if (recognitioneventTaskHandler) vTaskSuspend(recognitioneventTaskHandler);
    if (lcdTaskHandler) vTaskSuspend(lcdTaskHandler);
    if (cloudeTaskHandler) vTaskSuspend(cloudeTaskHandler);
    if (sensorsHandeler) vTaskSuspend(sensorsHandeler);
  


    // ESP_LOGE("Frequency", "delete all task");
    esp_pm_dump_locks(stdout);

    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = MIN_FREQ,   // Set both min and max to 80 MHz to reduce power
        .min_freq_mhz = MIN_FREQ,
        .light_sleep_enable = false
    };
    esp_err_t ret = esp_pm_configure(&pm_config);
    vTaskDelay(pdMS_TO_TICKS(10));  // Allow time for frequency update

    if (ret == ESP_OK) {
        // ESP_LOGI("Frequency", "Dynamic CPU frequency scaling configured.");
    } else {
        // ESP_LOGE("Frequency", "Failed to configure CPU frequency: %s", esp_err_to_name(ret));
    }    

    // if (lcdTaskHandler) vTaskResume(lcdTaskHandler); // uncomment if turn on display task

    // esp_pm_dump_locks(stdout);

    int cpu_freq_mhz = esp_clk_cpu_freq() / 1000000;
    ESP_LOGI("CPU Monitor", "Current CPU frequency: %d MHz", cpu_freq_mhz);

    // HALT

}


void restore_cpu_frequency() {


    if (lcdTaskHandler) vTaskSuspend(lcdTaskHandler);


    esp_pm_config_esp32s3_t pm_config = {
        .max_freq_mhz = MAX_FREQ,  // Restore max frequency
        .min_freq_mhz = MAX_FREQ,   // Set min to 80 for dynamic scaling
        .light_sleep_enable = false
    };

    esp_err_t ret = esp_pm_configure(&pm_config);
    vTaskDelay(pdMS_TO_TICKS(50));  // Allow time for frequency update

    if (ret == ESP_OK) {
        // ESP_LOGI("Frequency", "Dynamic CPU frequency scaling configured.");
    } else {
        // ESP_LOGE("Frequency", "Failed to configure CPU frequency: %s", esp_err_to_name(ret));
    } 

    if (cameraTaskHandler) vTaskResume(cameraTaskHandler);
    if (eventTaskHandler) vTaskResume(eventTaskHandler);
    if (recognitionTaskHandler) vTaskResume(recognitionTaskHandler);
    if (recognitioneventTaskHandler) vTaskResume(recognitioneventTaskHandler);
    if (lcdTaskHandler) vTaskResume(lcdTaskHandler);
    if (cloudeTaskHandler) vTaskResume(cloudeTaskHandler);
    if (sensorsHandeler) vTaskResume(sensorsHandeler);


}


// Function to enter light sleep mode--------------------------------------

void configure_wakeup() {
    // Configure the button GPIO for input with pull-up, active-low
    // gpio_pad_select_gpio(PIR);
    // gpio_set_direction(PIR, GPIO_MODE_INPUT);
    // gpio_pullup_en(PIR);  // Enable pull-up for stable input

    // Enable external wakeup on GPIO_WAKEUP_BUTTON, active low
    esp_sleep_enable_ext0_wakeup(PIR, 0);  // Wake on falling edge (button press)
}
void enter_light_sleep(void) {

    configure_wakeup();
    vTaskDelay(pdMS_TO_TICKS(10));
    ESP_LOGI("Sleep", "Entering light sleep...");
    // gpio_set_level((gpio_num_t)CAMP_DWN, 1);  // Ensure peripherals are powered off or set to sleep state

    // Set the wake-up source to external (GPIO_BOOT, active low)
    // esp_sleep_enable_timer_wakeup(1000000 * 120); // Wake up every 120 seconds (in microseconds)
    // Enter light sleep
    esp_light_sleep_start();
    ESP_LOGI("Sleep", "Woke up from light sleep!");
    sleepEnable = WAKEUP;  // Disable sleep mode after waking up
}


void enter_deep_sleep(void){

    ESP_LOGI("DEEP_SLEEP", "Going to deep sleep...");
    // Enable wake-up on button press (when GPIO is LOW)
    gpio_pullup_en(PIR);  // Enable pull-up resistor
    esp_sleep_enable_ext0_wakeup(PIR, 0); // Wake-up when button is pressed (LOW)
    // Enter deep sleep
    esp_deep_sleep_start();
    // This line will **never execute** because ESP restarts after wake-up
    ESP_LOGI("DEEP_SLEEP", "Woke up!");
}
//------------------------------------------------------------------------------


void init_adc() {
    // Configure ADC width and attenuation for ADC2
    adc2_config_channel_atten(BATTERY_ADC_CHANNEL, ADC_ATTEN_DB_11);  // 0-3.6V range
    adc_chars_battery = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars_battery);

    // Configure ADC for PIR sensor (ADC1)
    // adc1_config_width(ADC_WIDTH_BIT_12);                             // 12-bit resolution
    // adc1_config_channel_atten(PIR_ADC_CHANNEL, ADC_ATTEN_DB_11);     // 0-3.6V range
    // adc_chars_pir = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    // esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars_pir);


}


// Function to read ADC and calculate voltage
void readBatteryVoltage() {


    if (adc_chars_battery != NULL) {

        uint32_t adc_reading = 0;
        int raw;
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            // Read ADC2 channel (this call returns the raw value directly)
            if (adc2_get_raw(BATTERY_ADC_CHANNEL, ADC_WIDTH_BIT_12, &raw) == ESP_OK) {
                adc_reading += raw;
            }
        }
        adc_reading /= NO_OF_SAMPLES;
        batVoltage=esp_adc_cal_raw_to_voltage(adc_reading, adc_chars_battery);
        // printf("Battery Voltage: %d mV\n", batVoltage);
    }

}
//----------------------------------------pir adc

// Function to read ADC and calculate voltage
void pirRead() {

    if (adc_chars_pir != NULL) {

        uint32_t adc_reading = 0;
        for (int i = 0; i < NO_OF_SAMPLES; i++) {
            adc_reading += adc1_get_raw(PIR_ADC_CHANNEL);
        }
        adc_reading /= NO_OF_SAMPLES;

        uint16_t pirVal = esp_adc_cal_raw_to_voltage(adc_reading, adc_chars_pir); // Return voltage in mV
        printf("pirVal : %d mV\n", pirVal);
    }

}
//----------------------------------------



void plugIn(bool plugin){

    if(plugin){
        if(chargeState==CHARGE_IDLE || chargeState == UN_PLUGING){
            sleepTimeOut = xTaskGetTickCount();
            brightness(false);
            // printf("charging high lcd\n");
            chargeState=CHARGER_PLUGED;
        }

        if(sleepTimeOut+500 < xTaskGetTickCount() && chargeState == CHARGER_PLUGED ){
            brightness(true);
            // printf("charging low lcd\n");
            chargeState=CHARGEING;
        }
    }else{

        if(chargeState==CHARGEING || chargeState==CHARGER_PLUGED){
            sleepTimeOut = xTaskGetTickCount();
            // printf("CHARGER_UNPLUGING\n");
            brightness(false);
            chargeState=UN_PLUGING;
        }

        if(sleepTimeOut+500 < xTaskGetTickCount() && chargeState == UN_PLUGING ){
            brightness(true);
            // printf("CHARGER_UNPLUGING low lcd\n");
            chargeState=CHARGE_IDLE;
        }

    }

}







void shiftOut( uint8_t val){
    uint8_t c8;

    gpio_set_level((gpio_num_t)SER_LAT, 0);

    for(c8 = 0x80; c8; c8>>=1)
    {
        if(val&c8) gpio_set_level((gpio_num_t)SER_SDI, 1);
        else gpio_set_level((gpio_num_t)SER_SDI, 0);

        gpio_set_level((gpio_num_t)SER_CLK, 1);
        gpio_set_level((gpio_num_t)SER_CLK, 0);
    }
    gpio_set_level((gpio_num_t)SER_LAT, 1);

}





// static void sensor(void *arg)
// {
//     gpio_set_level((gpio_num_t)SER_SDI, 0);
//     gpio_pad_select_gpio(SER_SDI);
//     gpio_set_direction((gpio_num_t)SER_SDI, GPIO_MODE_OUTPUT);

//     gpio_set_level((gpio_num_t)SER_CLK, 0);
//     gpio_pad_select_gpio(SER_CLK);
//     gpio_set_direction((gpio_num_t)SER_CLK, GPIO_MODE_OUTPUT);

//     gpio_set_level((gpio_num_t)SER_LAT, 0);
//     gpio_pad_select_gpio(SER_LAT);
//     gpio_set_direction((gpio_num_t)SER_LAT, GPIO_MODE_OUTPUT);

//     // printf("in sensor\n");


//     uint8_t tempOld=0;
//     while (1)
//     {

//         // printf("Music: %d Shift %d\n",music, shiftOutData.bitset.MSDA);
//         if(music!=MUSIC_IDLE ){
//             if(music!=MUSIC_STOPING)shiftOutData.bitset.MSDA=1;
//                 //------------------------------------
//                 gpio_set_level((gpio_num_t)46, 1);

//         }

//         if(shiftOutData.read != tempOld){// true if any bit change
//             tempOld=shiftOutData.read;
//             shiftOut(shiftOutData.read);

//             printf("CAMEN: %d, MSDA: %d, MSCL: %d, PEREN: %d, LED: %d, LCDEN: %d, CAMPDWN: %d, IRLED: %d\n",
//             shiftOutData.bitset.CAMEN, shiftOutData.bitset.MSDA, shiftOutData.bitset.MSCL, shiftOutData.bitset.PEREN,shiftOutData.bitset.LED, 
//             shiftOutData.bitset.LCDEN, shiftOutData.bitset.CAMPDWN, shiftOutData.bitset.IRLED);
//             musicPlayDuration = xTaskGetTickCount();
   
//         }

//         switch (music)
//         {
//         case WELCOME:

//             printf("WELCOME \n");

//             vTaskDelay(pdMS_TO_TICKS(2));
//             shiftOutData.bitset.MSDA=0; 
//             music=MUSIC_STOPING;

//             break;
        
//         case UNREGISTERD:
//             printf("UNREGISTERD \n");
//             // vTaskDelay(pdMS_TO_TICKS(3));
//             ets_delay_us(300);
//             music=MUSIC_STOPING;
//             shiftOutData.bitset.MSDA=0;
//             //------------------------------------
//             gpio_set_level((gpio_num_t)46, 0);

//             break;
        
//         case MUSIC_STOPING:

//             // printf("MUSIC_STOPING\n");
//             if( xTaskGetTickCount()-musicPlayDuration>MUSIC_PLAY_TIME){
//                 music=MUSIC_STOP;
//                 shiftOutData.bitset.MSDA=1;
//             }

//             break;

//         case MUSIC_STOP:

//             vTaskDelay(pdMS_TO_TICKS(2));
//             music=MUSIC_IDLE;
//             shiftOutData.bitset.MSDA=0;

//             printf("MUSIC_STOP %d \n",shiftOutData.bitset.MSDA);
//             //------------------------------------
//             gpio_set_level((gpio_num_t)46, 1);

//             break;

//         case MUSIC_IMMEDIATE_STOP:

//             printf(" %d \n",shiftOutData.bitset.MSDA);
//             vTaskDelay(pdMS_TO_TICKS(1));
//             music=MUSIC_IDLE;
//             shiftOutData.bitset.MSDA=0;
//             //------------------------------------
//             gpio_set_level((gpio_num_t)46, 0);
//             break;

//         default:
//             break;
//         }

//         ets_delay_us(50);

//     }
// }




static void sensor(void *arg)
{
    gpio_set_level((gpio_num_t)SER_SDI, 0);
    gpio_pad_select_gpio(SER_SDI);
    gpio_set_direction((gpio_num_t)SER_SDI, GPIO_MODE_OUTPUT);

    gpio_set_level((gpio_num_t)SER_CLK, 0);
    gpio_pad_select_gpio(SER_CLK);
    gpio_set_direction((gpio_num_t)SER_CLK, GPIO_MODE_OUTPUT);

    gpio_set_level((gpio_num_t)SER_LAT, 0);
    gpio_pad_select_gpio(SER_LAT);
    gpio_set_direction((gpio_num_t)SER_LAT, GPIO_MODE_OUTPUT);

    // printf("in sensor\n");

    uint8_t welcomeMusic[12] = {1,2,1,2,0,2,1,2,0,2,1,2};
    uint8_t unregisterd[2] = {1,4};

    // init_pir();

    uint8_t tempOld=0;
    uint16_t musicTime=0;
    while (1)
    {


        // if(PIR_STATE==1){

        //     printf("PIR_STATE 1\n");

        // }else printf("PIR_STATE 0\n");

        while(music==MUSIC_IDLE && shiftOutData.read == tempOld ){
          

            if(PIR_STATE==1){
                 printf("PIR_STATE 1\n");
            }else printf("PIR_STATE 0\n");                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              
            // pirRead();
            // ets_delay_us(1000);//10
            vTaskDelay(pdMS_TO_TICKS(100));

        }

        if(shiftOutData.read != tempOld ){// true if any bit change
            tempOld = shiftOutData.read;
            shiftOut(shiftOutData.write);
        }

        switch (music)
        {
        case WELCOME:

            printf("WELCOME \n");
            // vTaskDelay(pdMS_TO_TICKS(2));
            // music=MUSIC_STOPING;

            break;
        
        case UNREGISTERD:
            printf("UNREGISTERD \n");
            // music=MUSIC_STOPING;
            // musicPlay(1);
            // musicTime=MUSIC_PLAY_TIME;
            //------------------------------------
            musicArrayPlay(&unregisterd,2);
            music=MUSIC_IDLE;
            
            break;
        
        // case MUSIC_STOPING:

        //     // printf("MUSIC_STOPING\n");
        //     if( xTaskGetTickCount()-musicPlayDuration>musicTime){
        //         music=MUSIC_STOP;
        //     }
        //     break;

        // case MUSIC_STOP:

        //     music=MUSIC_IDLE;
        //     printf("MUSIC_STOP \n");
        //     musicPlay(0);

        //     break;

        // case MUSIC_IMMEDIATE_STOP:

        //     printf("MUSIC_IMMEDIATE_STOP\n");
        //     music=MUSIC_IDLE;
        //     musicPlay(0);
        //     break;
        
        case TURN_ON_MUSIC:
            printf("TURN_ON_MUSIC \n");
            musicArrayPlay(welcomeMusic,12);
            break;
        default:
            break;
        }
        // ets_delay_us(1);

    }
}

void musicPlay(uint8_t musicNo ){

    // printf("music%s function \n",musicNo>0? "Play":"Stop");
    printf("music No %d \n",musicNo);

    #define  M_DELAY_uS 300

    for (size_t i = 0; i <=musicNo; i++)
    {
        gpio_set_level((gpio_num_t)MUSICPIN, 1);
        ets_delay_us(M_DELAY_uS);
        gpio_set_level((gpio_num_t)MUSICPIN, 0);
        ets_delay_us(M_DELAY_uS);
    }

    // musicPlayDuration = xTaskGetTickCount();

}



void musicArrayPlay(uint8_t *musicP ,uint8_t len){

    // uint8_t len = strlen(music);

    // printf("music sizeof %d \n" , len);
    uint8_t tempMusic =music;
    uint32_t musicPlayDuration = 0;

    for (size_t j = 0; j < len;j+=2)
    {

        // printf("loop no %d " ,j);
        // musicPlay(musicP[j]);


        for (size_t i = 0; i <=musicP[j]; i++)
        {
            gpio_set_level((gpio_num_t)MUSICPIN, 1);
            ets_delay_us(300);
            gpio_set_level((gpio_num_t)MUSICPIN, 0);
            ets_delay_us(300);
        }
        musicPlayDuration = xTaskGetTickCount();
        // printf("music delay %d ms\n" , music[j+1] * 100);
        while( xTaskGetTickCount()<musicPlayDuration+( musicP[j+1]*10)){
            
            if(music!=tempMusic){
                // printf("next music\n");
                musicPlay(0);
                return;
            }  
        }
    }
    musicPlay(0);
    music=MUSIC_IDLE;
}





void sensorHandel()
{
    xTaskCreatePinnedToCore(sensor, "sensor", 2 * 1024, NULL, 5, &sensorsHandeler, 1);
}





// void list_all_tasks(void) {
//     // Get the number of tasks currently running
//     UBaseType_t numTasks = uxTaskGetNumberOfTasks();
//     // Allocate memory to hold task information
//     TaskStatus_t *taskStatusArray = (TaskStatus_t *) pvPortMalloc(numTasks * sizeof(TaskStatus_t));
    
//     if (taskStatusArray != NULL) {
//         // Get task details
//         UBaseType_t taskCount = uxTaskGetSystemState(taskStatusArray, numTasks, NULL);

//         ESP_LOGI(TAG, "Number of tasks: %d", taskCount);
//         ESP_LOGI(TAG, "Listing all tasks:");

//         // Loop through all tasks and print their names and other info
//         for (UBaseType_t i = 0; i < taskCount; i++) {
//             ESP_LOGI(TAG, "Task Name: %s, Task State: %d, Priority: %d, Stack High Water Mark: %d",
//                      taskStatusArray[i].pcTaskName,
//                      taskStatusArray[i].eCurrentState,
//                      taskStatusArray[i].uxCurrentPriority,
//                      taskStatusArray[i].usStackHighWaterMark);
//         }

//         // Free the allocated memory after use
//         vPortFree(taskStatusArray);
//     } else {
//         ESP_LOGE(TAG, "Failed to allocate memory for task status array.");
//     }
// }



// void disable_core1() {
//     UBaseType_t numTasks = uxTaskGetNumberOfTasks();
//     TaskStatus_t *taskStatusArray = (TaskStatus_t *) pvPortMalloc(numTasks * sizeof(TaskStatus_t));

//     if (taskStatusArray != NULL) {
//         UBaseType_t taskCount = uxTaskGetSystemState(taskStatusArray, numTasks, NULL);
//         ESP_LOGI("disable_core1", "Disabling tasks on core 1:");

//         for (UBaseType_t i = 0; i < taskCount; i++) {
//             // Check if task is assigned to core 1 and is not an idle task
//             if (xTaskGetAffinity(taskStatusArray[i].xHandle) == 1 &&
//                 strcmp(taskStatusArray[i].pcTaskName, "IDLE1") != 0) {
//                 ESP_LOGI("disable_core1", "Suspending task: %s", taskStatusArray[i].pcTaskName);
//                 vTaskSuspend(taskStatusArray[i].xHandle);
//             }
//         }
//         vPortFree(taskStatusArray);  // Free allocated memory after use
//     } else {
//         ESP_LOGE("disable_core1", "Failed to allocate memory for task status array.");
//     }
// }

// void wake_up_core1() {
//     UBaseType_t numTasks = uxTaskGetNumberOfTasks();
//     TaskStatus_t *taskStatusArray = (TaskStatus_t *) pvPortMalloc(numTasks * sizeof(TaskStatus_t));

//     if (taskStatusArray != NULL) {
//         UBaseType_t taskCount = uxTaskGetSystemState(taskStatusArray, numTasks, NULL);
//         ESP_LOGI("wake_up_core1", "Waking up tasks on core 1:");

//         for (UBaseType_t i = 0; i < taskCount; i++) {
//             // Check if task is assigned to core 1 and is not an idle task
//             if (xTaskGetAffinity(taskStatusArray[i].xHandle) == 1 &&
//                 strcmp(taskStatusArray[i].pcTaskName, "IDLE1") != 0) {
//                 ESP_LOGI("wake_up_core1", "Resuming task: %s", taskStatusArray[i].pcTaskName);
//                 vTaskResume(taskStatusArray[i].xHandle);
//             }
//         }
//         vPortFree(taskStatusArray);  // Free allocated memory after use
//     } else {
//         ESP_LOGE("wake_up_core1", "Failed to allocate memory for task status array.");
//     }
// }



// #define MAX_TASKS 20  // Define a limit for the number of tasks you want to handle

// void list_and_control_tasks() {
//     TaskStatus_t taskStatusArray[MAX_TASKS];
//     UBaseType_t taskCount = uxTaskGetSystemState(taskStatusArray, MAX_TASKS, NULL);

//     ESP_LOGI("TaskList", "Total tasks: %d", taskCount);

//     for (int i = 0; i < taskCount; i++) {
//         ESP_LOGI("TaskList", "Task Name: %s, Task Handle: %p", taskStatusArray[i].pcTaskName, taskStatusArray[i].xHandle);

//         // Example: Suspend a task by name if it matches "exampleTaskName"
//         if (strcmp(taskStatusArray[i].pcTaskName, "exampleTaskName") == 0) {
//             ESP_LOGI("TaskControl", "Suspending task: %s", taskStatusArray[i].pcTaskName);
//             vTaskSuspend(taskStatusArray[i].xHandle);

//             // Optionally resume the task after a delay
//             vTaskDelay(pdMS_TO_TICKS(1000));
//             ESP_LOGI("TaskControl", "Resuming task: %s", taskStatusArray[i].pcTaskName);
//             vTaskResume(taskStatusArray[i].xHandle);
//         }
//     }
// }
