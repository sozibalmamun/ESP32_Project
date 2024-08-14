
#include "editbuff.h"
#include "logo&Icon.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "stdint.h"



uint8_t wifiStatus;
// uint64_t uniqueId;
volatile uint8_t sleepEnable=0;
TickType_t sleepTimeOut=0; 


void editDisplayBuff(camera_fb_t **buff){


    // time read here
    time_library_time_t current_time;
    uint8_t clockType = get_time(&current_time, 1);

    if(sleepEnable==1){// sleep time display

        sleepTimeDate(*buff,current_time);

    }else {// wekup time display

        writedateTime(*buff , current_time, clockType);

        if(wifiStatus==0){

            if( xTaskGetTickCount()-animationTime< 50){

                iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);
                
                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+12,WIFI_WIDTH,3,&wifiAnimation01,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 50 && xTaskGetTickCount()-animationTime< 100){

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+8,WIFI_WIDTH,4,&wifiAnimation02,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 100 && xTaskGetTickCount()-animationTime< 150){

                iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+5,WIFI_WIDTH,5,&wifiAnimation03,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 150 && xTaskGetTickCount()-animationTime< 200){

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y,WIFI_WIDTH,6,&wifiAnimation04,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 250){
                animationTime = xTaskGetTickCount();
            }
            iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,7,7 ,&noWifiIcon,RED,*buff);

            for (int y = qrInfo.yOfset-3; y < qrInfo.yOfset-3 + qrInfo.erase_size; y++)
            {
                for (int x = qrInfo.xOfset-3; x < qrInfo.xOfset-3 + qrInfo.erase_size; x++)
                {
                    int index = (y * (*buff)->width + x) * 2; // Assuming 2 bytes per pixel

                    (*buff)->buf[index] = 0xff;
                    (*buff)->buf[index + 1] = 0xff;
                
                }
            }

            // char tempFrame[15] ;
            // snprintf(tempFrame, sizeof(tempFrame), "%09llu", generate_unique_id());//uniqueId
            // createQrcode(tempFrame , *buff);
            // writeSn(*buff);

            // uint64_t Id= generate_unique_id();
            // char tempFrame[15] ;
            // snprintf(tempFrame, sizeof(tempFrame), "%s%010llu",DEVICE_VERSION_ID, Id);//uniqueId
            // createQrcode(tempFrame , *buff);
            // writeSn(*buff, Id);
        


        }else 
        {
            iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y,WIFI_WIDTH,WIFI_HEIGHT,&wifiIcon,WHITE,*buff);

            if(wifiStatus==2){

                iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,7,7 ,&connectedIcon,GREEN,*buff);

            }else{

                iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,2,7,&disconnectedIcon,RED,*buff);

            }
            animationTime = xTaskGetTickCount();
        }
    }

}

void iconPrint(int x_offset, int y_offset, uint8_t w, uint8_t h,char* logobuff,uint16_t color ,camera_fb_t *buff) {
    // Ensure logo fits within the buffer dimensions
    if (x_offset + w > buff->width || y_offset + h > buff->height) {
        printf("Logo position out of bounds\n");
        
        return;
    }
    for (int y = 0; y < h; y++) {
        for (int x = 0; x < w; x++) {
            int logo_index = y * w + x;
            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel

            // Copy the logo pixel to the buffer
            uint8_t pixel = logobuff[logo_index];
            if(pixel==0){// white color
            buff->buf[buff_index] = color>>8;
            buff->buf[buff_index + 1] = color&0xff;
            }
        }
    }
}
void writeSn(camera_fb_t *buff ,uint64_t id){

    char tempFrame[17] ;
    snprintf(tempFrame, sizeof(tempFrame), "SN-%s%09llu",DEVICE_VERSION_ID, id);

    uint16_t len = (buff->width-(pixleLen(1,&tempFrame )))-3;   //x start poss

    WriteString(1,len, buff->height-(tablehight[1]+3),tempFrame,buff);
}

void writedateTime(camera_fb_t *buff ,time_library_time_t current_time,uint8_t clockType){

//2024-08-10 3.47 PM

    char tempFrame[30] ;
    snprintf(tempFrame, sizeof(tempFrame), "%d-%d-%d   %d.%d %s",current_time.year,current_time.month,current_time.day, 
    current_time.hour, current_time.minute, clockType==1 ? "PM" : clockType==2?"AM" :" ");
    // printf("\nclock type %d",clockType);
    WriteString(1,4,5,tempFrame,buff);

}


