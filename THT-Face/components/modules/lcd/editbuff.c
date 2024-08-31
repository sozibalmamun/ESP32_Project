
#include "stdint.h"
#include "math.h"
#include "editbuff.h"
#include "logo&Icon.h"
#include "esp_wifi.h"
#include "esp_log.h"



volatile uint8_t sleepEnable=0;
volatile TickType_t sleepTimeOut=0; 


void editDisplayBuff(camera_fb_t **buff){


    // time read here
    time_library_time_t current_time;
    uint8_t clockType = get_time(&current_time, 1);

    if(sleepEnable==1){// sleep time display

        sleepTimeDate(*buff,current_time);

    }else {// wekup time display


        if(networkStatus==WIFI_DISS){

            if(ble_is_connected)iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);


            if( xTaskGetTickCount()-animationTime< 50){

            if(!ble_is_connected)iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);
                
                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+12,WIFI_WIDTH,3,&wifiAnimation01,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 50 && xTaskGetTickCount()-animationTime< 100){

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+8,WIFI_WIDTH,4,&wifiAnimation02,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 100 && xTaskGetTickCount()-animationTime< 150){

            if(!ble_is_connected)iconPrint(NETWORK_ICON_POSS_X-15,NETWORK_ICON_POSS_Y,BLE_W,BLE_H,&bleIcon,WHITE,*buff);

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y+5,WIFI_WIDTH,5,&wifiAnimation03,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 150 && xTaskGetTickCount()-animationTime< 200){

                iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y,WIFI_WIDTH,6,&wifiAnimation04,WHITE,*buff);

            }else if(xTaskGetTickCount()-animationTime> 250){
                animationTime = xTaskGetTickCount();
            }
            iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,7,7 ,&noWifiIcon,RED,*buff);

            for (uint8_t y = qrInfo.yOfset-3; y < qrInfo.yOfset-3 + qrInfo.erase_size; y++)
            {
                for (uint16_t x = qrInfo.xOfset-3; x < qrInfo.xOfset-3 + qrInfo.erase_size; x++)
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
            char tempFrame[14] ;
            snprintf(tempFrame, sizeof(tempFrame), "%s%9llu",DEVICE_VERSION_ID, generate_unique_id());//uniqueId
            createQrcode(tempFrame , *buff);
            writeSn(*buff, generate_unique_id());
        
        }else 
        {

                       
            if(networkStatus==STOMP_CONNECTED && key_state== KEY_IDLE ){
                
                display_faces( *buff);//face display at the uploading time

                if(percentage>0){
                
                    char tempFrame[13] ;
                    snprintf(tempFrame, sizeof(tempFrame), "%d%s",percentage,"%");
                    WriteString(0,150,145,tempFrame,*buff);
                    memset(tempFrame,0,sizeof(tempFrame));
                    snprintf(tempFrame, sizeof(tempFrame), "%s%s","Uploading",  percentage%10<=3?".":percentage%10<=6?".." : percentage%10<=9? "...":" " );
                    WriteString(0,140,160,"Face",*buff);

                    WriteString(0,130,175,tempFrame,*buff);

                }             
                
            }

            iconPrint(NETWORK_ICON_POSS_X,NETWORK_ICON_POSS_Y,WIFI_WIDTH,WIFI_HEIGHT,&wifiIcon,WHITE,*buff);
            if(networkStatus==STOMP_CONNECTED){

                iconPrint(NETWORK_ICON_POSS_X+14,NETWORK_ICON_POSS_Y+8,7,7 ,&connectedIcon,GREEN,*buff);
            }else{
                iconPrint(NETWORK_ICON_POSS_X+15,NETWORK_ICON_POSS_Y+9,2,7,&disconnectedIcon,RED,*buff);
            }
            animationTime = xTaskGetTickCount();

        }

        writedateTime(*buff , current_time, clockType);


    }
}

