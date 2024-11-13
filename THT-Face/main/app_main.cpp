#include "app_main.hpp"

extern "C" 
void app_main()
{

    ESP_LOGE(TAG, "Starting app_main");
    gpioInt();
    configure_dynamic_frequency();
    // Initialize Conectivity----------------------------
    bluFiStart();
    //--------------------------------------------------

    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));
    xQueueCloud = xQueueCreate(3, sizeof(int *));

    if (xQueueAIFrame == NULL || xQueueLCDFrame == NULL || xQueueEventLogic == NULL) {
        // ESP_LOGE(TAG, "Failed to create queues");
        esp_restart();
    }
    
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);//core 1    
    register_event(xQueueEventLogic);//core 1
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame,xQueueCloud ,false); //core 1+1
    register_lcd(xQueueLCDFrame, NULL, true);// core 0
    vTaskDelay(pdMS_TO_TICKS(10));

    //-------------------------
    // Initialize and mount FATFS
    if (init_fatfs()== ESP_OK) {
        
        print_memory_status();
        create_directories();
    }


    //-----------time int here-------------------------------------
    RtcInit();
    //--------------------------------------------------------------
    init_adc();
    //-------------------------
    // Initialize PWM using the PwmInt function
    PwmInt(&ledc_channel,(gpio_num_t)LCE_BL);
    ESP_LOGI(TAG, "app_main finished");

    while(true){

        
        // // Log or print the CPU frequency
        // int cpu_freq_mhz = esp_clk_cpu_freq() / 1000000;
        // ESP_LOGI("CPU Monitor", "Current CPU frequency: %d MHz", cpu_freq_mhz);
        // esp_pm_dump_locks(stdout);
        // list_all_tasks();


        if(xTaskGetTickCount()-sleepTimeOut>3000 && /*xTaskGetTickCount()-sleepTimeOut< 3500 &&*/ sleepEnable == WAKEUP){

            sleepEnable=SLEEP;
            printf("\nsleepEnable");
            gpio_set_level((gpio_num_t)CAM_CONTROL, 1);
            ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel, BRIGHTNESS(SLEEP_LCD));//8192
            ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);
            deinitBlufi();
            vTaskDelay(pdMS_TO_TICKS(2000));
            reduce_cpu_frequency();
            vTaskDelay(pdMS_TO_TICKS(2000));
        }

        if(sleepEnable == SLEEP){
            // enter_light_sleep();  // Enter light sleep mode
            if( WAKE  || CHARGING ){
                sleepTimeOut = xTaskGetTickCount();// imediate wake if display in sleep mode
                restore_cpu_frequency();
                reInt();
                ledc_set_duty(ledc_channel.speed_mode, ledc_channel.channel,BRIGHTNESS(WAKE_LCD));
                ledc_update_duty(ledc_channel.speed_mode, ledc_channel.channel);  
                printf("\nsleep disable");

            }

        }else {

            reconnect();
            readBatteryVoltage();
            vTaskDelay(pdMS_TO_TICKS(1000));  // Update every 1 second

        }

    }
}

void reInt(void){

    gpio_set_level((gpio_num_t)CAM_CONTROL, 0);
    RtcInit();
    sleepEnable = WAKEUP;
    bluFiStart();

}

