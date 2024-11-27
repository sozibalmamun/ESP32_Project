#include "gpioControl.h"

uint16_t batVoltage;
uint8_t chargeState=0;


void gpioInt(void){

    // gpio_set_level((gpio_num_t)LCE_BL, 0);
    // gpio_pad_select_gpio(LCE_BL);
    // gpio_set_direction((gpio_num_t)LCE_BL, GPIO_MODE_OUTPUT);
    // gpio_set_level((gpio_num_t)LCE_BL, 0);

   
    gpio_pad_select_gpio(CAM_CONTROL);
    gpio_set_direction((gpio_num_t)CAM_CONTROL, GPIO_MODE_OUTPUT);
    gpio_set_level((gpio_num_t)CAM_CONTROL, 0);

    gpio_config_t io_conf = {
        .intr_type = GPIO_INTR_DISABLE,
        .pin_bit_mask = (1ULL << GPIO_WAKEUP_BUTTON), // Pin mask
        .mode = GPIO_MODE_INPUT,                  // Set as input
        .pull_up_en = GPIO_PULLUP_ENABLE,         // Enable pull-up resistor
        .pull_down_en = GPIO_PULLDOWN_DISABLE     // Disable pull-down resistor
    };
    gpio_config(&io_conf);

    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << BATTERY_CHARGE_STATE); // Pin mask
    io_conf.mode = GPIO_MODE_INPUT;                  // Set as input
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;         // Enable pull-up resistor
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

    // // Configure LEDC channel for GPIO 14
    // ledc_channel->gpio_num   = pinNo;                // GPIO pin for PWM output
    // ledc_channel->speed_mode = LEDC_LOW_SPEED_MODE;        // Low-speed mode
    // ledc_channel->channel    = LEDC_CHANNEL_0;             // Channel: 0
    // ledc_channel->intr_type  = LEDC_INTR_DISABLE;          // Disable fade interrupt
    // ledc_channel->timer_sel  = LEDC_TIMER_0;               // Select Timer 0
    // ledc_channel->duty       = 0;                          // Initial duty cycle: 0
    // ledc_channel->hpoint     = 0;                          // Hpoint: 0
    // ledc_channel->flags.output_invert = 0;                 // Disable output inversion

    // Initialize the channel
    ledc_channel_config(&ledc_channel);

    // Set initial duty cycle to 50% (for 13-bit resolution, this is 4096 out of 8192)
    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, 8192);
    ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);

    // Optional: Use fading
    ledc_fade_func_install(0);  // Install the fade function
    ledc_set_fade_time_and_start(ledc_channel.speed_mode, ledc_channel.channel, 8192, 1000, LEDC_FADE_NO_WAIT);
    for (int duty = 8192; duty >= 0; duty -= 64) {
        ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, duty);
        ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
        vTaskDelay(100 / portTICK_PERIOD_MS);     // Delay to see the dimming effect
    }
}


void brightness(bool sleep){
    
    ledc_channel_config_t ledc_channel={

        .speed_mode = LEDC_LOW_SPEED_MODE,        // Low-speed mode
        .channel    = LEDC_CHANNEL_0,             // Channel: 0

    };

    ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, sleep?BRIGHTNESS(SLEEP_LCD): BRIGHTNESS(WAKE_LCD));//8192
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


    // ESP_LOGE("Frequency", "delete all task");
    // esp_pm_dump_locks(stdout);

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

    if (lcdTaskHandler) vTaskResume(lcdTaskHandler);

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

}


// Function to enter light sleep mode--------------------------------------

void configure_wakeup() {
    // Configure the button GPIO for input with pull-up, active-low
    gpio_pad_select_gpio(GPIO_WAKEUP_BUTTON);
    gpio_set_direction(GPIO_WAKEUP_BUTTON, GPIO_MODE_INPUT);
    gpio_pullup_en(GPIO_WAKEUP_BUTTON);  // Enable pull-up for stable input

    // Enable external wakeup on GPIO_WAKEUP_BUTTON, active low
    esp_sleep_enable_ext0_wakeup(GPIO_WAKEUP_BUTTON, 0);  // Wake on falling edge (button press)
}
void enter_light_sleep(void) {

    configure_wakeup();
    vTaskDelay(200);
    ESP_LOGI("Sleep", "Entering light sleep...");
    gpio_set_level((gpio_num_t)CAM_CONTROL, 1);  // Ensure peripherals are powered off or set to sleep state

    // Set the wake-up source to external (GPIO_BOOT, active low)
    // esp_sleep_enable_timer_wakeup(1000000 * 120); // Wake up every 120 seconds (in microseconds)
    // Enter light sleep
    esp_light_sleep_start();
    ESP_LOGI("Sleep", "Woke up from light sleep!");
    sleepEnable = WAKEUP;  // Disable sleep mode after waking up
}
//------------------------------------------------------------------------------


void init_adc() {
    // Configure ADC width and attenuation for ADC2
    adc2_config_channel_atten(ADC_CHANNEL, ADC_ATTEN_DB_11);  // 0-3.6V range
    adc_chars = (esp_adc_cal_characteristics_t*) calloc(1, sizeof(esp_adc_cal_characteristics_t));
    esp_adc_cal_characterize(ADC_UNIT_2, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, DEFAULT_VREF, adc_chars);
}


// Function to read ADC and calculate voltage
void readBatteryVoltage() {
    uint32_t adc_reading = 0;
    int raw;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        // Read ADC2 channel (this call returns the raw value directly)
        if (adc2_get_raw(ADC_CHANNEL, ADC_WIDTH_BIT_12, &raw) == ESP_OK) {
            adc_reading += raw;
        }
    }
    adc_reading /= NO_OF_SAMPLES;
    batVoltage=esp_adc_cal_raw_to_voltage(adc_reading, adc_chars);
    printf("Battery Voltage: %d mV\n", batVoltage);

}


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