void iconPrint(uint16_t x_offset, uint8_t y_offset, uint8_t w, uint8_t h,char* logobuff,uint16_t color ,camera_fb_t *buff) {
    // Ensure logo fits within the buffer dimensions
    if (x_offset + w > buff->width || y_offset + h > buff->height) {
        // printf("Logo position out of bounds\n");
        
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




// Function to scale an image to fit within a fixed-size frame buffer and display at a specific position
void scaleAndDisplayImageInFrame(uint8_t *src_image, uint8_t src_width, uint8_t src_height, camera_fb_t *dst_buff, uint8_t pos_x, uint8_t pos_y) {
    int8_t frame_width = 90;   // Fixed frame width
    int8_t frame_height = 101; // Fixed frame height


    drawFilledRoundedRectangle(pos_x-16,pos_y-5, frame_width+32 , frame_height+55, 3, 5,  0xc618,dst_buff);// round big ractrangle fill frame

    // Calculate scaling factors
    float scale_x = (float)src_width / frame_width;
    float scale_y = (float)src_height / frame_height;

    // Loop over each pixel in the scaled image
    for (int y = 1; y < frame_height; y++) {
        for (int x = 1; x < frame_width; x++) {
            // Calculate the corresponding pixel in the source image
            int src_x = (int)(x * scale_x);
            int src_y = (int)(y * scale_y);

            // Ensure the source coordinates are within bounds
            if (src_x >= src_width) src_x = src_width - 1;
            if (src_y >= src_height) src_y = src_height - 1;

            // Calculate the source buffer index
            int src_index = (src_y * src_width + src_x) * 2; // 2 bytes per pixel for RGB565

            // Calculate the destination buffer index with the specified position offset
            uint8_t dst_x = pos_x + x;
            uint8_t dst_y = pos_y + y;

            // Ensure the destination coordinates are within bounds
            if (dst_x >= dst_buff->width || dst_y >= dst_buff->height) {
                continue; // Skip if out of bounds
            }

            int dst_index = (dst_y * dst_buff->width + dst_x) * 2; // 2 bytes per pixel for RGB565

            // Copy the pixel data from the source image to the destination buffer
            dst_buff->buf[dst_index] = src_image[src_index];     // High byte of RGB565
            dst_buff->buf[dst_index + 1] = src_image[src_index + 1]; // Low byte of RGB565
        }
    }

    drawRoundedRectangleBorder(pos_x-1, pos_y-1, frame_width+2,frame_height+2, 3, 8, 0xc618,dst_buff); // image round frame
    drawRoundedRectangleBorder(pos_x+2, pos_y+2, frame_width-3,frame_height-3, 1, 8, 0x07e3,dst_buff); //image round reame


}



// Helper function to set a pixel in the framebuffer
// void setPixel(camera_fb_t *buff, int x, int y, uint16_t color) {
//     if (x >= 0 && x < buff->width && y >= 0 && y < buff->height) {
//         int index = (y * buff->width + x) * 2; // 2 bytes per pixel for RGB565
//         buff->buf[index] = color >> 8; // High byte of RGB565
//         buff->buf[index + 1] = color & 0xFF; // Low byte of RGB565
//     }
// }

// // Function to draw a rounded rectangle border
void drawRoundedRectangleBorder(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint8_t corner_radius, uint16_t color, camera_fb_t *buff) {

    // Ensure the rectangle fits within the buffer dimensions
    if (x_offset + width > buff->width || y_offset + height > buff->height) {
        // Out of bounds, do nothing
        return;
    }

    // Draw the border
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2;

            // Draw the top and bottom borders within the thickness range
            if ((y < thickness || y >= height - thickness) && (x >= corner_radius && x < width - corner_radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }

            // Draw the left and right borders within the thickness range
            if ((x < thickness || x >= width - thickness) && (y >= corner_radius && y < height - corner_radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }
        }
    }

    // Draw the rounded corners for the border
    for (int i = 0; i < thickness; i++) {
        for (int angle = 0; angle < 90; angle++) {
            int x = corner_radius - (int)(corner_radius * cos(angle * M_PI / 180.0));
            int y = corner_radius - (int)(corner_radius * sin(angle * M_PI / 180.0));

            for(int8_t i =-1; i<thickness-1;i++){

                // Top-left corner
                int tl_index = ((y+i + y_offset) * buff->width + (x+i + x_offset)) * 2;
                buff->buf[tl_index] = color >> 8;
                buff->buf[tl_index + 1] = color & 0xFF;

                // Top-right corner
                int tr_index = ((y+i + y_offset) * buff->width + (width - x - 1 + x_offset-i)) * 2;
                buff->buf[tr_index] = color >> 8;
                buff->buf[tr_index + 1] = color & 0xFF;

                // Bottom-left corner
                int bl_index = ((height - y - 1 + y_offset-i) * buff->width + (x + x_offset+i)) * 2;
                buff->buf[bl_index] = color >> 8;
                buff->buf[bl_index + 1] = color & 0xFF;

                // Bottom-right corner
                int br_index = ((height - y - 1 + y_offset-i) * buff->width + (width - x - 1 + x_offset-i)) * 2;
                buff->buf[br_index] = color >> 8;
                buff->buf[br_index + 1] = color & 0xFF;
            }

        }
    }
}


void drawFilledRoundedRectangle(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint8_t radius, uint16_t color, camera_fb_t *buff) {
    // Ensure the rectangle fits within the buffer dimensions
    if (x_offset + width > buff->width || y_offset + height > buff->height) {
        // Out of bounds, do nothing
        return;
    }

    // Draw filled interior and border
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            // Calculate the current position within the buffer
            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2;

            // Condition to draw the interior
            if ((x >= radius && x < width - radius) || (y >= radius && y < height - radius)) {
                buff->buf[buff_index] = color >> 8;     // High byte of RGB565
                buff->buf[buff_index + 1] = color & 0xFF; // Low byte of RGB565
            }

            // Draw top-left rounded corner
            if ((x < radius && y < radius) && ((x - radius) * (x - radius) + (y - radius) * (y - radius) < radius * radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }

            // Draw top-right rounded corner
            if ((x >= width - radius && y < radius) && ((x - (width - radius - 1)) * (x - (width - radius - 1)) + (y - radius) * (y - radius) < radius * radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }

            // Draw bottom-left rounded corner
            if ((x < radius && y >= height - radius) && ((x - radius) * (x - radius) + (y - (height - radius - 1)) * (y - (height - radius - 1)) < radius * radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }

            // Draw bottom-right rounded corner
            if ((x >= width - radius && y >= height - radius) && ((x - (width - radius - 1)) * (x - (width - radius - 1)) + (y - (height - radius - 1)) * (y - (height - radius - 1)) < radius * radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }
        }
    }

    // Draw the border
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2;

            // Draw the top and bottom borders within the thickness range
            if ((y < thickness || y >= height - thickness) && (x >= radius && x < width - radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }

            // Draw the left and right borders within the thickness range
            if ((x < thickness || x >= width - thickness) && (y >= radius && y < height - radius)) {
                buff->buf[buff_index] = color >> 8;
                buff->buf[buff_index + 1] = color & 0xFF;
            }
        }
    }

    // Draw the rounded corners for the border
    for (int i = 0; i < thickness; i++) {
        for (int angle = 0; angle < 90; angle++) {
            int x = radius - (int)(radius * cos(angle * M_PI / 180.0));
            int y = radius - (int)(radius * sin(angle * M_PI / 180.0));

            for(int8_t i =0; i<thickness;i++){

            // Top-left corner
            int tl_index = ((y+i + y_offset) * buff->width + (x+i + x_offset)) * 2;
            buff->buf[tl_index] = color >> 8;
            buff->buf[tl_index + 1] = color & 0xFF;

            // Top-right corner
            int tr_index = ((y+i + y_offset) * buff->width + (width - x - 1 + x_offset-i)) * 2;
            buff->buf[tr_index] = color >> 8;
            buff->buf[tr_index + 1] = color & 0xFF;

            // Bottom-left corner
            int bl_index = ((height - y - 1 + y_offset-i) * buff->width + (x + x_offset+i)) * 2;
            buff->buf[bl_index] = color >> 8;
            buff->buf[bl_index + 1] = color & 0xFF;

            // Bottom-right corner
            int br_index = ((height - y - 1 + y_offset-i) * buff->width + (width - x - 1 + x_offset-i)) * 2;
            buff->buf[br_index] = color >> 8;
            buff->buf[br_index + 1] = color & 0xFF;
            }

        }
    }
}



void writeSn(camera_fb_t *buff ,uint64_t id){

    char tempFrame[19] ;
    snprintf(tempFrame, sizeof(tempFrame), "SN-%s%9llu",DEVICE_VERSION_ID, id);

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
void WriteString(uint8_t letterSize, uint16_t x_offset, uint8_t y_offset, const char *str, camera_fb_t *buff) {
    
    uint8_t letterWidth;

    while (*str) {

        wrightChar(letterSize,x_offset, y_offset, *str, buff);

        if(letterSize==0){

            letterWidth = table_0_len[(uint8_t)*str];
            x_offset += (letterWidth+1); // Move to the next character position
            // printf("\npixle len %d for %c",letterWidth ,*str);

        }else if(letterSize==1){

            letterWidth = table_1_len[(uint8_t)*str];
            x_offset += (letterWidth+1); // Move to the next character position
            // printf("\npixle len %d for %c",letterWidth ,*str);

        }else if(letterSize==2){

            letterWidth = table_2_len[(uint8_t)*str];
            x_offset += (letterWidth+1); // Move to the next character position
            // printf("\npixle len %d for %c",letterWidth ,*str);

        }
        str++;
    }
}

// Function to render a character onto the display buffer
void wrightChar(uint8_t letterSize, uint16_t x_offset, uint8_t y_offset, char c, camera_fb_t *buff) {
    

    // Get the bitmap data for the character
    const uint16_t *char_data=NULL;
    uint8_t letterWidth = 0;

    if(letterSize==0){
        
        char_data = font_table_0[(uint8_t)c];
        letterWidth = table_0_len[(uint8_t)c];
        
    } else if(letterSize==1){

        char_data = font_table_1[(uint8_t)c];
        letterWidth = table_1_len[(uint8_t)c];

    }else if(letterSize==2){

        char_data = font_table_2[(uint8_t)c];
        letterWidth = table_2_len[(uint8_t)c];

    }

   // Ensure the character fits within the buffer dimensions
    if (x_offset + letterWidth > buff->width || y_offset + letterWidth > buff->height) {
        // printf("Character position out of bounds\n");
        return;
    }


    for (uint8_t y = 0; y < tablehight[letterSize] ; y++) {
        for (uint8_t x = 0; x <= letterWidth; x++) {

            uint32_t buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
            // Get the pixel value from the character data
            if (char_data[y] & (1 << (16-x))) {
                // Draw white (pixel set)
                buff->buf[buff_index] = letterSize==0 ?0x00: 0xFF;
                buff->buf[buff_index + 1] = letterSize==0 ?0x00: 0xFF;
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

            if(letSize==0){
                len += table_0_len[(uint8_t)*str];
            }else if(letSize==1){
                len += table_1_len[(uint8_t)*str];
            }else if(letSize==2){
                len += table_2_len[(uint8_t)*str];
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
void WriteMulti7segment(uint16_t x_offset, uint8_t y_offset, const char *str, camera_fb_t *buff) {

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
void wrighSingle7segment(uint16_t x_offset, uint8_t y_offset, char c, camera_fb_t *buff) {
    
    if(c=='a'|| c=='g'||c=='d'){

#define HEIGHT_32 6
#define WIDTH_32 30

        // Get the bitmap data for the character
        const uint32_t *char_data = segment_table1[(uint8_t)c];

        // Ensure the character fits within the buffer dimensions
        if (x_offset + WIDTH_32 > buff->width || y_offset + WIDTH_32 > buff->height) {
            // printf("Character position out of bounds\n");
            return;
        }

        for (uint8_t y = 0; y < HEIGHT_32; y++) {
            for (uint8_t x = 0; x <= WIDTH_32; x++) {

                uint32_t buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
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
            // printf("Character position out of bounds\n");
            return;
        }

        for (uint8_t y = 0; y < HEIGHT_8; y++) {
            for (uint8_t x = 0; x <= WIDTH_8; x++) {
                uint32_t buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel
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







// void drawImage(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t *image, camera_fb_t *buff) {
   
//     // Ensure the image fits within the buffer dimensions
//     if (x_offset + width > buff->width || y_offset + height > buff->height) {
//         // Out of bounds, do nothing
//         return;
//     }
//     // printf("\n image drw data:\n");

//     for (int y = 1; y < height; y++) {
//         for (int x = 1; x < width; x++) {
//             // Calculate the index in the image data array
//             int image_index = (y * width + x) * 2; // 2 bytes per pixel for RGB565

//             // Calculate the index in the framebuffer
//             int buff_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel for RGB565

//             // Copy the image pixel to the framebuffer
//             buff->buf[buff_index] = image[image_index]; // High byte of RGB565
//             buff->buf[buff_index + 1] = image[image_index + 1]; // Low byte of RGB565

//         }
//     }

//     // printf("\n");

// }



// // Function to draw a rectangle border on an image buffer
// void drawRectangleBorder(uint16_t x_offset, uint8_t y_offset, uint8_t width, uint8_t height, uint8_t thickness, uint16_t color, camera_fb_t *buff) {
//     // Ensure the rectangle fits within the buffer dimensions
//     if (x_offset + width > buff->width || y_offset + height > buff->height) {
//         // Out of bounds, do nothing
//         return;
//     }

//     // Draw the top and bottom borders
//     for (int y = 0; y < thickness; y++) {
//         for (int x = 0; x < width; x++) {
//             int top_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel for RGB565
//             int bottom_index = ((y + y_offset + height - thickness) * buff->width + (x + x_offset)) * 2;

//             // Draw top border
//             buff->buf[top_index] = color >> 8; // High byte of RGB565
//             buff->buf[top_index + 1] = color & 0xFF; // Low byte of RGB565

//             // Draw bottom border
//             buff->buf[bottom_index] = color >> 8;
//             buff->buf[bottom_index + 1] = color & 0xFF;
//         }
//     }

//     // Draw the left and right borders
//     for (int x = 0; x < thickness; x++) {
//         for (int y = 0; y < height; y++) {
//             int left_index = ((y + y_offset) * buff->width + (x + x_offset)) * 2; // 2 bytes per pixel for RGB565
//             int right_index = ((y + y_offset) * buff->width + (x + x_offset + width - thickness)) * 2;

//             // Draw left border
//             buff->buf[left_index] = color >> 8;
//             buff->buf[left_index + 1] = color & 0xFF;

//             // Draw right border
//             buff->buf[right_index] = color >> 8;
//             buff->buf[right_index + 1] = color & 0xFF;
//         }
//     }
// }