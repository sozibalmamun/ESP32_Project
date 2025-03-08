#include "app_main.hpp"

extern "C" 
void app_main()
{

    ESP_LOGE(TAG, "Starting app_main");
    gpioInt();
    init_adc();
    //-------------------------

    configure_dynamic_frequency();
    manageMusicShiftreg();

    musicShiftSemaphore = xSemaphoreCreateBinary();
    xQueueAIFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueLCDFrame = xQueueCreate(2, sizeof(camera_fb_t *));
    xQueueEventLogic = xQueueCreate(1, sizeof(int *));
    xQueueCloud = xQueueCreate(3, sizeof(int *));

    if (xQueueAIFrame == NULL || xQueueLCDFrame == NULL || xQueueEventLogic == NULL || musicShiftSemaphore == NULL || xQueueCloud == NULL) {
        // ESP_LOGE(TAG, "Failed to create queues");
        esp_restart();
    } 

    shiftOutData.bitset.LED=1;
    shiftOutData.bitset.PEREN=1;
    shiftOutData.bitset.CAMEN=1;  
    shiftOutData.bitset.CAMPDWN=0;
    shiftOutData.bitset.ADC_EN=1;
    shiftOutData.bitset.LCDEN=1;
    if(musicShiftSemaphore)xSemaphoreGive(musicShiftSemaphore); 

    // Initialize Conectivity---------------------------
    bluFiStart();

    register_lcd(xQueueLCDFrame, NULL, true);// core 0
    register_camera(PIXFORMAT_RGB565, FRAMESIZE_QVGA, 2, xQueueAIFrame);//core 1    //  FRAMESIZE_QVGA 320*240  //FRAMESIZE_VGA 640x480
    register_event(xQueueEventLogic);//core 1
    register_human_face_recognition(xQueueAIFrame, xQueueEventLogic, NULL, xQueueLCDFrame,xQueueCloud ,false); //core 1+1

    //-------------------------
    // Initialize and mount FATFS
    if (init_fatfs()== ESP_OK) {
        
        create_directories();
        
    }
    //-----------time int here-------------------------------------
    RtcInit();
    //--------------------------------------------------------------
    shiftOutData.bitset.LED=0;  //q4
    if(checkMusicEnable())music=TURN_ON_MUSIC;
    else welcomeMusic(true);
    if(musicShiftSemaphore)xSemaphoreGive(musicShiftSemaphore); // Notify the sensor task

    ESP_LOGI(TAG, "app_main finished");

    while(true){

        if(xTaskGetTickCount()-sleepTimeOut>TIMEOUT_30_S  && sleepEnable == WAKEUP && CPUBgflag==0){
            
            sleepEnable=SLEEP;
            welcomeMusic(false);
            deinitBlufi();
            shiftOutData.write=0x00;
            dispON(false);
            if(musicShiftSemaphore)xSemaphoreGive(musicShiftSemaphore); // Notify the sensor task
            reduce_cpu_frequency();
        
        }

        if(sleepEnable == SLEEP){ 
        
            enter_deep_sleep();

        }else {
            ensureLogDelivery();
            fetchBatteryPirStatus();
            vTaskDelay(pdMS_TO_TICKS(100));
        }

    }
}



void enter_deep_sleep(void){

    ESP_LOGI("DEEP_SLEEP", "Going to deep sleep...");
    // Enable wake-up on button press (when GPIO is LOW)
    // gpio_pullup_en(PIR);  // Enable pull-up resistor
    esp_sleep_enable_ext0_wakeup(PIR, 0); // Wake-up when button is pressed (LOW)
    // Enter deep sleep
    esp_deep_sleep_start();
    // This line will **never execute** because ESP restarts after wake-up
    // ESP_LOGI("DEEP_SLEEP", "Woke up!");
    // sleepEnable = WAKEUP;  // Disable sleep mode after waking up
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