// Function to render a string onto the display buffer
void WriteString(uint8_t letterSize, int x_offset, int y_offset, const char *str, camera_fb_t *buff) {
    
    uint8_t letterWidth;

    while (*str) {

        wrightChar(letterSize,x_offset, y_offset, *str, buff);

        if(letterSize==1){

            letterWidth = table0len[(uint8_t)*str];
            x_offset += (letterWidth+1); // Move to the next character position
            // printf("\npixle len %d for %c",letterWidth ,*str);

        }else if(letterSize==2){

            letterWidth = table1len[(uint8_t)*str];
            x_offset += (letterWidth+1); // Move to the next character position
            // printf("\npixle len %d for %c",letterWidth ,*str);

        }
        str++;
    }
}

// Function to render a character onto the display buffer
void wrightChar(uint8_t letterSize, int x_offset, int y_offset, char c, camera_fb_t *buff) {
    

    // Get the bitmap data for the character
    const uint16_t *char_data=NULL;
    uint8_t letterWidth = 0;
    if(letterSize==1){

        char_data = font_table0[(uint8_t)c];
        letterWidth = table0len[(uint8_t)c];

    }else if(letterSize==2){

        char_data = font_table1[(uint8_t)c];
        letterWidth = table1len[(uint8_t)c];

    }

   // Ensure the character fits within the buffer dimensions
    if (x_offset + letterWidth > buff->width || y_offset + letterWidth > buff->height) {
        // printf("Character position out of bounds\n");
        return;
    }


    for (int y = 0; y < tablehight[letterSize] ; y++) {
        for (int x = 0; x <= letterWidth; x++) {

            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
            // Get the pixel value from the character data
            if (char_data[y] & (1 << (16-x))) {
                // Draw white (pixel set)
                buff->buf[buff_index] = 0xFF;
                buff->buf[buff_index + 1] = 0xFF;
            }
        }
    }

}



void sleepTimeDate(camera_fb_t *buff, time_library_time_t current_time){

    for (int y = 0; y < 240; y++)
    {
        for (int x = 0; x < 320; x++)
        {
            int index = (y * buff->width + x) * 2; // Assuming 2 bytes per pixel
            buff->buf[index] = 0;
            buff->buf[index + 1] = 0;
        }
    }

    uint8_t tempHours= current_time.hour;
    uint8_t tempMinuts= current_time.minute;
//170-87
#define segmentBaseX  73
#define segmentBaseY  58 
// write hours
    if(tempHours<=9){
    timeDisplay( segmentBaseX, segmentBaseY, 0 , buff);
    timeDisplay( segmentBaseX+44, segmentBaseY, tempHours , buff);
    }else { 
    timeDisplay( segmentBaseX, segmentBaseY, tempHours/10 , buff);
    timeDisplay( segmentBaseX+44, segmentBaseY, tempHours%10 , buff);
    }
// write minuts
    if(tempMinuts<=9){
        timeDisplay( segmentBaseX+100, segmentBaseY, 0 , buff);
        timeDisplay( segmentBaseX+144, segmentBaseY, tempMinuts , buff);
    }else if(tempMinuts==0){
        timeDisplay( segmentBaseX+100, segmentBaseY, 0 , buff);
        timeDisplay( segmentBaseX+144, segmentBaseY, 0 , buff);
    }else{ 
        timeDisplay( segmentBaseX+100, segmentBaseY, tempMinuts / 10 , buff);
        timeDisplay( segmentBaseX+144, segmentBaseY, tempMinuts % 10 , buff);
    }
// secend dot toggole
    if(xTaskGetTickCount()-animationTime <50){
        iconPrint(segmentBaseX+85,segmentBaseY+10 ,5,5 ,&secondicon,WHITE,buff);
        iconPrint(segmentBaseX+85,segmentBaseY+56,5,5 ,&secondicon,WHITE,buff);
    }else if(xTaskGetTickCount()-animationTime >140){
        animationTime = xTaskGetTickCount();
    }
// date 2024-08-08 day
    char tempFrame[17] ;
    snprintf(tempFrame, sizeof(tempFrame), "%d-%d-%d  %s",current_time.year,current_time.month,current_time.day,
    day_names[calculate_day_of_week( current_time.year, current_time.month, current_time.day )]);

    uint16_t len = 160- (pixleLen(2,&tempFrame)/2);//x start poss
    WriteString(2,len, 130,tempFrame,buff);

/*
    WriteString(2,5, 0,"abcdefghijklmnopqrstuvwxyz",buff);
    WriteString(2,5, 130,"ABCDEFGHIJKLMNOPQRSTUVWXYZ",buff);
    WriteString(2,5, 160,"1234567890",buff);
*/

}
uint16_t pixleLen(uint8_t letSize, char *str){

    uint16_t len=0;
        while (*str) {
            if(letSize==1){
            len += table0len[(uint8_t)*str];
            }else if(letSize==2){
            len += table1len[(uint8_t)*str];
            }
            str++;
        }

    return len;

}
void timeDisplay(uint8_t x, uint8_t y, uint8_t value,camera_fb_t *buff){
    
    switch(value){

        case 0:
            WriteMulti7segment(x,y,ZERO,buff);
        break;

        case 1:
            WriteMulti7segment(x,y,ONE,buff);
        break;
        
        case 2:
            WriteMulti7segment(x,y,TWO,buff);
        break;

        case 3:
            WriteMulti7segment(x,y,THREE,buff);
        break;

        case 4:
            WriteMulti7segment(x,y,FOUR,buff);
        break;

        case 5:
            WriteMulti7segment(x,y,FIVE,buff);
        break;

        case 6:
            WriteMulti7segment(x,y,SIX,buff);
        break;

        case 7:
            WriteMulti7segment(x,y,SEVEEN,buff);
        break;

        case 8:
            WriteMulti7segment(x,y,EIGHT,buff);
        break;

        case 9:
            WriteMulti7segment(x,y,NINE,buff);
        break;

        default:
        break;
        
    }
}

