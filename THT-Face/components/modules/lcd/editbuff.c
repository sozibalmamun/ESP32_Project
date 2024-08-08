
#include "editbuff.h"
#include "logo&Icon.h"
#include "esp_wifi.h"
#include "esp_log.h"
#include "stdint.h"

#define WIFI_WIDTH 16
#define WIFI_HEIGHT 15
#define BLE_W 11
#define BLE_H 15

#define NETWORK_ICON_POSS_X 295
#define NETWORK_ICON_POSS_Y 3

#define CHAR_WIDTH 8//910
#define CHAR_HEIGHT 10//13

#define LETTER_WIDTH 8//910
#define LETTER_HEIGHT 10//13

#define WHITE 0xFFFF
#define RED 0xf8c0
#define GREEN 0x4f00
#define GRAY 0x6b4d
#define BLACK 0x0000

#define ZERO "abcdef"
#define ONE "bc"
#define TWO "abged"
#define THREE "abcdg"
#define FOUR "bcfg"
#define FIVE "acdfg"
#define SIX "acdefg"
#define SEVEEN "abc"
#define EIGHT "abcdefg"
#define NINE "abcdfg"

uint8_t wifiStatus;
TickType_t animationTime=0; 


const uint32_t *segment_table1[]={
    ['a']=a,
    ['d']=d,
    ['g']=g

};
const uint8_t *segment_table2[]={
    ['b']=b,
    ['c']=c,
    ['e']=e,
    ['f']=f
};

const uint8_t *font_table[] = {

    ['0'] = char_0,
    ['1'] = char_1,
    ['2'] = char_2,
    ['3'] = char_3,
    ['4'] = char_4,
    ['5'] = char_5,
    ['6'] = char_6,
    ['7'] = char_7,
    ['8'] = char_8,
    ['9'] = char_9,
    ['-'] = charSignNeg

    // ['0'] = char_0,
    // ['0'] = char_0,
    // ['0'] = char_0,
};

const uint16_t *font_table1[] = {

    ['N'] = char_N,
    ['S'] = char_S

};


void editDisplayBuff(camera_fb_t **buff){

    if(true){

        segmentTime(*buff);

    }else {

        if(wifiStatus==0){

            if( xTaskGetTickCount()-animationTime< 50){

                iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);
                
                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+12,WIFI_WIDTH,3,&animationTime,WHITE,*buff);
            }else if(xTaskGetTickCount()-animationTime> 50 && xTaskGetTickCount()-animationTime< 100){
                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+8,WIFI_WIDTH,4,&animationTime,WHITE,*buff);
            }else if(xTaskGetTickCount()-animationTime> 100 && xTaskGetTickCount()-animationTime< 150){
                iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);
                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+5,WIFI_WIDTH,5,&animationTime,WHITE,*buff);
            }else if(xTaskGetTickCount()-animationTime> 150 && xTaskGetTickCount()-animationTime< 200){

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y,WIFI_WIDTH,6,&animationTime,WHITE,*buff);

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

            char tempFrame[10] ;
            snprintf(tempFrame, sizeof(tempFrame), "%09llu", generate_unique_id());
            createQrcode(tempFrame , *buff);
            writeSn(*buff);


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
void writeSn(camera_fb_t *buff){

    char tempFrame[13] ;
    snprintf(tempFrame, sizeof(tempFrame), "SN-%09llu", generate_unique_id());

    uint16_t len = (buff->width-(strlen(tempFrame)*LETTER_WIDTH))-3;//x start poss

    WriteString(len, buff->height-(LETTER_HEIGHT+3),tempFrame,buff);
}


// Function to render a string onto the display buffer
void WriteString(int x_offset, int y_offset, const char *str, camera_fb_t *buff) {
    while (*str) {
        wrightChar(x_offset, y_offset, *str, buff);
        x_offset += CHAR_WIDTH; // Move to the next character position
        str++;
    }
}

