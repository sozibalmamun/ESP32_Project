#include "app_main.hpp"

extern "C" 
void app_main()
{

    sleepEnable=START;
    ESP_LOGE(TAG, "Starting app_main");
    gpioInt();

    configure_dynamic_frequency();
    sensorHandel(); //1


    sensorSemaphore = xSemaphoreCreateBinary();
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));
    xQueueCloud = xQueueCreate(3, sizeof(int *));




    if (xQueueAIFrame == NULL || xQueueLCDFrame == NULL || xQueueEventLogic == NULL || sensorSemaphore == NULL) {
        // ESP_LOGE(TAG, "Failed to create queues");
        esp_restart();
    } 
    
    // Initialize Conectivity---------------------------
    bluFiStart();
    //--------------------------------------------------
    shiftOutData.bitset.LED=1;
    shiftOutData.bitset.PEREN=1;
    shiftOutData.bitset.CAMEN=1;  
    shiftOutData.bitset.CAMPDWN=0;
    shiftOutData.bitset.ADC_EN=1;
    shiftOutData.bitset.LCDEN=1;
    if(sensorSemaphore)xSemaphoreGive(sensorSemaphore); // Notify the sensor task

    vTaskDelay(pdMS_TO_TICKS(5));

    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);//core 1    //  FRAMESIZE_QVGA 320*240  //FRAMESIZE_VGA 640x480
    register_event(xQueueEventLogic);//core 1
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame,xQueueCloud ,false); //core 1+1
    register_lcd(xQueueLCDFrame, NULL, true);// core 0

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
    shiftOutData.bitset.LED=0;  //q4
    if(checkMusicEnable())music=TURN_ON_MUSIC;
    else welcomeMusic(true);
    if(sensorSemaphore)xSemaphoreGive(sensorSemaphore); // Notify the sensor task

    ESP_LOGI(TAG, "app_main finished");
    sleepEnable=WAKEUP;

    while(true){

        // // Log or print the CPU frequency
        // int cpu_freq_mhz = esp_clk_cpu_freq() / 1000000;
        // ESP_LOGI("CPU Monitor", "Current CPU frequency: %d MHz", cpu_freq_mhz);

        // if(xTaskGetTickCount()-sleepTimeOut>3000  && sleepEnable == WAKEUP){
            
        //     sleepEnable=SLEEP;
        //     welcomeMusic(false);
        //     printf("\nsleepEnable"); 
        //     vTaskDelay(pdMS_TO_TICKS(10));
        //     shiftOutData.write=0x00;
        //     dispON(false);
        //     deinitBlufi();
        //     vTaskDelay(pdMS_TO_TICKS(10));
        //     reduce_cpu_frequency();
        // }

            
        if(sleepEnable == SLEEP){ 
           
    
            #if 0
            enter_light_sleep();  // Enter light sleep mode
            #else
            enter_deep_sleep();
            #endif


        }else {
            reconnect();
            readBatteryVoltage();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}


// void reInt(void){



//     shiftOutData.bitset.CAMPDWN=0;//q6
//     vTaskDelay(pdMS_TO_TICKS(10));
//     RtcInit();
//     vTaskDelay(pdMS_TO_TICKS(10));  // Allow time for frequency update
//     sleepEnable = WAKEUP;
//     bluFiStart();

// }



// if(sleepEnable == SLEEP){ 
//     enter_light_sleep();  // Enter light sleep mode
//     // if( WAKE_STATE ){

//     //     sleepTimeOut = xTaskGetTickCount();// imediate wake if display in sleep mode
//     //     restore_cpu_frequency();
//     //     reInt();
//     //     brightness(false);//sleep
//     //     printf("\nsleep disable");

//     // } 
    
//     // if(CHARGING_STATE){
//     //     plugIn(true);
//     //     printf("plugIn\n");

//     // }else {
//     //     plugIn(false);
//     //     printf("plug out\n");

//     // }

// }