/*
#define segmentBaseX  20
#define segmentBaseY  20
    wrighSingle7segment(segmentBaseX, segmentBaseY,'a',buff);
    wrighSingle7segment(segmentBaseX+23, segmentBaseY+2,'b',buff);
    wrighSingle7segment(segmentBaseX+23, segmentBaseY+32,'c',buff);
    wrighSingle7segment(segmentBaseX, segmentBaseY+54,'d',buff);
    wrighSingle7segment(segmentBaseX-3, segmentBaseY+32,'e',buff);
    wrighSingle7segment(segmentBaseX-3, segmentBaseY+2,'f',buff);
    wrighSingle7segment(segmentBaseX+1, segmentBaseY+27,'g',buff);
*/
void WriteMulti7segment(int x_offset, int y_offset, const char *str, camera_fb_t *buff) {

    while (*str) {

        switch (*str)
        {

        case 'a':
            wrighSingle7segment(x_offset, y_offset,'a',buff);
            break;

        case 'b':
            wrighSingle7segment(x_offset+23, y_offset+2,'b',buff);
            break; 

        case 'c':
            wrighSingle7segment(x_offset+23, y_offset+32,'c',buff);
            break;

        case 'd':
            wrighSingle7segment(x_offset, y_offset+54,'d',buff);
            break;

        case 'e':
            wrighSingle7segment(x_offset-3, y_offset+32,'e',buff);
            break;

        case 'f':
            wrighSingle7segment(x_offset-3, y_offset+2,'f',buff);
            break;
        
        case 'g':
            wrighSingle7segment(x_offset+1, y_offset+27,'g',buff);
            break;
        
        default:
            break;

        }
        str++;
    }
}
// Function to render a character onto the display buffer a
void wrighSingle7segment(int x_offset, int y_offset, char c, camera_fb_t *buff) {
    
    if(c=='a'|| c=='g'||c=='d'){

#define HEIGHT_32 6
#define WIDTH_32 30

        // Get the bitmap data for the character
        const uint32_t *char_data = segment_table1[(uint8_t)c];

        // Ensure the character fits within the buffer dimensions
        if (x_offset + WIDTH_32 > buff->width || y_offset + WIDTH_32 > buff->height) {
            printf("Character position out of bounds\n");
            return;
        }

        for (int y = 0; y < HEIGHT_32; y++) {
            for (int x = 0; x <= WIDTH_32; x++) {

                int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
                // Get the pixel value from the character data
                if (char_data[y] & (1 << (WIDTH_32-x))) {
                    // Draw white (pixel set)
                    buff->buf[buff_index] = 0xFF;
                    buff->buf[buff_index + 1] = 0xFF;
                }
            }
        }

    }else{

#define HEIGHT_8 26
#define WIDTH_8 8

        // Get the bitmap data for the character
        const uint8_t *char_data = segment_table2[(uint8_t)c];
        // Ensure the character fits within the buffer dimensions
        if (x_offset + WIDTH_8 > buff->width || y_offset + WIDTH_8 > buff->height) {
            printf("Character position out of bounds\n");
            return;
        }

        for (int y = 0; y < HEIGHT_8; y++) {
            for (int x = 0; x <= WIDTH_8; x++) {
                int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
                // Get the pixel value from the character data
                if (char_data[y] & (1 << (WIDTH_8-x))) {
                    // Draw white (pixel set)
                    buff->buf[buff_index] = 0xFF;
                    buff->buf[buff_index + 1] = 0xFF;
                }
            }
        }
    }

}