// Function to render a character onto the display buffer
void wrightChar(int x_offset, int y_offset, char c, camera_fb_t *buff) {
    
    if((c>'@'&& c<'[') ||(c>0x60 && c<'{') ){

        // Get the bitmap data for the character
        const uint16_t *char_data = font_table1[(uint8_t)c];

        // Ensure the character fits within the buffer dimensions
        if (x_offset + LETTER_WIDTH > buff->width || y_offset + LETTER_WIDTH > buff->height) {
            printf("Character position out of bounds\n");
            return;
        }

        for (int y = 0; y < LETTER_HEIGHT; y++) {
            for (int x = 0; x <= LETTER_WIDTH; x++) {

                int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
                // Get the pixel value from the character data
                if (char_data[y] & (1 << (LETTER_WIDTH-x))) {
                    // Draw white (pixel set)
                    buff->buf[buff_index] = 0xFF;
                    buff->buf[buff_index + 1] = 0xFF;
                }
            }
        }
    }else{

        // Get the bitmap data for the character
        const uint8_t *char_data = font_table[(uint8_t)c];
        // Ensure the character fits within the buffer dimensions
        if (x_offset + CHAR_WIDTH > buff->width || y_offset + CHAR_WIDTH > buff->height) {
            printf("Character position out of bounds\n");
            return;
        }

        for (int y = 0; y < CHAR_HEIGHT; y++) {
            for (int x = 0; x <= CHAR_WIDTH; x++) {
                int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
                // Get the pixel value from the character data
                if (char_data[y] & (1 << (CHAR_WIDTH-x))) {
                    // Draw white (pixel set)
                    buff->buf[buff_index] = 0xFF;
                    buff->buf[buff_index + 1] = 0xFF;
                }
            }
        }
    }

}



void segmentTime(camera_fb_t *buff){

            for (int y = 0; y < 240; y++)
            {
                for (int x = 0; x < 320; x++)
                {
                    int index = (y * buff->width + x) * 2; // Assuming 2 bytes per pixel
                    buff->buf[index] = 0;
                    buff->buf[index + 1] = 0;
                }
            }
// input time in here
    time_library_time_t current_time;
    time_library_get_time(&current_time);
    printf("Current time: %d-%d-%d %d:%d:%d\n",
        current_time.year, current_time.month, current_time.day,
        current_time.hour, current_time.minute, current_time.second);

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
    if(xTaskGetTickCount()-animationTime < 50){
        iconPrint(segmentBaseX+85,segmentBaseY+10 ,5,5 ,&secondicon,WHITE,buff);
        iconPrint(segmentBaseX+85,segmentBaseY+56,5,5 ,&secondicon,WHITE,buff);
    }else if(xTaskGetTickCount()-animationTime >100){
        animationTime = xTaskGetTickCount();
    }
}

void timeDisplay(uint8_t x, uint8_t y, uint8_t value,camera_fb_t *buff){
    
    switch(value){

        case 0:
            WriteTimeString(x,y,ZERO,buff);
        break;

        case 1:
            WriteTimeString(x,y,ONE,buff);
        break;
        
        case 2:
            WriteTimeString(x,y,TWO,buff);
        break;

        case 3:
            WriteTimeString(x,y,THREE,buff);
        break;

        case 4:
            WriteTimeString(x,y,FOUR,buff);
        break;

        case 5:
            WriteTimeString(x,y,FIVE,buff);
        break;

        case 6:
            WriteTimeString(x,y,SIX,buff);
        break;

        case 7:
            WriteTimeString(x,y,SEVEEN,buff);
        break;

        case 8:
            WriteTimeString(x,y,EIGHT,buff);
        break;

        case 9:
            WriteTimeString(x,y,NINE,buff);
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
    wrighSingle7segment(segmentBaseX+23, segmentBaseY+30,'c',buff);
    wrighSingle7segment(segmentBaseX, segmentBaseY+52,'d',buff);
    wrighSingle7segment(segmentBaseX-3, segmentBaseY+30,'e',buff);
    wrighSingle7segment(segmentBaseX-3, segmentBaseY+2,'f',buff);
    wrighSingle7segment(segmentBaseX+1, segmentBaseY+26,'g',buff);
*/

void WriteTimeString(int x_offset, int y_offset, const char *str, camera_fb_t *buff) {

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
